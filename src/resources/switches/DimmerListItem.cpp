#include "DimmerListItem.hpp"

namespace godot {
    void DimmerListItem::_bind_methods() {
        BIND_PROPERTY(DimmerListItem, Variant::BOOL, high_beam);
        BIND_PROPERTY(DimmerListItem, Variant::BOOL, dimmed);
        BIND_PROPERTY(DimmerListItem, Variant::BOOL, off);
    }
} // namespace godot
