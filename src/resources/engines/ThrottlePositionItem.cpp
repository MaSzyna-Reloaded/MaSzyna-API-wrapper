#include "ThrottlePositionItem.hpp"

namespace godot {
    void ThrottlePositionItem::_bind_methods() {
        BIND_PROPERTY(ThrottlePositionItem, Variant::INT, throttle_position);
        BIND_PROPERTY(ThrottlePositionItem, Variant::FLOAT, fuel_dose);
        BIND_PROPERTY_W_HINT(
                ThrottlePositionItem, Variant::INT, clutch_behavior, PROPERTY_HINT_ENUM,
                "None,HalfClutchMinRpm,FullClutchMaxRpm,HalfClutchThenFull,TwoThirdsClutch");

        BIND_ENUM_CONSTANT(CLUTCH_BEHAVIOR_NONE);
        BIND_ENUM_CONSTANT(CLUTCH_BEHAVIOR_HALF_CLUTCH_MIN_RPM);
        BIND_ENUM_CONSTANT(CLUTCH_BEHAVIOR_FULL_CLUTCH_MAX_RPM);
        BIND_ENUM_CONSTANT(CLUTCH_BEHAVIOR_HALF_CLUTCH_THEN_FULL);
        BIND_ENUM_CONSTANT(CLUTCH_BEHAVIOR_TWO_THIRDS_CLUTCH);
    }
} // namespace godot
