#include "WWListItem.hpp"

namespace godot {
    void WWListItem::_bind_methods() {
        BIND_PROPERTY(Variant::FLOAT, "rpm", "rpm", &WWListItem::set_rpm, &WWListItem::get_rpm, "rpm");
        BIND_PROPERTY(Variant::FLOAT, "max_power", "max_power", &WWListItem::set_max_power, &WWListItem::get_max_power, "max_power");
        BIND_PROPERTY(Variant::FLOAT, "max_voltage", "max_voltage", &WWListItem::set_max_voltage, &WWListItem::get_max_voltage, "max_voltage");
        BIND_PROPERTY(Variant::FLOAT, "max_current", "max_current", &WWListItem::set_max_current, &WWListItem::get_max_current, "max_current");
        BIND_PROPERTY(Variant::BOOL, "shunting", "has_shunting", &WWListItem::set_has_shunting, &WWListItem::get_has_shunting, "has_shunting");
        BIND_PROPERTY(Variant::FLOAT, "min_wakeup_voltage", "min_wakeup_voltage", &WWListItem::set_min_wakeup_voltage, &WWListItem::get_min_wakeup_voltage, "min_wakeup_voltage");
        BIND_PROPERTY(Variant::FLOAT, "max_wakeup_voltage", "max_wakeup_voltage", &WWListItem::set_max_wakeup_voltage, &WWListItem::get_max_wakeup_voltage, "max_wakeup_voltage");
        BIND_PROPERTY(Variant::FLOAT, "max_wakeup_power", "max_wakeup_power", &WWListItem::set_max_wakeup_power, &WWListItem::get_max_wakeup_power, "max_wakeup_power");
    }
} // namespace godot
