#include "WiperListItem.hpp"

namespace godot {
    void WiperListItem::_bind_methods() {
        BIND_PROPERTY(WiperListItem, Variant::INT, wiper_mask);
        BIND_PROPERTY(WiperListItem, Variant::FLOAT, transit_time);
        BIND_PROPERTY(WiperListItem, Variant::FLOAT, period);
        BIND_PROPERTY(WiperListItem, Variant::FLOAT, return_delay);
    }
} // namespace godot
