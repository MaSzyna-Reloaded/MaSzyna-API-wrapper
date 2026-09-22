#include "VehicleAIHints.hpp"

namespace godot {
    void VehicleAIHints::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                VehicleAIHints, Variant::INT, pantograph_state, PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"Front", PANTOGRAPH_STATE_FRONT},
                         {"Rear", PANTOGRAPH_STATE_REAR},
                         {"Both", PANTOGRAPH_STATE_BOTH}}));
        BIND_PROPERTY(VehicleAIHints, Variant::BOOL, raise_pantographs_when_idle);
        BIND_PROPERTY(VehicleAIHints, Variant::FLOAT, local_brake_acceleration_factor);

        BIND_ENUM_CONSTANT(PANTOGRAPH_STATE_FRONT);
        BIND_ENUM_CONSTANT(PANTOGRAPH_STATE_REAR);
        BIND_ENUM_CONSTANT(PANTOGRAPH_STATE_BOTH);
    }

    void VehicleAIHints::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        VehicleComponent::_do_update_internal_mover(p_mover);

        p_mover->AIHintPantstate = pantograph_state;
        p_mover->AIHintPantUpIfIdle = raise_pantographs_when_idle;
        p_mover->AIHintLocalBrakeAccFactor = local_brake_acceleration_factor;
    }

    void VehicleAIHints::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        ASSERT_MOVER(p_mover);
    }
} // namespace godot
