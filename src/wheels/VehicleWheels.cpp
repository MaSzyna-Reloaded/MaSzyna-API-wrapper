#include "VehicleWheels.hpp"
#include "../core/VehicleController.hpp"
#include "../physics/RailVehicleServer.hpp"
#include <godot_cpp/core/math.hpp>

namespace godot {
    /* RailVehicleServer's track-offset distance is rear-relative (see its process_movement()),
     * so a positive distance samples toward the vehicle's rear - the sign here is deliberate and
     * was confirmed live: getting it wrong flips the whole vehicle the moment it starts moving
     * (test_rail_vehicle_idle_orientation_regression.gd). */
    Transform3D VehicleWheels::get_bogie_transform(const Bogie p_bogie) const {
        const VehicleController *controller = get_controller();
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (controller == nullptr || server == nullptr) {
            return Transform3D();
        }
        const double half_spacing = 0.5 * get_bogie_pivot_spacing();
        return server->vehicle_get_transform_at_distance(
                controller->get_rid(), p_bogie == BOGIE_FRONT ? -half_spacing : half_spacing);
    }

    void VehicleWheels::_bind_methods() {
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, powered_wheel_diameter);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, front_rolling_wheel_diameter);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, rear_rolling_wheel_diameter);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, axle_inertial_moment);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, track_width);
        BIND_PROPERTY(VehicleWheels, Variant::STRING, axle_arrangement);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, bogie_axle_spacing);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, bogie_pivot_spacing);
        BIND_PROPERTY(VehicleWheels, Variant::FLOAT, minimum_curve_radius);
        BIND_PROPERTY_W_HINT(VehicleWheels, Variant::INT, bearing_type, PROPERTY_HINT_ENUM, "Slide,Roll");

        ClassDB::bind_method(D_METHOD("get_bogie_transform", "bogie"), &VehicleWheels::get_bogie_transform);
        BIND_ENUM_CONSTANT(BOGIE_FRONT);
        BIND_ENUM_CONSTANT(BOGIE_REAR);
        BIND_ENUM_CONSTANT(BEARING_TYPE_SLIDE);
        BIND_ENUM_CONSTANT(BEARING_TYPE_ROLL);

        ClassDB::bind_method(D_METHOD("get_angle_front_deg"), &VehicleWheels::get_angle_front_deg);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "angle_front_deg", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_angle_front_deg");
        ClassDB::bind_method(D_METHOD("get_angle_powered_deg"), &VehicleWheels::get_angle_powered_deg);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "angle_powered_deg", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_angle_powered_deg");
        ClassDB::bind_method(D_METHOD("get_angle_rear_deg"), &VehicleWheels::get_angle_rear_deg);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "angle_rear_deg", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_angle_rear_deg");
        ClassDB::bind_method(D_METHOD("get_rotation_speed_rps"), &VehicleWheels::get_rotation_speed_rps);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rotation_speed_rps", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rotation_speed_rps");
        ClassDB::bind_method(D_METHOD("get_rotation_acceleration_rps2"), &VehicleWheels::get_rotation_acceleration_rps2);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "rotation_acceleration_rps2", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rotation_acceleration_rps2");
        ClassDB::bind_method(D_METHOD("get_slipping"), &VehicleWheels::get_slipping);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "slipping", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_slipping");
        ClassDB::bind_method(D_METHOD("get_flat"), &VehicleWheels::get_flat);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "flat", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_flat");
    }
} // namespace godot
