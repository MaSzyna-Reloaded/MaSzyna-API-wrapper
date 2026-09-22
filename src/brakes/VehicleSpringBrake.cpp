#include "VehicleSpringBrake.hpp"

namespace godot {
    void VehicleSpringBrake::_bind_methods() {
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_actuator_chamber_volume, "spring/actuator")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_actuator_max_filling_force, "spring/actuator")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, pressure_force_coefficient)
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_actuator_preload_pressure, "spring/actuator")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, spring_full_balance_pressure, "spring")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, brake_signal_released_state_pressure, "brake_signal")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, brake_signal_braked_state_pressure, "brake_signal")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, valve_cross_section_actuator_discharge, "valve_cross_section")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, valve_cross_section_actuator_charge, "valve_cross_section")
        BIND_PROPERTY(VehicleSpringBrake, Variant::FLOAT, valve_cross_section_pneumatic_brake, "valve_cross_section")
        BIND_PROPERTY(VehicleSpringBrake, Variant::INT, required_coupler_connection_method)

        ClassDB::bind_method(D_METHOD("set_spring_brake_active", "active"), &VehicleSpringBrake::set_spring_brake_active);
        ClassDB::bind_method(
                D_METHOD("set_spring_brake_enabled", "enabled"), &VehicleSpringBrake::set_spring_brake_enabled);
        ClassDB::bind_method(D_METHOD("spring_brake_release"), &VehicleSpringBrake::spring_brake_release);

        ClassDB::bind_method(D_METHOD("get_ready"), &VehicleSpringBrake::get_ready);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "ready", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_ready");
        ClassDB::bind_method(D_METHOD("get_shut_off"), &VehicleSpringBrake::get_shut_off);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "shut_off", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_shut_off");
        ClassDB::bind_method(D_METHOD("get_active"), &VehicleSpringBrake::get_active);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active");
        ClassDB::bind_method(D_METHOD("get_cylinder_pressure"), &VehicleSpringBrake::get_cylinder_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "cylinder_pressure", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_cylinder_pressure");
    }

    void VehicleSpringBrake::_register_commands() {
        register_command("set_spring_brake_active", Callable(this, "set_spring_brake_active"));
        register_command("set_spring_brake_enabled", Callable(this, "set_spring_brake_enabled"));
        register_command("spring_brake_release", Callable(this, "spring_brake_release"));
    }

    void VehicleSpringBrake::_unregister_commands() {
        unregister_command("set_spring_brake_active", Callable(this, "set_spring_brake_active"));
        unregister_command("set_spring_brake_enabled", Callable(this, "set_spring_brake_enabled"));
        unregister_command("spring_brake_release", Callable(this, "spring_brake_release"));
    }
} // namespace godot
