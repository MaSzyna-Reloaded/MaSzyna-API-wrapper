#include "../core/TrainController.hpp"
#include "../core/TrainPart.hpp"
#include "TrainPhysicsServer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>

namespace godot {
    TrainPhysicsServer *TrainPhysicsServer::get_instance() {
        return dynamic_cast<TrainPhysicsServer *>(Engine::get_singleton()->get_singleton("TrainPhysicsServer"));
    }

    void TrainPhysicsServer::_bind_methods() {
        ClassDB::bind_method(D_METHOD("step_physics", "delta"), &TrainPhysicsServer::step_physics);
        ClassDB::bind_method(D_METHOD("_physics_process_callback"), &TrainPhysicsServer::_physics_process_callback);
        ClassDB::bind_method(D_METHOD("_step_consist_task", "index"), &TrainPhysicsServer::_step_consist_task);
    }

    void TrainPhysicsServer::_setup_connection() {
        SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree != nullptr) {
            const Callable cb = Callable(this, "_physics_process_callback");
            if (!tree->is_connected("physics_frame", cb)) {
                tree->connect("physics_frame", cb);
            }
        }
    }

    void TrainPhysicsServer::_physics_process_callback() {
        const SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree != nullptr) {
            // SceneTree has no get_physics_process_delta_time() of its own; get_root() is a Node,
            // which does.
            const double delta = tree->get_root()->get_physics_process_delta_time();
            step_physics(delta);
        }
    }

    void TrainPhysicsServer::register_controller(TrainController *p_controller) {
        if (!active_controllers.has(p_controller)) {
            active_controllers.push_back(p_controller);
        }
        _setup_connection();
    }

    void TrainPhysicsServer::unregister_controller(TrainController *p_controller) {
        active_controllers.erase(p_controller);
    }

    void TrainPhysicsServer::register_part(TrainPart *p_part) {
        if (!active_parts.has(p_part)) {
            active_parts.push_back(p_part);
        }
        _setup_connection();
    }

    void TrainPhysicsServer::unregister_part(TrainPart *p_part) {
        active_parts.erase(p_part);
    }

    RID TrainPhysicsServer::create_mover(
            const double p_initial_velocity, const std::string &p_type_name, const std::string &p_name,
            const int p_cabin_number) {
        auto *mover = new TMoverParameters(p_initial_velocity, p_type_name, p_name, p_cabin_number);
        return movers.make_rid(mover);
    }

    TMoverParameters *TrainPhysicsServer::get_mover(const RID p_mover) const {
        return movers.get_or_null(p_mover);
    }

    void TrainPhysicsServer::free_mover(const RID p_mover) {
        if (TMoverParameters *mover = movers.get_or_null(p_mover); mover != nullptr) {
            movers.free(p_mover);
            delete mover;
        }
    }

    // Groups registered controllers into connected consists (chains of physically/electrically/
    // pneumatically coupled vehicles, linked via TMoverParameters::Couplers[end].Connected) so that
    // step_physics() can dispatch one WorkerThreadPool task per consist: vehicles in the same consist
    // read and write each other's state during a tick (brake-pipe pressure propagation, coupled command
    // cascades) and must be stepped sequentially, on one thread; vehicles in different consists share no
    // state and can run fully concurrently. Each consist's members are ordered the same way
    // active_controllers already is, to preserve today's single-threaded, order-dependent numerical
    // behavior (e.g. which coupled car's brake-pipe "push" lands first in a given tick).
    Vector<Vector<TrainController *>> TrainPhysicsServer::_discover_consists() const {
        HashMap<TMoverParameters *, TrainController *> owner_by_mover;
        for (auto *const controller: active_controllers) {
            if (TMoverParameters *mover = controller->get_mover(); mover != nullptr) {
                owner_by_mover.insert(mover, controller);
            }
        }

        HashMap<TrainController *, TrainController *> representative_of;

        for (auto *const controller: active_controllers) {
            if (representative_of.has(controller)) {
                continue;
            }

            TMoverParameters *mover = controller->get_mover();
            if (mover == nullptr) {
                representative_of.insert(controller, controller);
                continue;
            }

            // Walk the coupled chain in both directions (each vehicle has at most two neighbors:
            // front/rear), collecting every controller reachable from this one.
            Vector<TMoverParameters *> to_visit;
            to_visit.push_back(mover);
            HashSet<TMoverParameters *> seen_movers;
            seen_movers.insert(mover);
            Vector<TrainController *> chain;
            chain.push_back(controller);

            while (!to_visit.is_empty()) {
                TMoverParameters *current = to_visit[to_visit.size() - 1];
                to_visit.resize(to_visit.size() - 1);

                for (const auto & coupler : current->Couplers) {
                    TMoverParameters *neighbor = coupler.Connected;
                    if (neighbor == nullptr || seen_movers.has(neighbor)) {
                        continue;
                    }
                    seen_movers.insert(neighbor);
                    to_visit.push_back(neighbor);
                    if (TrainController *const *neighbor_controller = owner_by_mover.getptr(neighbor);
                        neighbor_controller != nullptr) {
                        chain.push_back(*neighbor_controller);
                    }
                }
            }

            for (auto *const chain_controller: chain) {
                representative_of.insert(chain_controller, controller);
            }
        }

        HashMap<TrainController *, int> consist_index_of;
        Vector<Vector<TrainController *>> consists;

        for (auto *const controller: active_controllers) {
            TrainController *representative = representative_of.has(controller) ? representative_of[controller] : controller;
            if (!consist_index_of.has(representative)) {
                consist_index_of.insert(representative, static_cast<int>(consists.size()));
                consists.push_back(Vector<TrainController *>());
            }
            consists.write[consist_index_of[representative]].push_back(controller);
        }

        return consists;
    }

    void TrainPhysicsServer::_step_consist_task(const int p_index) {
        for (auto *const controller: current_consists[p_index]) {
            controller->_process_mover_thread_safe(current_delta);
        }
    }

    void TrainPhysicsServer::step_physics(const double p_delta) {
        // Simulation (McZapkie compute: ComputeTotalForce + ComputeMovement, pure data computation,
        // no Godot API calls) runs in parallel, one WorkerThreadPool task per connected consist.
        // Coupled vehicles share mutable state (brake-pipe pressure, command cascades via
        // SendCtrlToNext) and must stay on one thread; uncoupled vehicles/consists share nothing and
        // run fully concurrently. wait_for_group_task_completion() blocks the main thread until every
        // consist has finished, so no command-mutating TrainController/TrainPart method (battery,
        // main_controller_increase/decrease, etc. — all main-thread-only, dispatched via Godot
        // signals/Callables) can ever run concurrently with a consist task.
        // Pre-process (main thread): flush any dirty config/enabled-state before this tick's
        // simulation runs, so config changes apply exactly once per physics tick, synchronized with
        // the compute that follows — previously this ran on Node's own _process() (idle/render rate),
        // decoupled from the simulation's actual cadence. Unconditional (not gated by get_enabled()):
        // a part that just became disabled still needs its enabled-transition handled here.
        for (auto *const active_controller: active_controllers) {
            active_controller->_update_mover_config_if_dirty();
        }
        for (auto *const active_part: active_parts) {
            active_part->_flush_dirty_state();
        }

        current_delta = p_delta;
        current_consists = _discover_consists();

        if (current_consists.size() > 1) {
            // Actually worth farming out: more than one independent consist to compute.
            const int64_t group_id = WorkerThreadPool::get_singleton()->add_group_task(
                    Callable(this, "_step_consist_task"), static_cast<int32_t>(current_consists.size()), -1, true,
                    "TrainPhysicsServer::step_consists");
            WorkerThreadPool::get_singleton()->wait_for_group_task_completion(group_id);
        } else if (current_consists.size() == 1) {
            // Only one consist (the common case: a single train, or everything coupled into one
            // consist) — nothing to parallelize against, so skip WorkerThreadPool dispatch/wake-up
            // latency entirely and step it inline. That overhead is pure loss here and, left in,
            // measurably slows down the effective simulation rate tick-over-tick for the common case.
            _step_consist_task(0);
        }
        current_consists.clear();

        // Parts: some (e.g. TrainWheels) drive Godot Node3D transforms directly — Godot scene-tree API,
        // must stay on the main thread.
        for (auto *const active_part: active_parts) {
            if (active_part->get_enabled()) {
                active_part->_process_mover_thread_safe(p_delta);
            }
        }

        // Post process (update Godot dictionaries and emit signals safely on main thread)
        for (auto *const active_controller: active_controllers) {
            active_controller->_post_process_mover_sync();
        }

        for (auto *const active_part: active_parts) {
            if (active_part->get_enabled()) {
                active_part->_post_process_mover_sync();
            }
        }
    }
} // namespace godot
