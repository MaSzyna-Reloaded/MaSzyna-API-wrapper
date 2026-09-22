#include "BaseVehiclePhysicsServer.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    // NOLINTBEGIN(misc-unused-parameters) - a contract's defaults use none of their arguments
    void BaseVehiclePhysicsServer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("vehicle_create", "type_name", "name", "initial_velocity", "cabin_number"),
                &BaseVehiclePhysicsServer::vehicle_create);
        ClassDB::bind_method(D_METHOD("vehicle_free", "vehicle"), &BaseVehiclePhysicsServer::vehicle_free);
        ClassDB::bind_method(D_METHOD("vehicle_exists", "vehicle"), &BaseVehiclePhysicsServer::vehicle_exists);
        ClassDB::bind_method(D_METHOD("vehicle_is_active", "vehicle"), &BaseVehiclePhysicsServer::vehicle_is_active);
        ClassDB::bind_method(
                D_METHOD("vehicle_get_velocity", "vehicle"), &BaseVehiclePhysicsServer::vehicle_get_velocity);
        ClassDB::bind_method(D_METHOD("vehicle_get_speed", "vehicle"), &BaseVehiclePhysicsServer::vehicle_get_speed);
        ClassDB::bind_method(
                D_METHOD("vehicle_set_location", "vehicle", "position"),
                &BaseVehiclePhysicsServer::vehicle_set_location);
        ClassDB::bind_method(
                D_METHOD("vehicle_compute_forces", "vehicle", "delta"),
                &BaseVehiclePhysicsServer::vehicle_compute_forces);
        ClassDB::bind_method(
                D_METHOD("vehicle_compute_movement", "vehicle", "delta", "kind"),
                &BaseVehiclePhysicsServer::vehicle_compute_movement);

        BIND_ENUM_CONSTANT(MOVEMENT_FAST);
        BIND_ENUM_CONSTANT(MOVEMENT_FULL);
        BIND_ENUM_CONSTANT(VEHICLE_END_FRONT);
        BIND_ENUM_CONSTANT(VEHICLE_END_REAR);
    }

    /* The contract has no behaviour of its own: an implementation that forgets one of these says
     * so out loud rather than silently doing nothing. */
    static void _not_implemented(const char *p_method) {
        UtilityFunctions::push_error(
                String("BaseVehiclePhysicsServer::") + p_method + "() is not implemented by this backend");
    }

    RID BaseVehiclePhysicsServer::vehicle_create(
            const String &p_type_name, const String &p_name, const double p_initial_velocity,
            const int p_cabin_number) {
        _not_implemented("vehicle_create");
        return RID();
    }

    void BaseVehiclePhysicsServer::vehicle_free(const RID &p_vehicle) {
        _not_implemented("vehicle_free");
    }

    bool BaseVehiclePhysicsServer::vehicle_exists(const RID &p_vehicle) const {
        _not_implemented("vehicle_exists");
        return false;
    }

    bool BaseVehiclePhysicsServer::vehicle_is_active(const RID &p_vehicle) const {
        _not_implemented("vehicle_is_active");
        return false;
    }

    double BaseVehiclePhysicsServer::vehicle_get_velocity(const RID &p_vehicle) const {
        _not_implemented("vehicle_get_velocity");
        return 0.0;
    }

    double BaseVehiclePhysicsServer::vehicle_get_speed(const RID &p_vehicle) const {
        _not_implemented("vehicle_get_speed");
        return 0.0;
    }

    void BaseVehiclePhysicsServer::vehicle_set_location(const RID &p_vehicle, const Vector3 &p_position) {
        _not_implemented("vehicle_set_location");
    }

    void BaseVehiclePhysicsServer::vehicle_compute_forces(const RID &p_vehicle, const double p_delta) {
        _not_implemented("vehicle_compute_forces");
    }

    double BaseVehiclePhysicsServer::vehicle_compute_movement(const RID &p_vehicle, const double p_delta, const MovementKind p_kind) {
        _not_implemented("vehicle_compute_movement");
        return 0.0;
    }

    // NOLINTEND(misc-unused-parameters)
} // namespace godot
