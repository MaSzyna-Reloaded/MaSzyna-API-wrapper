#include "LoadListItem.hpp"
#include "macros.hpp"

namespace godot {
    void LoadListItem::_bind_methods() {
        BIND_PROPERTY(Variant::FLOAT, "max_load", "max_load", &LoadListItem::set_max_load, &LoadListItem::get_max_load, "max_load")
        BIND_PROPERTY_W_HINT(Variant::INT, "load_type", "load_type", &LoadListItem::set_load_type, &LoadListItem::get_load_type, "load_type", PROPERTY_HINT_ENUM, "None,Passenger,Cargo")
        BIND_PROPERTY(Variant::FLOAT, "load_offset", "load_offset", &LoadListItem::set_load_offset, &LoadListItem::get_load_offset, "load_offset")

        BIND_ENUM_CONSTANT(LOAD_TYPE_NONE);
        BIND_ENUM_CONSTANT(LOAD_TYPE_PASSENGER);
        BIND_ENUM_CONSTANT(LOAD_TYPE_CARGO);
    }
} // namespace godot
