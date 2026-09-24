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
        ClassDB::bind_method(
                D_METHOD("set_streaming_enabled", "enabled"), &SceneryStreamingServer::set_streaming_enabled);
        ClassDB::bind_method(D_METHOD("is_streaming_enabled"), &SceneryStreamingServer::is_streaming_enabled);
        ClassDB::bind_method(D_METHOD("set_camera", "camera"), &SceneryStreamingServer::set_camera);
        ClassDB::bind_method(D_METHOD("drain"), &SceneryStreamingServer::drain);
        ClassDB::bind_method(D_METHOD("get_draw_distance"), &SceneryStreamingServer::get_draw_distance);
        ClassDB::bind_method(D_METHOD("get_camera_position"), &SceneryStreamingServer::get_camera_position);
        ClassDB::bind_method(D_METHOD("has_camera"), &SceneryStreamingServer::has_camera);
        ClassDB::bind_method(
                D_METHOD("is_area_ready", "chunk_radius"), &SceneryStreamingServer::is_area_ready, DEFVAL(1));
        ClassDB::bind_method(D_METHOD("get_streamed_count"), &SceneryStreamingServer::get_streamed_count);
        ClassDB::bind_method(D_METHOD("get_statistics"), &SceneryStreamingServer::get_statistics);
    }

    SceneryStreamingServer::SceneryStreamingServer() {
        mutex.instantiate();
        semaphore.instantiate();
        draw_distance =
                ProjectSettings::get_singleton()->get_setting("maszyna/scenery/draw_distance", DEFAULT_DRAW_DISTANCE_M);
    }

    /// The worker finishes the pass it is in before it is joined
    SceneryStreamingServer::~SceneryStreamingServer() {
        set_camera(nullptr);
        drain();
    }

    void SceneryStreamingServer::drain() {
        if (worker.is_null()) {
            return;
        }
        {
            MutexLock lock(**mutex);
            exiting = true;
        }
        semaphore->post();
        worker->wait_to_finish();
        worker.unref();
        // the thread is joined, so the next plan may start a new one
        MutexLock lock(**mutex);
        exiting = false;
        planning = false;
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
        return static_cast<float>(Math::sqrt((x * x) + (z * z)));
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
        p_chunk.built_count = 0;
        for (const Entry &entry: kept) {
            if (entry.built) {
                p_chunk.built_count++;
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
        return static_cast<int>(owners.size() - 1);
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
        content_dirty = true;
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
        Entry *entry = _get_entry(p_stream_rid);
        if (entry != nullptr && entry->wanted_revision == target_revision && !entry->built) {
            pending_build_count--;
        }
        chunks[*key].dirty = true;
        entry_chunks.erase(p_stream_rid);
        freed_pending = true;
        content_dirty = true;
    }

    SceneryStreamingServer::Entry *SceneryStreamingServer::_get_entry(const RID &p_stream_rid) {
        const Vector2i *key = entry_chunks.getptr(p_stream_rid);
        if (key == nullptr) {
            return nullptr;
        }
        Chunk *chunk = chunks.getptr(*key);
        if (chunk == nullptr) {
            return nullptr;
        }
        for (Entry &entry: chunk->entries) {
            if (entry.stream_rid == p_stream_rid) {
                return &entry;
            }
        }
        return nullptr;
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
        builds.clear();
        for (const PendingBuild &pending: planned_builds) {
            if (entry_chunks.has(pending.stream_rid)) {
                builds.push_back(pending);
            }
        }
        planned_builds = builds;
        Vector<PendingClear> clears;
        for (const PendingClear &pending: pending_clears) {
            if (entry_chunks.has(pending.stream_rid)) {
                clears.push_back(pending);
            }
        }
        pending_clears = clears;
        clears.clear();
        for (const PendingClear &pending: planned_clears) {
            if (entry_chunks.has(pending.stream_rid)) {
                clears.push_back(pending);
            }
        }
        planned_clears = clears;
        freed_pending = false;
    }

    /// A new camera revision invalidates work which has not reached its owner yet. Already-built
    /// pieces stay alive until the new plan decides whether they are still wanted.
    void SceneryStreamingServer::_drop_stale_work() {
        Vector<PendingBuild> builds;
        for (const PendingBuild &pending: pending_builds) {
            if (pending.revision == target_revision && entry_chunks.has(pending.stream_rid)) {
                builds.push_back(pending);
            }
        }
        pending_builds = builds;
        planned_builds.clear();

        Vector<PendingClear> clears;
        for (const PendingClear &pending: pending_clears) {
            if (pending.revision == target_revision && entry_chunks.has(pending.stream_rid)) {
                clears.push_back(pending);
            }
        }
        pending_clears = clears;
        planned_clears.clear();
    }

    /// Camera the streaming follows; without one nothing is ever built. Setting the first camera
    /// starts the per-frame tick, clearing it (null) stops it - the main loop does not exist yet
    /// when the singleton is created.
    void SceneryStreamingServer::set_camera(Camera3D *p_camera) {
        bool was_streaming;
        bool is_streaming;
        {
            MutexLock lock(**mutex);
            was_streaming = camera_id.is_valid();
            camera_id = p_camera != nullptr ? ObjectID(p_camera->get_instance_id()) : ObjectID();
            is_streaming = camera_id.is_valid();
            target_revision++;
            scanned_revision = 0;
            pending_build_count = 0;
            force_plan = is_streaming;
            _drop_stale_work();
        }
        if (is_streaming == was_streaming) {
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

    Vector3 SceneryStreamingServer::get_camera_position() const {
        ObjectID current_camera_id;
        {
            MutexLock lock(**mutex);
            current_camera_id = camera_id;
        }
        const Camera3D *camera = Object::cast_to<Camera3D>(ObjectDB::get_instance(current_camera_id));
        return camera != nullptr && camera->is_inside_tree() ? camera->get_global_position() : last_camera_position;
    }

    bool SceneryStreamingServer::has_camera() const {
        MutexLock lock(**mutex);
        return camera_id.is_valid();
    }

    bool SceneryStreamingServer::_is_area_ready_locked(const int p_chunk_radius) const {
        return camera_id.is_valid() && scanned_revision == target_revision &&
               _get_pending_nearby_locked(p_chunk_radius) == 0;
    }

    int SceneryStreamingServer::_get_pending_builds_locked() const {
        return pending_build_count;
    }

    int SceneryStreamingServer::_get_pending_nearby_locked(const int p_chunk_radius) const {
        int pending = 0;
        const Vector2i camera_key = _get_chunk_key(camera_position);
        for (int x = camera_key.x - p_chunk_radius; x <= camera_key.x + p_chunk_radius; x++) {
            for (int y = camera_key.y - p_chunk_radius; y <= camera_key.y + p_chunk_radius; y++) {
                const Chunk *chunk = chunks.getptr(Vector2i(x, y));
                if (chunk == nullptr) {
                    continue;
                }
                for (const Entry &entry: chunk->entries) {
                    if (entry.wanted_revision == target_revision && !entry.built) {
                        pending++;
                    }
                }
            }
        }
        return pending;
    }

    bool SceneryStreamingServer::is_area_ready(const int p_chunk_radius) const {
        MutexLock lock(**mutex);
        return _is_area_ready_locked(MAX(0, p_chunk_radius));
    }

    /// Pieces currently built - what the streaming actually keeps alive
    int SceneryStreamingServer::get_streamed_count() const {
        MutexLock lock(**mutex);
        int count = 0;
        for (const KeyValue<Vector2i, Chunk> &item: chunks) {
            count += item.value.built_count;
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
            streamed += item.value.built_count;
            if (item.value.built_count > 0) {
                active_chunks++;
            }
        }
        statistics["owners"] = owners.size();
        statistics["registered"] = entry_chunks.size();
        statistics["streamed"] = streamed;
        statistics["chunks"] = chunks.size();
        statistics["active_chunks"] = active_chunks;
        const int pending_count = _get_pending_builds_locked();
        statistics["pending_builds"] = pending_count;
        statistics["pending_clears"] = pending_clears.size() + planned_clears.size();
        statistics["plan_msec"] = plan_msec;
        statistics["build_rate"] = build_rate;
        statistics["passes"] = passes;
        statistics["planning"] = planning;
        statistics["target_revision"] = target_revision;
        statistics["scanned_revision"] = scanned_revision;
        statistics["pending_nearby"] = _get_pending_nearby_locked(1);
        statistics["nearby_ready"] = _is_area_ready_locked(1);
        statistics["budget_msec"] = !_is_area_ready_locked(1) && pending_count + pending_clears.size() > CATCHUP_BACKLOG
                                            ? CATCHUP_BUDGET_MSEC
                                            : BUDGET_MSEC;
        statistics["draw_distance"] = draw_distance;
        statistics["chunk_size"] = CHUNK_SIZE_M;
        statistics["camera_position"] = camera_position;
        statistics["camera_chunk"] = _get_chunk_key(camera_position);
        statistics["has_camera"] = camera_id.is_valid();
        return statistics;
    }

    void SceneryStreamingServer::_request_plan(const Vector3 &p_position) {
        MutexLock lock(**mutex);
        camera_position = p_position;
        target_revision++;
        scanned_revision = 0;
        pending_build_count = 0;
        content_dirty = false;
        force_plan = false;
        _drop_stale_work();
        if (planning) {
            return;
        }
        planning = true;
        if (worker.is_null()) {
            worker.instantiate();
            worker->start(callable_mp(this, &SceneryStreamingServer::_worker_loop));
        }
        semaphore->post();
    }

    /// Plans after a meaningful camera move or a content change and applies whatever the worker has
    /// published so far, a few milliseconds per frame.
    void SceneryStreamingServer::set_streaming_enabled(const bool p_enabled) {
        streaming_enabled = p_enabled;
    }

    bool SceneryStreamingServer::is_streaming_enabled() const {
        return streaming_enabled;
    }

    void SceneryStreamingServer::_process_streaming() {
        if (!streaming_enabled) {
            return;
        }
        ObjectID current_camera_id;
        bool requested;
        {
            MutexLock lock(**mutex);
            current_camera_id = camera_id;
            requested = force_plan || content_dirty;
        }
        const Camera3D *camera = Object::cast_to<Camera3D>(ObjectDB::get_instance(current_camera_id));
        if (camera == nullptr || !camera->is_inside_tree()) {
            return;
        }
        const Vector3 position = camera->get_global_position();
        const uint64_t now = Time::get_singleton()->get_ticks_msec();
        const float movement = position.distance_to(last_camera_position);
        requested = requested || movement >= CAMERA_STEP_M || (movement > 0.0 && now - last_plan_msec >= INTERVAL_MSEC);
        if (requested) {
            last_plan_msec = now;
            last_camera_position = position;
            _request_plan(position);
        }
        _apply_plan();
    }

    /// Applies incrementally published work within a frame budget. Every task is validated against
    /// the current camera revision immediately before it reaches its owner.
    void SceneryStreamingServer::_apply_plan() {
        bool catching_up;
        {
            MutexLock lock(**mutex);
            if (planned_builds.size() > 0 || planned_clears.size() > 0) {
                pending_builds.append_array(planned_builds);
                pending_clears.append_array(planned_clears);
                planned_builds.clear();
                planned_clears.clear();
                pending_builds.sort_custom<DistanceComparator>();
            }
            if (freed_pending) {
                _drop_freed_work();
            }
            catching_up =
                    !_is_area_ready_locked(1) && _get_pending_builds_locked() + pending_clears.size() > CATCHUP_BACKLOG;
        }

        const uint64_t now = Time::get_singleton()->get_ticks_msec();
        const uint64_t deadline = now + (catching_up ? CATCHUP_BUDGET_MSEC : BUDGET_MSEC);
        if (now - build_rate_msec >= 1000) {
            build_rate = applied_builds;
            applied_builds = 0;
            build_rate_msec = now;
        }

        // Clearing first gives back what the builds below take.
        while (pending_clears.size() > 0) {
            const PendingClear pending = pending_clears[pending_clears.size() - 1];
            pending_clears.resize(pending_clears.size() - 1);
            bool apply = false;
            {
                MutexLock lock(**mutex);
                Entry *entry = _get_entry(pending.stream_rid);
                if (pending.revision == target_revision && entry != nullptr && entry->built &&
                    entry->wanted_revision != target_revision) {
                    entry->built = false;
                    chunks[entry_chunks[pending.stream_rid]].built_count--;
                    apply = true;
                }
            }
            if (apply && pending.clear.is_valid()) {
                pending.clear.call(pending.user_rid);
            }
            if (Time::get_singleton()->get_ticks_msec() >= deadline) {
                return;
            }
        }

        while (pending_builds.size() > 0) {
            const PendingBuild pending = pending_builds[pending_builds.size() - 1];
            pending_builds.resize(pending_builds.size() - 1);
            bool apply = false;
            {
                MutexLock lock(**mutex);
                Entry *entry = _get_entry(pending.stream_rid);
                apply = pending.revision == target_revision && entry != nullptr && !entry->built &&
                        entry->wanted_revision == target_revision;
                if (entry != nullptr && entry->queued_revision == pending.revision) {
                    entry->queued_revision = 0;
                }
            }
            if (apply && pending.build.is_valid()) {
                pending.build.call(pending.user_rid, pending.preloaded);
                MutexLock lock(**mutex);
                Entry *entry = _get_entry(pending.stream_rid);
                if (pending.revision == target_revision && entry != nullptr && !entry->built &&
                    entry->wanted_revision == target_revision) {
                    entry->built = true;
                    chunks[entry_chunks[pending.stream_rid]].built_count++;
                    pending_build_count--;
                }
                applied_builds++;
            }
            if (Time::get_singleton()->get_ticks_msec() >= deadline) {
                return;
            }
        }
    }

    /// Computes the complete desired set first, then preloads and publishes it nearest-first. A
    /// changed camera revision aborts the pass between individual preloads.
    bool SceneryStreamingServer::_plan(const uint64_t p_revision, const Vector3 &p_camera_position) {
        const uint64_t started_msec = Time::get_singleton()->get_ticks_msec();
        Vector<PendingBuild> entering;
        Vector<PendingClear> leaving;
        int wanted_unbuilt = 0;
        {
            MutexLock lock(**mutex);
            if (p_revision != target_revision || !camera_id.is_valid()) {
                return false;
            }
            for (KeyValue<Vector2i, Chunk> &item: chunks) {
                Chunk &chunk = item.value;
                if (chunk.dirty) {
                    _sort_chunk(chunk);
                }
                const float distance = _get_chunk_distance(item.key, p_camera_position);
                for (Entry &entry: chunk.entries) {
                    const bool wanted =
                            entry.built ? distance <= entry.range_end + HYSTERESIS_M : distance <= entry.range_end;
                    entry.wanted_revision = wanted ? p_revision : 0;
                    if (wanted && !entry.built && entry.queued_revision != p_revision) {
                        PendingBuild build;
                        build.preload = owners[entry.owner].preload;
                        build.build = owners[entry.owner].build;
                        build.stream_rid = entry.stream_rid;
                        build.user_rid = entry.user_rid;
                        build.chunk = item.key;
                        build.distance = distance;
                        build.revision = p_revision;
                        entering.push_back(build);
                        entry.queued_revision = p_revision;
                    } else if (!wanted && entry.built) {
                        PendingClear clear;
                        clear.clear = owners[entry.owner].clear;
                        clear.stream_rid = entry.stream_rid;
                        clear.user_rid = entry.user_rid;
                        clear.revision = p_revision;
                        leaving.push_back(clear);
                    }
                    if (wanted && !entry.built) {
                        wanted_unbuilt++;
                    }
                }
            }
            pending_build_count = wanted_unbuilt;
            scanned_revision = p_revision;
            planned_clears.append_array(leaving);
        }

        entering.sort_custom<DistanceComparator>();
        for (int64_t i = entering.size() - 1; i >= 0; i--) {
            PendingBuild build = entering[i];
            {
                MutexLock lock(**mutex);
                if (p_revision != target_revision || !entry_chunks.has(build.stream_rid)) {
                    return false;
                }
            }
            if (build.preload.is_valid()) {
                build.preloaded = build.preload.call(build.user_rid);
            }
            {
                MutexLock lock(**mutex);
                Entry *entry = _get_entry(build.stream_rid);
                if (p_revision != target_revision || entry == nullptr || entry->wanted_revision != p_revision) {
                    return false;
                }
                planned_builds.push_back(build);
            }
        }

        MutexLock lock(**mutex);
        if (p_revision != target_revision) {
            return false;
        }
        plan_msec = Time::get_singleton()->get_ticks_msec() - started_msec;
        passes++;
        return true;
    }

    void SceneryStreamingServer::_worker_loop() {
        while (true) {
            semaphore->wait();
            while (true) {
                uint64_t revision;
                Vector3 position;
                {
                    MutexLock lock(**mutex);
                    if (exiting) {
                        return;
                    }
                    if (!camera_id.is_valid()) {
                        planning = false;
                        break;
                    }
                    revision = target_revision;
                    position = camera_position;
                }
                const bool completed = _plan(revision, position);
                MutexLock lock(**mutex);
                if (exiting) {
                    return;
                }
                if (completed && revision == target_revision) {
                    planning = false;
                    break;
                }
                if (!camera_id.is_valid()) {
                    planning = false;
                    break;
                }
            }
        }
    }
} // namespace godot
