#include "DriverSystem.hpp"
#include "simulation/SimulationServer.hpp"
#include "vehicles/base/VehicleServer.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    const char *DriverSystem::driver_timetable_changed_signal = "driver_timetable_changed";
    const char *DriverSystem::driver_vehicle_attached_signal = "driver_vehicle_attached";
    const char *DriverSystem::driver_freed_signal = "driver_freed";
    const char *DriverSystem::vehicle_driven_changed_signal = "vehicle_driven_changed";

    void DriverSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("driver_create"), &DriverSystem::driver_create);
        ClassDB::bind_method(D_METHOD("driver_free", "driver"), &DriverSystem::driver_free);
        ClassDB::bind_method(D_METHOD("driver_get_rids"), &DriverSystem::driver_get_rids);
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
        ClassDB::bind_method(
                D_METHOD("vehicle_set_control_active", "vehicle", "active"), &DriverSystem::vehicle_set_control_active);
        ClassDB::bind_method(
                D_METHOD("vehicle_is_control_active", "vehicle"), &DriverSystem::vehicle_is_control_active);
        ClassDB::bind_method(D_METHOD("vehicle_is_driven", "vehicle"), &DriverSystem::vehicle_is_driven);
        ClassDB::bind_method(
                D_METHOD("driver_get_timetable_state", "driver"), &DriverSystem::driver_get_timetable_state);
        ClassDB::bind_method(
                D_METHOD("driver_report_timetable_changed", "driver"), &DriverSystem::driver_report_timetable_changed);
        ClassDB::bind_method(D_METHOD("driver_get_state", "driver"), &DriverSystem::driver_get_state);
        ADD_SIGNAL(MethodInfo(driver_timetable_changed_signal, PropertyInfo(Variant::RID, "driver")));
        ADD_SIGNAL(MethodInfo(
                driver_vehicle_attached_signal, PropertyInfo(Variant::RID, "driver"),
                PropertyInfo(Variant::RID, "vehicle")));
        ADD_SIGNAL(MethodInfo(driver_freed_signal, PropertyInfo(Variant::RID, "driver")));
        ADD_SIGNAL(MethodInfo(
                vehicle_driven_changed_signal, PropertyInfo(Variant::RID, "vehicle"),
                PropertyInfo(Variant::BOOL, "driven")));
    }

    /// A freed vehicle leaves its driver without one. No explicit disconnect: callable_mp reports
    /// this instance as the callable's object, so the engine drops the connection when it dies.
    DriverSystem::DriverSystem() {
        VehicleServer *vehicles = VehicleServer::get_instance();
        ERR_FAIL_NULL(vehicles);
        vehicles->connect(VehicleServer::vehicle_freed_signal, callable_mp(this, &DriverSystem::_on_vehicle_freed));
    }

    DriverSystem::~DriverSystem() {
        _set_processing(false);
    }

    /// Scheduled, it holds the runtime's clock, so the time it waits for passes
    void DriverSystem::_set_processing(const bool p_processing) {
        if (processing == p_processing) {
            return;
        }
        SimulationServer *runtime = SimulationServer::get_instance();
        ERR_FAIL_NULL(runtime);
        processing = p_processing;
        if (p_processing) {
            runtime->clock_subscribe(callable_mp(this, &DriverSystem::_process_updates));
            return;
        }
        runtime->clock_unsubscribe(callable_mp(this, &DriverSystem::_process_updates));
    }

    /// Only the updates whose time has come are taken, and only those scheduled before the pass:
    /// what an update schedules, even for now, runs on the next frame
    void DriverSystem::_process_updates(double /* p_seconds */) {
        const SimulationServer *runtime = SimulationServer::get_instance();
        ERR_FAIL_NULL(runtime);
        const double time = runtime->simulation_get_time();
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
        _set_processing(!updates.empty());
    }

    void DriverSystem::vehicle_set_control_active(const RID &p_vehicle, const bool p_active) {
        if (!(player_controlled_vehicles.has(p_vehicle) == p_active)) {
            return;
        }
        const bool was_driven = vehicle_is_driven(p_vehicle);
        if (!p_active) {
            player_controlled_vehicles.insert(p_vehicle);
            _report_driven(p_vehicle, was_driven);
            return;
        }
        player_controlled_vehicles.erase(p_vehicle);
        _report_driven(p_vehicle, was_driven);
        // taken back: the delegate learns it drives a vehicle a player left as it is now
        // (TController::TakeControl(), Driver.cpp:5700-5712)
        const RID driver_rid = vehicle_get_driver(p_vehicle);
        const DriverData *data = drivers.getptr(driver_rid);
        if (data != nullptr && data->delegate.is_valid()) {
            // held: the delegate may change the drivers, which rehashes the table
            const Ref<DriverDelegate> delegate = data->delegate;
            delegate->control_taken(driver_rid);
        }
    }

    bool DriverSystem::vehicle_is_control_active(const RID &p_vehicle) const {
        return drivers_by_vehicle.has(p_vehicle) && !player_controlled_vehicles.has(p_vehicle);
    }

    bool DriverSystem::vehicle_is_driven(const RID &p_vehicle) const {
        return drivers_by_vehicle.has(p_vehicle) || player_controlled_vehicles.has(p_vehicle);
    }

    /// Announces the vehicle's change of being driven, after the operation that changed it
    void DriverSystem::_report_driven(const RID &p_vehicle, const bool p_was_driven) {
        const bool driven = vehicle_is_driven(p_vehicle);
        if (!(driven == p_was_driven)) {
            // a vehicle left standing has switched its simulation off; whoever takes it - the AI
            // or the player - wakes it
            if (RailVehicleServer *vehicles = RailVehicleServer::get_instance(); driven && vehicles != nullptr) {
                vehicles->vehicle_wake(p_vehicle);
            }
            emit_signal(vehicle_driven_changed_signal, p_vehicle, driven);
        }
    }

    void DriverSystem::driver_schedule_update(const RID &p_driver, const double p_seconds) {
        DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        data->update_sequence = next_sequence++;
        const SimulationServer *runtime = SimulationServer::get_instance();
        ERR_FAIL_NULL(runtime);
        updates.push(
                UpdateEntry{runtime->simulation_get_time() + MAX(p_seconds, 0.0), data->update_sequence, p_driver});
        _set_processing(!updates.empty());
    }

    void DriverSystem::_on_vehicle_freed(const RID &p_vehicle) {
        player_controlled_vehicles.erase(p_vehicle);
        const RID *driver = drivers_by_vehicle.getptr(p_vehicle);
        if (driver == nullptr) {
            return;
        }
        const RID driver_rid = *driver;
        if (DriverData *data = drivers.getptr(driver_rid); data != nullptr) {
            data->vehicle = RID();
        }
        drivers_by_vehicle.erase(p_vehicle);
        emit_signal(driver_vehicle_attached_signal, driver_rid, RID());
    }

    RID DriverSystem::driver_create() {
        const RID rid = UtilityFunctions::rid_from_int64(UtilityFunctions::rid_allocate_id());
        drivers.insert(rid, DriverData());
        return rid;
    }

    TypedArray<RID> DriverSystem::driver_get_rids() const {
        TypedArray<RID> result;
        for (const KeyValue<RID, DriverData> &entry: drivers) {
            result.append(entry.key);
        }
        return result;
    }

    void DriverSystem::driver_free(const RID &p_driver) {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL(data);
        const DriverData freed = *data;
        drivers.erase(p_driver);
        if (freed.vehicle.is_valid()) {
            const bool was_driven = vehicle_is_driven(freed.vehicle);
            drivers_by_vehicle.erase(freed.vehicle);
            _report_driven(freed.vehicle, was_driven);
        }
        if (freed.delegate.is_valid()) {
            freed.delegate->driver_detached(p_driver);
        }
        emit_signal(driver_freed_signal, p_driver);
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
        const RID previous = data->vehicle;
        const bool previous_was_driven = vehicle_is_driven(previous);
        const bool was_driven = vehicle_is_driven(p_vehicle);
        if (previous.is_valid()) {
            drivers_by_vehicle.erase(previous);
        }
        data->vehicle = p_vehicle;
        if (p_vehicle.is_valid()) {
            drivers_by_vehicle.insert(p_vehicle, p_driver);
        }
        emit_signal(driver_vehicle_attached_signal, p_driver, p_vehicle);
        if (previous.is_valid() && !(previous == p_vehicle)) {
            _report_driven(previous, previous_was_driven);
        }
        if (p_vehicle.is_valid()) {
            _report_driven(p_vehicle, was_driven);
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

    Dictionary DriverSystem::driver_get_timetable_state(const RID &p_driver) const {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL_V(data, Dictionary());
        return data->delegate.is_valid() ? data->delegate->get_timetable_state(p_driver) : Dictionary();
    }

    void DriverSystem::driver_report_timetable_changed(const RID &p_driver) {
        ERR_FAIL_COND(!drivers.has(p_driver));
        emit_signal(driver_timetable_changed_signal, p_driver);
    }

    Dictionary DriverSystem::driver_get_state(const RID &p_driver) const {
        const DriverData *data = drivers.getptr(p_driver);
        ERR_FAIL_NULL_V(data, Dictionary());
        return data->delegate.is_valid() ? data->delegate->get_state(p_driver) : Dictionary();
    }
} // namespace godot
