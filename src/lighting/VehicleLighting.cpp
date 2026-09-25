#include "VehicleLighting.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleLighting::_bind_methods() {
        BIND_PROPERTY(VehicleLighting, Variant::COLOR, head_light_color, "head_light");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_dimmed_multiplier, "head_light");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_normal_multiplier, "head_light");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_high_beam_dimmed_multiplier, "head_light/high_beam");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_high_beam_normal_multiplier, "head_light/high_beam");
        BIND_PROPERTY(VehicleLighting, Variant::INT, lights_default_selector_position, "lights");
        BIND_PROPERTY(VehicleLighting, Variant::BOOL, lights_wrap_selector, "lights");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleLighting, Variant::ARRAY, lights_list, "lights", PROPERTY_HINT_TYPE_STRING, "LightListItem");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, light_source, "light", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, source_generator_engine, "source/generator", PROPERTY_HINT_ENUM,
                "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                "DieselElectric,Main");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, source_accumulator_max_voltage, "source/accumulator");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, light_alternative_source, "light/alternative", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, light_alternative_max_voltage, "light/alternative");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, light_alternative_capacity, "light/alternative");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, source_accumulator_recharge_source, "source/accumulator",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(VehicleLighting, Variant::INT, instrument_type);
        ClassDB::bind_method(
                D_METHOD("increase_light_selector_position"), &VehicleLighting::increase_light_selector_position);
        ClassDB::bind_method(
                D_METHOD("decrease_light_selector_position"), &VehicleLighting::decrease_light_selector_position);
        ClassDB::bind_method(D_METHOD("light", "light", "enabled"), &VehicleLighting::light);
        ClassDB::bind_method(D_METHOD("light_switch", "light", "enabled"), &VehicleLighting::light_switch);
        ClassDB::bind_method(D_METHOD("roof_light", "enabled"), &VehicleLighting::roof_light);
        ClassDB::bind_method(D_METHOD("devices_light", "enabled"), &VehicleLighting::devices_light);
        ClassDB::bind_method(D_METHOD("headlights_dim", "enabled"), &VehicleLighting::headlights_dim);
        ClassDB::bind_method(D_METHOD("get_headlights_dimmed"), &VehicleLighting::get_headlights_dimmed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "headlights_dimmed", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_headlights_dimmed");
        ADD_SIGNAL(MethodInfo(selector_position_changed_signal, PropertyInfo(Variant::INT, "position")));

        ClassDB::bind_method(D_METHOD("get_position"), &VehicleLighting::get_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "position", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_position");
        ClassDB::bind_method(D_METHOD("get_power"), &VehicleLighting::get_power);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power");
        ClassDB::bind_method(D_METHOD("get_power_source"), &VehicleLighting::get_power_source);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "power_source", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_power_source");
        ClassDB::bind_method(D_METHOD("get_front_headlight_upper_enabled"), &VehicleLighting::get_front_headlight_upper_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "front_headlight_upper_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_front_headlight_upper_enabled");
        ClassDB::bind_method(D_METHOD("get_front_headlight_left_enabled"), &VehicleLighting::get_front_headlight_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "front_headlight_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_front_headlight_left_enabled");
        ClassDB::bind_method(D_METHOD("get_front_headlight_right_enabled"), &VehicleLighting::get_front_headlight_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "front_headlight_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_front_headlight_right_enabled");
        ClassDB::bind_method(D_METHOD("get_front_redmarker_left_enabled"), &VehicleLighting::get_front_redmarker_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "front_redmarker_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_front_redmarker_left_enabled");
        ClassDB::bind_method(D_METHOD("get_front_redmarker_right_enabled"), &VehicleLighting::get_front_redmarker_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "front_redmarker_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_front_redmarker_right_enabled");
        ClassDB::bind_method(D_METHOD("get_rear_headlight_upper_enabled"), &VehicleLighting::get_rear_headlight_upper_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "rear_headlight_upper_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rear_headlight_upper_enabled");
        ClassDB::bind_method(D_METHOD("get_rear_headlight_left_enabled"), &VehicleLighting::get_rear_headlight_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "rear_headlight_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rear_headlight_left_enabled");
        ClassDB::bind_method(D_METHOD("get_rear_headlight_right_enabled"), &VehicleLighting::get_rear_headlight_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "rear_headlight_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rear_headlight_right_enabled");
        ClassDB::bind_method(D_METHOD("get_rear_redmarker_left_enabled"), &VehicleLighting::get_rear_redmarker_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "rear_redmarker_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rear_redmarker_left_enabled");
        ClassDB::bind_method(D_METHOD("get_rear_redmarker_right_enabled"), &VehicleLighting::get_rear_redmarker_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "rear_redmarker_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_rear_redmarker_right_enabled");
        ClassDB::bind_method(D_METHOD("get_active_headlight_upper_enabled"), &VehicleLighting::get_active_headlight_upper_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active_headlight_upper_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active_headlight_upper_enabled");
        ClassDB::bind_method(D_METHOD("get_active_headlight_left_enabled"), &VehicleLighting::get_active_headlight_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active_headlight_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active_headlight_left_enabled");
        ClassDB::bind_method(D_METHOD("get_active_headlight_right_enabled"), &VehicleLighting::get_active_headlight_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active_headlight_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active_headlight_right_enabled");
        ClassDB::bind_method(D_METHOD("get_active_redmarker_left_enabled"), &VehicleLighting::get_active_redmarker_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active_redmarker_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active_redmarker_left_enabled");
        ClassDB::bind_method(D_METHOD("get_active_redmarker_right_enabled"), &VehicleLighting::get_active_redmarker_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "active_redmarker_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_active_redmarker_right_enabled");
        ClassDB::bind_method(D_METHOD("get_opposite_headlight_upper_enabled"), &VehicleLighting::get_opposite_headlight_upper_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "opposite_headlight_upper_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_opposite_headlight_upper_enabled");
        ClassDB::bind_method(D_METHOD("get_opposite_headlight_left_enabled"), &VehicleLighting::get_opposite_headlight_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "opposite_headlight_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_opposite_headlight_left_enabled");
        ClassDB::bind_method(D_METHOD("get_opposite_headlight_right_enabled"), &VehicleLighting::get_opposite_headlight_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "opposite_headlight_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_opposite_headlight_right_enabled");
        ClassDB::bind_method(D_METHOD("get_opposite_redmarker_left_enabled"), &VehicleLighting::get_opposite_redmarker_left_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "opposite_redmarker_left_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_opposite_redmarker_left_enabled");
        ClassDB::bind_method(D_METHOD("get_opposite_redmarker_right_enabled"), &VehicleLighting::get_opposite_redmarker_right_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "opposite_redmarker_right_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_opposite_redmarker_right_enabled");
        ClassDB::bind_method(D_METHOD("get_devices_light_enabled"), &VehicleLighting::get_devices_light_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "devices_light_enabled", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_devices_light_enabled");
        ClassDB::bind_method(D_METHOD("get_roof_light_level"), &VehicleLighting::get_roof_light_level);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "roof_light_level", PROPERTY_HINT_NONE, "",
                             PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_roof_light_level");
    }

    const char *VehicleLighting::selector_position_changed_signal = "selector_position_changed";



    void VehicleLighting::_register_commands() {
        register_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        register_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        register_command("light", Callable(this, "light"));
        register_command("light_switch", Callable(this, "light_switch"));
        register_command("roof_light", Callable(this, "roof_light"));
        register_command("devices_light", Callable(this, "devices_light"));
        register_command("headlights_dim", Callable(this, "headlights_dim"));
        VehicleComponent::_register_commands();
    }

    void VehicleLighting::_unregister_commands() {
        unregister_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        unregister_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        unregister_command("light", Callable(this, "light"));
        unregister_command("light_switch", Callable(this, "light_switch"));
        unregister_command("roof_light", Callable(this, "roof_light"));
        unregister_command("devices_light", Callable(this, "devices_light"));
        unregister_command("headlights_dim", Callable(this, "headlights_dim"));
        VehicleComponent::_unregister_commands();
    }
} // namespace godot
