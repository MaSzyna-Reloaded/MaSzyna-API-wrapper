#include "CurvePointItem.hpp"

namespace godot {
    void CurvePointItem::_bind_methods() {
        BIND_PROPERTY(CurvePointItem, Variant::FLOAT, x);
        BIND_PROPERTY(CurvePointItem, Variant::FLOAT, y);
    }
} // namespace godot
