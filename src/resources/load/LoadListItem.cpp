#include "LoadListItem.hpp"
#include "macros.hpp"

namespace godot {
    void LoadListItem::_bind_methods() {
        BIND_PROPERTY(LoadListItem, Variant::FLOAT, max_load)
        BIND_PROPERTY_W_HINT(LoadListItem, Variant::INT, load_type, PROPERTY_HINT_ENUM, "None,Passenger,Cargo")
        BIND_PROPERTY(LoadListItem, Variant::FLOAT, load_offset)

        BIND_ENUM_CONSTANT(LOAD_TYPE_NONE);
        BIND_ENUM_CONSTANT(LOAD_TYPE_PASSENGER);
        BIND_ENUM_CONSTANT(LOAD_TYPE_CARGO);
    }
} // namespace godot
