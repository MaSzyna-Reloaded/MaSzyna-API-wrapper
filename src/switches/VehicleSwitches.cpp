#include "VehicleSwitches.hpp"

namespace godot {
    void VehicleSwitches::_bind_methods() {
        BIND_PROPERTY(VehicleSwitches, Variant::BOOL, pantograph_impulse);
        BIND_PROPERTY(VehicleSwitches, Variant::BOOL, converter_impulse);
        BIND_PROPERTY(VehicleSwitches, Variant::BOOL, motor_connectors_impulse);
        BIND_PROPERTY_W_HINT(
                VehicleSwitches, Variant::INT, relay_reset_button_1, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY_W_HINT(
                VehicleSwitches, Variant::INT, relay_reset_button_2, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY_W_HINT(
                VehicleSwitches, Variant::INT, relay_reset_button_3, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY(VehicleSwitches, Variant::PACKED_INT32_ARRAY, pantograph_presets);
        BIND_PROPERTY(VehicleSwitches, Variant::INT, pantograph_preset_default);
        BIND_PROPERTY(VehicleSwitches, Variant::BOOL, modern_dimmer);
        BIND_PROPERTY(VehicleSwitches, Variant::BOOL, dimmer_list_cycle, "dimmer_list_positions");
        BIND_PROPERTY(VehicleSwitches, Variant::INT, dimmer_list_default_position, "dimmer_list_positions");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleSwitches, Variant::ARRAY, dimmer_list_positions, "dimmer_list_positions",
                PROPERTY_HINT_TYPE_STRING, "DimmerListItem");
        ClassDB::bind_method(D_METHOD("sand", "active"), &VehicleSwitches::sand);

        ClassDB::bind_method(D_METHOD("get_sand_active"), &VehicleSwitches::get_sand_active);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::BOOL, "sand_active", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY),
                "", "get_sand_active");
    }

    void VehicleSwitches::_register_commands() {
        VehicleComponent::_register_commands();
        register_command("sand", Callable(this, "sand"));
    }

    void VehicleSwitches::_unregister_commands() {
        VehicleComponent::_unregister_commands();
        unregister_command("sand", Callable(this, "sand"));
    }
    // how the cab operates the pantographs (PantSwitchType, Train.cpp:3175, 3285)
    void VehicleSwitches::_fill_config_dictionary(Dictionary &p_config) const {
        p_config["pantograph_switch_impulse"] = get_pantograph_impulse();
    }
} // namespace godot
