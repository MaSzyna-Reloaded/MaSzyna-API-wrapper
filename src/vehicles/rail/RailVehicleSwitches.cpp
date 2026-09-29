#include "RailVehicleSwitches.hpp"

namespace godot {
    void RailVehicleSwitches::_bind_methods() {
        BIND_PROPERTY(RailVehicleSwitches, Variant::BOOL, pantograph_impulse);
        BIND_PROPERTY(RailVehicleSwitches, Variant::BOOL, converter_impulse);
        BIND_PROPERTY(RailVehicleSwitches, Variant::BOOL, motor_connectors_impulse);
        BIND_PROPERTY_W_HINT(
                RailVehicleSwitches, Variant::INT, relay_reset_button_1, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY_W_HINT(
                RailVehicleSwitches, Variant::INT, relay_reset_button_2, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY_W_HINT(
                RailVehicleSwitches, Variant::INT, relay_reset_button_3, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY(RailVehicleSwitches, Variant::PACKED_INT32_ARRAY, pantograph_presets);
        BIND_PROPERTY(RailVehicleSwitches, Variant::INT, pantograph_preset_default);
        BIND_PROPERTY(RailVehicleSwitches, Variant::BOOL, modern_dimmer);
        BIND_PROPERTY(RailVehicleSwitches, Variant::BOOL, dimmer_list_cycle, "dimmer_list_positions");
        BIND_PROPERTY(RailVehicleSwitches, Variant::INT, dimmer_list_default_position, "dimmer_list_positions");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                RailVehicleSwitches, Variant::ARRAY, dimmer_list_positions, "dimmer_list_positions",
                PROPERTY_HINT_TYPE_STRING, "RailVehicleDimmerListItem");
        ClassDB::bind_method(D_METHOD("sand", "active"), &RailVehicleSwitches::sand);

        ClassDB::bind_method(D_METHOD("get_sand_active"), &RailVehicleSwitches::get_sand_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "sand_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_sand_active");
    }

    void RailVehicleSwitches::_register_commands() {
        VehicleComponent::_register_commands();
        register_command("sand", Callable(this, "sand"));
    }

    void RailVehicleSwitches::_unregister_commands() {
        VehicleComponent::_unregister_commands();
        unregister_command("sand");
    }
    // how the cab operates the pantographs (PantSwitchType, Train.cpp:3175, 3285)
    void RailVehicleSwitches::_fill_config_dictionary(Dictionary &p_config) const {
        p_config["pantograph_switch_impulse"] = get_pantograph_impulse();
    }
} // namespace godot
