#include "UniversalControllerListItem.hpp"
#include "macros.hpp"

namespace godot {
    void UniversalControllerListItem::_bind_methods() {
        BIND_PROPERTY(
                Variant::INT, "pneumatic_brake_position", "pneumatic_brake_position",
                &UniversalControllerListItem::set_pneumatic_brake_position,
                &UniversalControllerListItem::get_pneumatic_brake_position, "pneumatic_brake_position");
        BIND_PROPERTY(
                Variant::FLOAT, "min_percentage", "min_percentage", &UniversalControllerListItem::set_min_percentage,
                &UniversalControllerListItem::get_min_percentage, "min_percentage");
        BIND_PROPERTY(
                Variant::FLOAT, "max_percentage", "max_percentage", &UniversalControllerListItem::set_max_percentage,
                &UniversalControllerListItem::get_max_percentage, "max_percentage");
        BIND_PROPERTY(
                Variant::FLOAT, "target_value", "target_value", &UniversalControllerListItem::set_target_value,
                &UniversalControllerListItem::get_target_value, "target_value");
        BIND_PROPERTY(
                Variant::FLOAT, "increase_speed", "increase_speed", &UniversalControllerListItem::set_increase_speed,
                &UniversalControllerListItem::get_increase_speed, "increase_speed");
        BIND_PROPERTY(
                Variant::FLOAT, "decrease_speed", "decrease_speed", &UniversalControllerListItem::set_decrease_speed,
                &UniversalControllerListItem::get_decrease_speed, "decrease_speed");
        BIND_PROPERTY(
                Variant::INT, "bounce_back_position", "bounce_back_position",
                &UniversalControllerListItem::set_bounce_back_position,
                &UniversalControllerListItem::get_bounce_back_position, "bounce_back_position");
        BIND_PROPERTY(
                Variant::INT, "nearest_stable_down", "nearest_stable_down",
                &UniversalControllerListItem::set_nearest_stable_down,
                &UniversalControllerListItem::get_nearest_stable_down, "nearest_stable_down");
        BIND_PROPERTY(
                Variant::INT, "nearest_stable_up", "nearest_stable_up",
                &UniversalControllerListItem::set_nearest_stable_up,
                &UniversalControllerListItem::get_nearest_stable_up, "nearest_stable_up");
    }
} // namespace godot
