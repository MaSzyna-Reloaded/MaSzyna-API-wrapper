#include "VehicleSpeedControl.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleSpeedControl::_bind_methods() {
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, speed_control_enabled);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, delay);
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, impulse_lever);
        BIND_PROPERTY_W_HINT(
                VehicleSpeedControl, Variant::INT, disables_on, PROPERTY_HINT_FLAGS,
                "Main Controller Movement,Braking");
        BIND_PROPERTY(VehicleSpeedControl, Variant::PACKED_FLOAT64_ARRAY, preset_speeds);
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, override_manual_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, initial_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, full_power_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, start_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, velocity_step);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, power_step);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, min_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, max_power);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, min_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, max_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, offset);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, proportional_gain_positive);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, proportional_gain_negative);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, integral_gain_positive);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, integral_gain_negative);
        BIND_PROPERTY(VehicleSpeedControl, Variant::BOOL, brake_intervention);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, brake_intervention_max_velocity);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, power_up_speed);
        BIND_PROPERTY(VehicleSpeedControl, Variant::FLOAT, power_down_speed);

        ClassDB::bind_method(D_METHOD("get_active"), &VehicleSpeedControl::get_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active");
        ClassDB::bind_method(D_METHOD("get_desired_velocity"), &VehicleSpeedControl::get_desired_velocity);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "desired_velocity", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_desired_velocity");
        ClassDB::bind_method(D_METHOD("get_desired_power"), &VehicleSpeedControl::get_desired_power);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "desired_power", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_desired_power");
        ClassDB::bind_method(D_METHOD("speed_control_increase"), &VehicleSpeedControl::speed_control_increase);
        ClassDB::bind_method(D_METHOD("speed_control_decrease"), &VehicleSpeedControl::speed_control_decrease);
        ClassDB::bind_method(
                D_METHOD("speed_control_power_increase"), &VehicleSpeedControl::speed_control_power_increase);
        ClassDB::bind_method(
                D_METHOD("speed_control_power_decrease"), &VehicleSpeedControl::speed_control_power_decrease);
        ClassDB::bind_method(D_METHOD("speed_control_button", "button"), &VehicleSpeedControl::speed_control_button);
        ClassDB::bind_method(D_METHOD("speed_control_set", "velocity"), &VehicleSpeedControl::speed_control_set);
        ClassDB::bind_method(D_METHOD("get_set_velocity"), &VehicleSpeedControl::get_set_velocity);
        ClassDB::bind_method(D_METHOD("get_selected_velocity"), &VehicleSpeedControl::get_selected_velocity);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::FLOAT, "selected_velocity", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_selected_velocity");
    }

    void VehicleSpeedControl::_register_commands() {
        register_command("speed_control_increase", Callable(this, "speed_control_increase"));
        register_command("speed_control_decrease", Callable(this, "speed_control_decrease"));
        register_command("speed_control_power_increase", Callable(this, "speed_control_power_increase"));
        register_command("speed_control_power_decrease", Callable(this, "speed_control_power_decrease"));
        register_command("speed_control_button", Callable(this, "speed_control_button"));
        register_command("speed_control_set", Callable(this, "speed_control_set"));
    }

    void VehicleSpeedControl::_unregister_commands() {
        unregister_command("speed_control_increase", Callable(this, "speed_control_increase"));
        unregister_command("speed_control_decrease", Callable(this, "speed_control_decrease"));
        unregister_command("speed_control_power_increase", Callable(this, "speed_control_power_increase"));
        unregister_command("speed_control_power_decrease", Callable(this, "speed_control_power_decrease"));
        unregister_command("speed_control_button", Callable(this, "speed_control_button"));
        unregister_command("speed_control_set", Callable(this, "speed_control_set"));
    }
} // namespace godot
