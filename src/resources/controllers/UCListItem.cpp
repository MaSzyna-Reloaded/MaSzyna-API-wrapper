#include "UCListItem.hpp"
#include "macros.hpp"

namespace godot {
    void UCListItem::_bind_methods() {
        BIND_PROPERTY(
                Variant::INT, "position", "position", &UCListItem::set_position, &UCListItem::get_position, "position");
        BIND_PROPERTY(
                Variant::INT, "pneumatic_brake_position", "pneumatic_brake_position",
                &UCListItem::set_pneumatic_brake_position, &UCListItem::get_pneumatic_brake_position,
                "pneumatic_brake_position");
        BIND_PROPERTY(
                Variant::FLOAT, "min_percentage", "min_percentage", &UCListItem::set_min_percentage,
                &UCListItem::get_min_percentage, "min_percentage");
        BIND_PROPERTY(
                Variant::FLOAT, "max_percentage", "max_percentage", &UCListItem::set_max_percentage,
                &UCListItem::get_max_percentage, "max_percentage");
        BIND_PROPERTY(
                Variant::FLOAT, "target_value", "target_value", &UCListItem::set_target_value,
                &UCListItem::get_target_value, "target_value");
        BIND_PROPERTY(
                Variant::FLOAT, "increase_speed", "increase_speed", &UCListItem::set_increase_speed,
                &UCListItem::get_increase_speed, "increase_speed");
        BIND_PROPERTY(
                Variant::FLOAT, "decrease_speed", "decrease_speed", &UCListItem::set_decrease_speed,
                &UCListItem::get_decrease_speed, "decrease_speed");
        BIND_PROPERTY(
                Variant::INT, "bounce_back_position", "bounce_back_position", &UCListItem::set_bounce_back_position,
                &UCListItem::get_bounce_back_position, "bounce_back_position");
        BIND_PROPERTY(
                Variant::INT, "nearest_stable_down", "nearest_stable_down", &UCListItem::set_nearest_stable_down,
                &UCListItem::get_nearest_stable_down, "nearest_stable_down");
        BIND_PROPERTY(
                Variant::INT, "nearest_stable_up", "nearest_stable_up", &UCListItem::set_nearest_stable_up,
                &UCListItem::get_nearest_stable_up, "nearest_stable_up");
    }
} // namespace godot
