#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/rid_owner.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <string>

namespace godot {
    class TrainController;
    class TrainPart;

    class TrainPhysicsServer : public Object {
            GDCLASS(TrainPhysicsServer, Object);

        private:
            Vector<TrainController *> active_controllers;
            Vector<TrainPart *> active_parts;

            // Centralized ownership of the McZapkie simulation instances, mirroring how Godot's own
            // servers (e.g. PhysicsServer3D) own their data behind opaque RIDs instead of nodes holding
            // raw pointers. RID_PtrOwner stores the externally-allocated TMoverParameters* as-is (no
            // copy of the pointee, unlike RID_Owner). THREAD_SAFE=true guards make_rid()/free()/
            // get_or_null() against concurrent registration while worker tasks may be reading the table
            // during step_physics(). `mutable` so get_mover() can stay const like TrainController's does.
            mutable RID_PtrOwner<TMoverParameters, true> movers;

            // Scratch state, valid only for the duration of a single step_physics() call.
            Vector<Vector<TrainController *>> current_consists;
            double current_delta = 0.0;

            Vector<Vector<TrainController *>> _discover_consists() const;

        public:
            static TrainPhysicsServer *get_instance();

            void register_controller(TrainController *p_controller);
            void unregister_controller(TrainController *p_controller);

            void register_part(TrainPart *p_part);
            void unregister_part(TrainPart *p_part);

            RID create_mover(double p_initial_velocity, const std::string &p_type_name, const std::string &p_name, int p_cabin_number);
            TMoverParameters *get_mover(RID p_mover) const;
            void free_mover(RID p_mover);

            void _setup_connection();
            void _physics_process_callback();

            // The entire per-tick lifecycle for every registered TrainController/TrainPart lives here,
            // driven by Godot's physics_frame signal — never by Node's own _process()/_physics_process()
            // — so config flushing, simulation, and state sync all happen exactly once per physics tick,
            // in this fixed order: (1) flush dirty config/enabled-state (main thread); (2) simulation
            // (McZapkie compute, no Godot API calls) in parallel, one WorkerThreadPool task per connected
            // consist (coupled vehicles must stay on one thread, see _discover_consists()); (3) parts and
            // Dictionary/signal sync, sequential on the main thread (Godot Object/Variant API). No
            // command-mutating TrainController/TrainPart method may be called while a consist task is in
            // flight — wait_for_group_task_completion() blocks the main thread until every consist
            // finishes before step (3) starts.
            void step_physics(double p_delta);
            void _step_consist_task(int p_index);

            // Safety note (not code): TMoverParameters::ComputeMass() reads the file-scope
            // Maszyna::simulation::Weights map (MOVER.h) on the hot path. It's never populated
            // anywhere in the repo today (read-only/inert), which is why concurrent consist tasks
            // reading it is currently safe. If it's ever populated at runtime, that must happen
            // before any step_physics() call is in flight, not concurrently with one.

        protected:
            static void _bind_methods();
    };
} // namespace godot
