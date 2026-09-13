#include "WWListItem.hpp"

namespace godot {
    void WWListItem::_bind_methods() {
        BIND_PROPERTY(WWListItem, Variant::FLOAT, rpm);
        BIND_PROPERTY(WWListItem, Variant::FLOAT, max_power);
        BIND_PROPERTY(WWListItem, Variant::FLOAT, max_voltage);
        BIND_PROPERTY(WWListItem, Variant::FLOAT, max_current);
        BIND_PROPERTY(WWListItem, Variant::BOOL, has_shunting);
        BIND_PROPERTY(WWListItem, Variant::FLOAT, min_wakeup_voltage);
        BIND_PROPERTY(WWListItem, Variant::FLOAT, max_wakeup_voltage);
        BIND_PROPERTY(WWListItem, Variant::FLOAT, max_wakeup_power);
    }
} // namespace godot
