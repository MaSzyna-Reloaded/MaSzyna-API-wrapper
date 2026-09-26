#include "../physics/RailVehicleServer.hpp"
#include "DriverSystem.hpp"
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
    }

    /// A freed vehicle leaves its driver without one. No explicit disconnect: callable_mp reports this
    /// instance as the callable's object, so the engine drops the connection when it dies.
    DriverSystem::DriverSystem() {
        RailVehicleServer *vehicles = RailVehicleServer::get_instance();
        ERR_FAIL_NULL(vehicles);
        vehicles->connect(RailVehicleServer::vehicle_freed_signal, callable_mp(this, &DriverSystem::_on_vehicle_freed));
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
