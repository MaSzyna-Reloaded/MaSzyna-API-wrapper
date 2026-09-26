#include "../core/MaszynaRuntime.hpp"
#include "../physics/RailVehicleServer.hpp"
#include "DriverSystem.hpp"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void DriverSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("driver_create"), &DriverSystem::driver_create);
        ClassDB::bind_method(D_METHOD("driver_free", "driver"), &DriverSystem::driver_free);
        ClassDB::bind_method(
                D_METHOD("driver_attach_delegate", "driver", "delegate"), &DriverSystem::driver_attach_delegate);
        ClassDB::bind_method(D_METHOD("driver_get_delegate", "driver"), &DriverSystem::driver_get_delegate);
        ClassDB::bind_method(
                D_METHOD("driver_attach_vehicle", "driver", "vehicle"), &DriverSystem::driver_attach_vehicle);
        ClassDB::bind_method(D_METHOD("driver_get_vehicle", "driver"), &DriverSystem::driver_get_vehicle);
        ClassDB::bind_method(D_METHOD("vehicle_get_driver", "vehicle"), &DriverSystem::vehicle_get_driver);
        ClassDB::bind_method(
                D_METHOD("driver_send_command", "driver", "command", "value1", "value2", "position"),
                &DriverSystem::driver_send_command, DEFVAL(Vector3()));
        ClassDB::bind_method(
                D_METHOD("driver_schedule_update", "driver", "seconds"), &DriverSystem::driver_schedule_update);
    }

    /// A freed vehicle leaves its driver without one; the time follows the runtime. No explicit
    /// disconnect: callable_mp reports this instance as the callable's object, so the engine drops
    /// the connections when it dies.
    DriverSystem::DriverSystem() {
        RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        ERR_FAIL_NULL(vehicles);
        vehicles->connect(RailVehicleServer::vehicle_freed_signal, callable_mp(this, &DriverSystem::_on_vehicle_freed));
        MaszynaRuntime *runtime = MaszynaRuntime::get_instance();
        ERR_FAIL_NULL(runtime);
        simulation_speed = runtime->get_simulation_speed();
        runtime->connect(MaszynaRuntime::paused_signal, callable_mp(this, &DriverSystem::_refresh_processing));
        runtime->connect(MaszynaRuntime::unpaused_signal, callable_mp(this, &DriverSystem::_refresh_processing));
        runtime->connect(
                MaszynaRuntime::simulation_speed_changed_signal,
                callable_mp(this, &DriverSystem::_on_simulation_speed_changed));
    }

    DriverSystem::~DriverSystem() {
        _set_processing(false);
    }

    void DriverSystem::_on_simulation_speed_changed() {
        const MaszynaRuntime *runtime = MaszynaRuntime::get_instance();
        ERR_FAIL_NULL(runtime);
        simulation_speed = runtime->get_simulation_speed();
    }

    /// Processes while an update is scheduled and the runtime is not paused
    void DriverSystem::_refresh_processing() {
        const MaszynaRuntime *runtime = MaszynaRuntime::get_instance();
        _set_processing(!updates.empty() && runtime != nullptr && !runtime->is_paused());
    }

    void DriverSystem::_set_processing(const bool p_processing) {
        if (processing == p_processing) {
            return;
        }
        SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        if (tree == nullptr) {
            return;
        }
        processing = p_processing;
        if (p_processing) {
            tree->connect("process_frame", callable_mp(this, &DriverSystem::_process_updates));
            return;
        }
        tree->disconnect("process_frame", callable_mp(this, &DriverSystem::_process_updates));
    }

    /// Only the updates whose time has come are taken, and only those scheduled before the pass:
    /// what an update schedules, even for now, runs on the next frame
    void DriverSystem::_process_updates() {
        const SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
        ERR_FAIL_NULL(tree);
        time += MIN(tree->get_root()->get_process_delta_time() * simulation_speed, MAX_FRAME_TIME);
        const uint64_t pass_end = next_sequence;
        while (!updates.empty() && updates.top().time <= time && updates.top().sequence < pass_end) {
            const UpdateEntry entry = updates.top();
            updates.pop();
            DriverData *data = drivers.getptr(entry.driver);
            if (data == nullptr || !(data->update_sequence == entry.sequence)) {
                continue;
            }
            data->update_sequence = 0;
            // taken before it runs: the update may create or free drivers, which rehashes the table
            const Ref<DriverDelegate> delegate = data->delegate;
            if (delegate.is_valid()) {
                delegate->update(entry.driver);
            }
        }
        _refresh_processing();
    }

    void DriverSystem::driver_schedule_update(const RID &p_driver, const double p_seconds) {
        DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        data->update_sequence = next_sequence++;
        updates.push(UpdateEntry{time + MAX(p_seconds, 0.0), data->update_sequence, p_driver});
        _refresh_processing();
    }

    void DriverSystem::_on_vehicle_freed(const RID &p_vehicle) {
        const RID *driver = drivers_by_vehicle.getptr(p_vehicle);
        if (driver == nullptr) {
            return;
        }
        if (DriverData *data = drivers.getptr(*driver); data != nullptr) {
            data->vehicle = RID();
        }
        drivers_by_vehicle.erase(p_vehicle);
    }

    RID DriverSystem::driver_create() {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        drivers.insert(rid, DriverData());
        return rid;
    }

    void DriverSystem::driver_free(const RID &p_driver) {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        const DriverData freed = *data;
        drivers.erase(p_driver);
        if (freed.vehicle.is_valid()) {
            drivers_by_vehicle.erase(freed.vehicle);
        }
        if (freed.delegate.is_valid()) {
            freed.delegate->driver_detached(p_driver);
        }
    }

    void DriverSystem::driver_attach_delegate(const RID &p_driver, const Ref<DriverDelegate> &p_delegate) {
        DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        const Ref<DriverDelegate> previous = data->delegate;
        data->delegate = p_delegate;
        if (previous.is_valid()) {
            previous->driver_detached(p_driver);
        }
        if (p_delegate.is_valid()) {
            p_delegate->driver_attached(p_driver);
        }
    }

    Ref<DriverDelegate> DriverSystem::driver_get_delegate(const RID &p_driver) const {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL_V(data, Ref<DriverDelegate>());
        return data->delegate;
    }

    void DriverSystem::driver_attach_vehicle(const RID &p_driver, const RID &p_vehicle) {
        DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        ERR_FAIL_COND_MSG(
                drivers_by_vehicle.has(p_vehicle) && !(drivers_by_vehicle[p_vehicle] == p_driver),
                "The vehicle has a driver already.");
        if (data->vehicle.is_valid()) {
            drivers_by_vehicle.erase(data->vehicle);
        }
        data->vehicle = p_vehicle;
        if (p_vehicle.is_valid()) {
            drivers_by_vehicle.insert(p_vehicle, p_driver);
        }
    }

    RID DriverSystem::driver_get_vehicle(const RID &p_driver) const {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL_V(data, RID());
        return data->vehicle;
    }

    RID DriverSystem::vehicle_get_driver(const RID &p_vehicle) const {
        const RID *driver = drivers_by_vehicle.getptr(p_vehicle);
        return driver != nullptr ? *driver : RID();
    }

    void DriverSystem::driver_send_command(
            const RID &p_driver, const String &p_command, const double p_value1, const double p_value2,
            const Vector3 &p_position) {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        const Ref<DriverDelegate> delegate = data->delegate;
        if (delegate.is_null()) {
            return;
        }
        delegate->handle_command(p_driver, p_command, p_value1, p_value2, p_position);
    }
} // namespace godot
