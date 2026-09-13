#include "UniversalControllerListItem.hpp"
#include "macros.hpp"

namespace godot {
    void UniversalControllerListItem::_bind_methods() {
        BIND_PROPERTY(UniversalControllerListItem, Variant::INT, pneumatic_brake_position);
        BIND_PROPERTY(UniversalControllerListItem, Variant::FLOAT, min_percentage);
        BIND_PROPERTY(UniversalControllerListItem, Variant::FLOAT, max_percentage);
        BIND_PROPERTY(UniversalControllerListItem, Variant::FLOAT, target_value);
        BIND_PROPERTY(UniversalControllerListItem, Variant::FLOAT, increase_speed);
        BIND_PROPERTY(UniversalControllerListItem, Variant::FLOAT, decrease_speed);
        BIND_PROPERTY(UniversalControllerListItem, Variant::INT, bounce_back_position);
        BIND_PROPERTY(UniversalControllerListItem, Variant::INT, nearest_stable_down);
        BIND_PROPERTY(UniversalControllerListItem, Variant::INT, nearest_stable_up);
    }
} // namespace godot
