#include "CompressorListItem.hpp"

namespace godot {
    void CompressorListItem::_bind_methods() {
        BIND_PROPERTY(CompressorListItem, Variant::INT, allow);
        BIND_PROPERTY(CompressorListItem, Variant::INT, speed_factor);
        BIND_PROPERTY(CompressorListItem, Variant::INT, min_pressure_factor);
        BIND_PROPERTY(CompressorListItem, Variant::INT, max_pressure_factor);
    }
} // namespace godot
