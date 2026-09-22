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
    }

    void VehicleSwitches::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        VehicleComponent::_do_update_internal_mover(p_mover);

        p_mover->PantSwitchType = pantograph_impulse ? "impulse" : "";
        p_mover->ConvSwitchType = converter_impulse ? "impulse" : "";
        p_mover->StLinSwitchType = motor_connectors_impulse ? "impulse" : "toggle";
    }


    void VehicleSwitches::_fill_state_dictionary(Dictionary &p_state) const {
        const TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_state["sand_active"] = mover->SandDose;
    }

    void VehicleSwitches::sand(const bool p_active) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Train.cpp:1917-1939 (OnCommand_sandboxactivate) -> SandboxManual(State),
        // "sand_bt:"/ggSandButton (Train.cpp:10044) - momentary, active only while held.
        mover->SandboxManual(p_active);
    }

    void VehicleSwitches::_register_commands() {
        VehicleComponent::_register_commands();
        register_command("sand", Callable(this, "sand"));
    }

    void VehicleSwitches::_unregister_commands() {
        VehicleComponent::_unregister_commands();
        unregister_command("sand", Callable(this, "sand"));
    }
} // namespace godot
