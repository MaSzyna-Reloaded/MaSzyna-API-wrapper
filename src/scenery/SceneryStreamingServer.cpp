#include "SceneryStreamingServer.hpp"
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/mutex_lock.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void SceneryStreamingServer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("owner_create", "preload", "build", "clear"), &SceneryStreamingServer::owner_create);
        ClassDB::bind_method(
                D_METHOD("stream_register", "owner", "rid", "position", "range_end"),
                &SceneryStreamingServer::stream_register);
        ClassDB::bind_method(D_METHOD("stream_free", "stream_rid"), &SceneryStreamingServer::stream_free);
        ClassDB::bind_method(D_METHOD("set_camera", "camera"), &SceneryStreamingServer::set_camera);
        ClassDB::bind_method(D_METHOD("get_draw_distance"), &SceneryStreamingServer::get_draw_distance);
        ClassDB::bind_method(D_METHOD("get_streamed_count"), &SceneryStreamingServer::get_streamed_count);
        ClassDB::bind_method(D_METHOD("get_statistics"), &SceneryStreamingServer::get_statistics);
    }

    SceneryStreamingServer::SceneryStreamingServer() {
        mutex.instantiate();
        semaphore.instantiate();
        draw_distance = ProjectSettings::get_singleton()->get_setting(
                "maszyna/rendering/scenery_draw_distance", DEFAULT_DRAW_DISTANCE_M);
    }

    /// The worker finishes the pass it is in before it is joined
    SceneryStreamingServer::~SceneryStreamingServer() {
        set_camera(nullptr);
        {
            MutexLock lock(**mutex);
            exiting = true;
        }
        semaphore->post();
        if (worker.is_valid()) {
            worker->wait_to_finish();
        }
    }

    Vector2i SceneryStreamingServer::_get_chunk_key(const Vector3 &p_origin) {
        return Vector2i(
                static_cast<int32_t>(Math::floor(p_origin.x / CHUNK_SIZE_M)),
                static_cast<int32_t>(Math::floor(p_origin.z / CHUNK_SIZE_M)));
    }

    /// Distance from the camera to the nearest edge of the chunk (0 inside it), on the XZ plane -
    /// height does not tell how far a scenery piece is
    float SceneryStreamingServer::_get_chunk_distance(const Vector2i &p_key, const Vector3 &p_position) {
        const double min_x = static_cast<double>(p_key.x) * CHUNK_SIZE_M;
        const double min_z = static_cast<double>(p_key.y) * CHUNK_SIZE_M;
        const double x = MAX(0.0, MAX(min_x - p_position.x, p_position.x - (min_x + CHUNK_SIZE_M)));
        const double z = MAX(0.0, MAX(min_z - p_position.z, p_position.z - (min_z + CHUNK_SIZE_M)));
        return static_cast<float>(Math::sqrt(x * x + z * z));
    }

    /// Drops the entries of freed pieces and restores the range order and the streamed count
    void SceneryStreamingServer::_sort_chunk(Chunk &p_chunk) const {
        Vector<Entry> kept;
        for (const Entry &entry: p_chunk.entries) {
            if (entry_chunks.has(entry.stream_rid)) {
                kept.push_back(entry);
            }
        }
        kept.sort_custom<RangeComparator>();
        p_chunk.streamed_count = 0;
        for (const Entry &entry: kept) {
            if (entry.streamed) {
                p_chunk.streamed_count++;
            }
        }
        p_chunk.entries = kept;
        p_chunk.dirty = false;
    }

    /// Registers a rendering server with the streaming. [param preload] is called on the worker
    /// thread and its result is passed to [param build]; pass an invalid Callable when there is
    /// nothing to prepare off the main thread.
    int
    SceneryStreamingServer::owner_create(const Callable &p_preload, const Callable &p_build, const Callable &p_clear) {
        MutexLock lock(**mutex);
        Owner owner;
        owner.preload = p_preload;
        owner.build = p_build;
        owner.clear = p_clear;
        owners.push_back(owner);
        return owners.size() - 1;
    }

    /// Registers one piece of an owner. [param range_end] of 0 or less means the piece declares no
    /// range of its own and is streamed up to the global draw distance.
    RID SceneryStreamingServer::stream_register(
            const int p_owner, const RID &p_user_rid, const Vector3 &p_position, const float p_range_end) {
        MutexLock lock(**mutex);
        ERR_FAIL_INDEX_V(p_owner, owners.size(), RID());
        Entry entry;
        entry.owner = p_owner;
        entry.stream_rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        entry.user_rid = p_user_rid;
        entry.range_end = p_range_end > 0.0 && p_range_end < draw_distance ? p_range_end : draw_distance;
        const Vector2i key = _get_chunk_key(p_position);
        Chunk &chunk = chunks[key];
        chunk.entries.push_back(entry);
        chunk.dirty = true;
        entry_chunks[entry.stream_rid] = key;
        return entry.stream_rid;
    }

    /// The entry itself is dropped by the next _sort_chunk() - freeing a whole scenery stays O(1)
    /// per piece instead of searching the chunk for every one of them. The owner's clear callable
    /// is not called: whoever frees the piece frees its content too.
    void SceneryStreamingServer::stream_free(const RID &p_stream_rid) {
        MutexLock lock(**mutex);
        const Vector2i *key = entry_chunks.getptr(p_stream_rid);
        if (key == nullptr) {
            return;
        }
        chunks[*key].dirty = true;
        entry_chunks.erase(p_stream_rid);
        freed_pending = true;
    }

    /// Queued work of pieces freed since the last frame. Unloading a scenery frees thousands of
    /// them at once, and their builds would otherwise keep running - eating the whole frame budget
    /// while the game is already showing the next screen, and delaying the next scenery's own work.
    /// Must be called with the mutex held.
    void SceneryStreamingServer::_drop_freed_work() {
        Vector<PendingBuild> builds;
        for (const PendingBuild &pending: pending_builds) {
            if (entry_chunks.has(pending.stream_rid)) {
                builds.push_back(pending);
            }
        }
        pending_builds = builds;
        Vector<PendingClear> clears;
        for (const PendingClear &pending: pending_clears) {
            if (entry_chunks.has(pending.stream_rid)) {
                clears.push_back(pending);
            }
        }
        pending_clears = clears;
        freed_pending = false;
    }

    /// Camera the streaming follows; without one nothing is ever built. Setting the first camera
    /// starts the per-frame tick, clearing it (null) stops it - the main loop does not exist yet
    /// when the singleton is created.
    void SceneryStreamingServer::set_camera(Camera3D *p_camera) {
        const bool was_streaming = camera_id.is_valid();
        camera_id = p_camera != nullptr ? ObjectID(p_camera->get_instance_id()) : ObjectID();
        if (camera_id.is_valid() == was_streaming) {
            return;
        }
        SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree == nullptr) {
            return;
        }
        if (was_streaming) {
            tree->disconnect("process_frame", callable_mp(this, &SceneryStreamingServer::_process_streaming));
            return;
        }
        tree->connect("process_frame", callable_mp(this, &SceneryStreamingServer::_process_streaming));
    }

    float SceneryStreamingServer::get_draw_distance() const {
        return draw_distance;
    }

    /// Pieces currently built - what the streaming actually keeps alive
    int SceneryStreamingServer::get_streamed_count() const {
        MutexLock lock(**mutex);
        int count = 0;
        for (const KeyValue<Vector2i, Chunk> &item: chunks) {
            count += item.value.streamed_count;
        }
        return count;
    }

    /// What the streaming is doing right now, for the "Scenery Streaming" debug window
    Dictionary SceneryStreamingServer::get_statistics() const {
        Dictionary statistics;
        MutexLock lock(**mutex);
        int streamed = 0;
        int active_chunks = 0;
        for (const KeyValue<Vector2i, Chunk> &item: chunks) {
            streamed += item.value.streamed_count;
            if (item.value.streamed_count > 0) {
                active_chunks++;
            }
        }
        statistics["owners"] = owners.size();
        statistics["registered"] = entry_chunks.size();
        statistics["streamed"] = streamed;
        statistics["chunks"] = chunks.size();
        statistics["active_chunks"] = active_chunks;
        statistics["pending_builds"] = pending_builds.size() + planned_builds.size();
        statistics["pending_clears"] = pending_clears.size() + planned_clears.size();
        statistics["plan_msec"] = plan_msec;
        statistics["build_rate"] = build_rate;
        statistics["passes"] = passes;
        statistics["budget_msec"] =
                pending_builds.size() + pending_clears.size() > CATCHUP_BACKLOG ? CATCHUP_BUDGET_MSEC : BUDGET_MSEC;
        statistics["draw_distance"] = draw_distance;
        statistics["chunk_size"] = CHUNK_SIZE_M;
        statistics["camera_position"] = camera_position;
        statistics["camera_chunk"] = _get_chunk_key(camera_position);
        statistics["has_camera"] = camera_id.is_valid();
        return statistics;
    }

    /// Plans rarely (the camera rarely leaves the chunks it already streamed) and applies whatever
    /// the worker has planned so far, a few milliseconds per frame
    void SceneryStreamingServer::_process_streaming() {
        const Camera3D *camera = Object::cast_to<Camera3D>(ObjectDB::get_instance(camera_id));
        if (camera == nullptr) {
            return;
        }
        const Vector3 position = camera->get_global_position();
        const uint64_t now = Time::get_singleton()->get_ticks_msec();
        if (now - last_plan_msec >= INTERVAL_MSEC || position.distance_to(last_camera_position) >= CAMERA_STEP_M) {
            last_plan_msec = now;
            last_camera_position = position;
            {
                MutexLock lock(**mutex);
                camera_position = position;
                if (!planning) {
                    planning = true;
                    if (worker.is_null()) {
                        // started with the first pass, not in the constructor (the object is
                        // fully set up then)
                        worker.instantiate();
                        worker->start(callable_mp(this, &SceneryStreamingServer::_worker_loop));
                    }
                    semaphore->post();
                }
            }
        }
        _apply_plan();
    }

    /// Applies what the worker planned, a few milliseconds per frame. The plan is taken over
    /// first, so the worker never touches the queues this walks through.
    void SceneryStreamingServer::_apply_plan() {
        {
            MutexLock lock(**mutex);
            if (planned_builds.size() > 0 || planned_clears.size() > 0) {
                pending_builds.append_array(planned_builds);
                pending_clears.append_array(planned_clears);
                planned_builds.clear();
                planned_clears.clear();
                // re-prioritise the whole backlog against where the camera is now: a pass planned
                // while the camera was elsewhere must not keep distant pieces ahead of near ones
                PendingBuild *builds = pending_builds.ptrw();
                for (int i = 0; i < pending_builds.size(); i++) {
                    builds[i].distance = _get_chunk_distance(builds[i].chunk, camera_position);
                }
                pending_builds.sort_custom<DistanceComparator>();
            }
            if (freed_pending) {
                _drop_freed_work();
            }
        }
        const uint64_t now = Time::get_singleton()->get_ticks_msec();
        const bool catching_up = pending_builds.size() + pending_clears.size() > CATCHUP_BACKLOG;
        const uint64_t deadline = now + (catching_up ? CATCHUP_BUDGET_MSEC : BUDGET_MSEC);
        if (now - build_rate_msec >= 1000) {
            build_rate = applied_builds;
            applied_builds = 0;
            build_rate_msec = now;
        }
        // clearing first: it gives back what the builds below take
        while (pending_clears.size() > 0) {
            const PendingClear pending = pending_clears[pending_clears.size() - 1];
            pending_clears.resize(pending_clears.size() - 1);
            pending.clear.call(pending.user_rid);
            if (Time::get_singleton()->get_ticks_msec() >= deadline) {
                return;
            }
        }
        while (pending_builds.size() > 0) {
            const PendingBuild pending = pending_builds[pending_builds.size() - 1];
            pending_builds.resize(pending_builds.size() - 1);
            pending.build.call(pending.user_rid, pending.preloaded);
            applied_builds++;
            if (Time::get_singleton()->get_ticks_msec() >= deadline) {
                return;
            }
        }
    }

    /// One planning pass: decide what enters and leaves the streamed set, then prepare what the
    /// main thread will need. The streamed flags are updated here, so a pass never re-plans what a
    /// previous one already handed over, however long the main thread takes to apply it.
    void SceneryStreamingServer::_plan() {
        const uint64_t started_msec = Time::get_singleton()->get_ticks_msec();
        Vector<PendingBuild> entering;
        Vector<PendingClear> leaving;
        Vector<Callable> preloads;
        {
            MutexLock lock(**mutex);
            for (KeyValue<Vector2i, Chunk> &item: chunks) {
                Chunk &chunk = item.value;
                if (chunk.dirty) {
                    _sort_chunk(chunk);
                }
                const int count = chunk.entries.size();
                if (count == 0) {
                    continue;
                }
                const float distance = _get_chunk_distance(item.key, camera_position);
                // nothing streamed and the longest range does not reach, or everything streamed
                // and even the shortest range reaches: the chunk is already as it should be
                if (chunk.streamed_count == 0 && distance > chunk.entries[0].range_end) {
                    continue;
                }
                if (chunk.streamed_count == count && distance <= chunk.entries[count - 1].range_end) {
                    continue;
                }
                Entry *entries = chunk.entries.ptrw();
                for (int i = 0; i < count; i++) {
                    Entry &entry = entries[i];
                    const bool wanted =
                            entry.streamed ? distance <= entry.range_end + HYSTERESIS_M : distance <= entry.range_end;
                    if (wanted == entry.streamed) {
                        continue;
                    }
                    entry.streamed = wanted;
                    if (wanted) {
                        chunk.streamed_count++;
                        PendingBuild build;
                        build.stream_rid = entry.stream_rid;
                        build.user_rid = entry.user_rid;
                        build.chunk = item.key;
                        build.distance = distance;
                        build.build = owners[entry.owner].build;
                        entering.push_back(build);
                        preloads.push_back(owners[entry.owner].preload);
                        continue;
                    }
                    chunk.streamed_count--;
                    PendingClear clear;
                    clear.clear = owners[entry.owner].clear;
                    clear.stream_rid = entry.stream_rid;
                    clear.user_rid = entry.user_rid;
                    leaving.push_back(clear);
                }
            }
        }

        // the slow part of a pass, deliberately outside the lock
        PendingBuild *builds = entering.ptrw();
        for (int i = 0; i < entering.size(); i++) {
            if (preloads[i].is_valid()) {
                builds[i].preloaded = preloads[i].call(builds[i].user_rid);
            }
        }
        MutexLock lock(**mutex);
        planned_builds.append_array(entering);
        planned_clears.append_array(leaving);
        plan_msec = Time::get_singleton()->get_ticks_msec() - started_msec;
        passes++;
        planning = false;
    }

    void SceneryStreamingServer::_worker_loop() {
        while (true) {
            semaphore->wait();
            {
                MutexLock lock(**mutex);
                if (exiting) {
                    return;
                }
            }
            _plan();
        }
    }
} // namespace godot
