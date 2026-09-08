#include "TrainSwitches.hpp"

namespace godot {
    void TrainSwitches::_bind_methods() {
        BIND_PROPERTY(TrainSwitches, Variant::BOOL, pantograph_impulse);
        BIND_PROPERTY(TrainSwitches, Variant::BOOL, converter_impulse);
        BIND_PROPERTY(TrainSwitches, Variant::BOOL, motor_connectors_impulse);
        BIND_PROPERTY_W_HINT(
                TrainSwitches, Variant::INT, relay_reset_button_1, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY_W_HINT(
                TrainSwitches, Variant::INT, relay_reset_button_2, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY_W_HINT(
                TrainSwitches, Variant::INT, relay_reset_button_3, "relay_reset_button", PROPERTY_HINT_FLAGS,
                "Main Circuit Diff,Aux Circuit Diff,Traction Motor Overload,Main Converter Overload,"
                "Aux Converter Overload,Fan Overload,Heating Overload,ED Brake Overload");
        BIND_PROPERTY(TrainSwitches, Variant::PACKED_INT32_ARRAY, pantograph_presets);
        BIND_PROPERTY(TrainSwitches, Variant::INT, pantograph_preset_default);
        BIND_PROPERTY(TrainSwitches, Variant::BOOL, modern_dimmer);
        BIND_PROPERTY(TrainSwitches, Variant::BOOL, dimmer_list_cycle, "dimmer_list_positions");
        BIND_PROPERTY(TrainSwitches, Variant::INT, dimmer_list_default_position, "dimmer_list_positions");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainSwitches, Variant::ARRAY, dimmer_list_positions, "dimmer_list_positions",
                PROPERTY_HINT_TYPE_STRING, "DimmerListItem");
    }

    void TrainSwitches::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        TrainPart::_do_update_internal_mover(p_mover);

        p_mover->PantSwitchType = pantograph_impulse ? "impulse" : "";
        p_mover->ConvSwitchType = converter_impulse ? "impulse" : "";
        p_mover->StLinSwitchType = motor_connectors_impulse ? "impulse" : "toggle";
    }

    void TrainSwitches::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        // RelayResetButtonX/PantographPresets/ModernDimmer/DimmerList are not wired to the
        // mover: see the class-level note in TrainSwitches.hpp.
    }
} // namespace godot
