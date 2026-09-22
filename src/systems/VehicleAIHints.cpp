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
} // namespace godot
