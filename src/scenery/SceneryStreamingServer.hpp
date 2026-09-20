#pragma once
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    /// Spatial streaming of scenery content. A rendering server registers where each of its pieces
    /// is placed and how far it is meant to be visible; the piece is built once the camera comes
    /// within that range of the chunk holding it and cleared again when the camera leaves.
    ///
    /// A real scenery places hundreds of thousands of pieces, so building them all at load costs
    /// both the loading time and the frame rate - most of them are nowhere near the camera.
    ///
    /// Planning runs on a worker thread, together with whatever an owner wants prepared off the
    /// main thread (loading a model, building a mesh). Building and clearing run on the main
    /// thread within a per-frame time budget, because they touch RenderingServer and GDScript.
    class SceneryStreamingServer : public Object {
            GDCLASS(SceneryStreamingServer, Object)

        public:
            /// XZ size of a streaming chunk, same grid as the scenery triangle chunks
            /// (SceneryInstancer.TRIANGLE_CHUNK_SIZE_M)
            static constexpr float CHUNK_SIZE_M = 1000.0;
            /// Pieces are cleared only beyond their range plus this margin (whichever of the two
            /// is larger), so a camera moving around a range boundary does not rebuild them over
            /// and over - rebuilding is far more expensive than keeping them a little longer
            static constexpr float HYSTERESIS_M = 250.0;
            static constexpr float HYSTERESIS_FACTOR = 0.25;
            /// A pass is planned at most this often...
            static constexpr uint64_t INTERVAL_MSEC = 250;
            /// ...or as soon as the camera has moved this far
            static constexpr float CAMERA_STEP_M = 50.0;
            /// Time spent building and clearing per frame once the streaming has caught up
            static constexpr uint64_t BUDGET_MSEC = 4;
            /// ...and while it has not: filling a scenery in means thousands of builds, and at the
            /// idle budget that takes minutes of pop-in (the backlog only drains at frame rate
            /// times budget). A short hitch while catching up beats watching the world appear.
            static constexpr uint64_t CATCHUP_BUDGET_MSEC = 16;
            /// Backlog above which the catch-up budget is used
            static constexpr int CATCHUP_BACKLOG = 64;
            /// Fallback for maszyna/rendering/scenery_draw_distance, also the range of the pieces
            /// that declare none of their own
            static constexpr float DEFAULT_DRAW_DISTANCE_M = 3000.0;

            static SceneryStreamingServer *get_instance() {
                return Object::cast_to<SceneryStreamingServer>(
                        Engine::get_singleton()->get_singleton("SceneryStreamingServer"));
            }

        private:
            struct Owner {
                    Callable preload; // (rid) -> Variant, on the worker thread; may be invalid
                    Callable build;   // (rid, preloaded) on the main thread
                    Callable clear;   // (rid) on the main thread
            };

            struct Entry {
                    int owner = 0;
                    RID stream_rid;
                    RID user_rid;
                    float range_end = 0.0;
                    bool streamed = false; // handed to the main thread as a build
            };

            struct Chunk {
                    Vector<Entry> entries; // sorted by range_end, descending
                    int streamed_count = 0;
                    bool dirty = false; // entries registered or unregistered since the last sort
            };

            struct PendingBuild {
                    Callable build;
                    RID stream_rid;
                    RID user_rid;
                    Variant preloaded;
                    Vector2i chunk;
                    float distance = 0.0;
            };

            struct PendingClear {
                    Callable clear;
                    RID stream_rid;
                    RID user_rid;
            };

            /// Longest visible range first, so the pieces a chunk wants at a given distance are
            /// always a prefix of its entries
            struct RangeComparator {
                    bool operator()(const Entry &p_left, const Entry &p_right) const {
                        return p_left.range_end > p_right.range_end;
                    }
            };

            /// Farthest first: pending_builds is a priority queue kept as a sorted vector, so the
            /// nearest build is its last element and taking it costs nothing
            struct DistanceComparator {
                    bool operator()(const PendingBuild &p_left, const PendingBuild &p_right) const {
                        return p_left.distance > p_right.distance;
                    }
            };

            Ref<Mutex> mutex;
            Ref<Semaphore> semaphore;
            Ref<Thread> worker;
            bool exiting = false;
            bool planning = false;
            bool freed_pending = false; // a piece was freed, the queues may hold dead work

            Vector<Owner> owners;
            HashMap<Vector2i, Chunk> chunks;
            HashMap<RID, Vector2i> entry_chunks;
            Vector3 camera_position;
            float draw_distance = DEFAULT_DRAW_DISTANCE_M;

            ObjectID camera_id;
            Vector3 last_camera_position;
            uint64_t last_plan_msec = 0;
            Vector<PendingBuild> planned_builds; // published by the worker
            Vector<PendingClear> planned_clears;
            /// Taken over by the main thread; builds are ordered farthest first and taken from the
            /// back, so the pieces around the camera are built first
            Vector<PendingBuild> pending_builds;
            Vector<PendingClear> pending_clears;
            uint64_t plan_msec = 0; // duration of the last planning pass
            int applied_builds = 0; // builds applied in the second being counted
            int build_rate = 0;     // ...and in the last full second, for the debug window
            uint64_t build_rate_msec = 0;
            int passes = 0; // planning passes finished, so a caller can tell "not started yet"

            static Vector2i _get_chunk_key(const Vector3 &p_origin);
            static float _get_chunk_distance(const Vector2i &p_key, const Vector3 &p_position);
            void _sort_chunk(Chunk &p_chunk) const;
            void _plan();
            void _drop_freed_work();
            void _worker_loop();
            void _process_streaming();
            void _apply_plan();

        protected:
            static void _bind_methods();

        public:
            SceneryStreamingServer();
            ~SceneryStreamingServer() override;

            int owner_create(const Callable &p_preload, const Callable &p_build, const Callable &p_clear);
            RID stream_register(int p_owner, const RID &p_user_rid, const Vector3 &p_position, float p_range_end);
            void stream_free(const RID &p_stream_rid);

            void set_camera(Camera3D *p_camera);
            float get_draw_distance() const;
            /// Where the streaming camera is, for anything else that has to know what is near
            Vector3 get_camera_position() const;
            bool has_camera() const;
            int get_streamed_count() const;
            Dictionary get_statistics() const;
    };
} // namespace godot
