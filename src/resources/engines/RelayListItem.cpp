#include "RelayListItem.hpp"

namespace godot {
    void RelayListItem::_bind_methods() {
        BIND_PROPERTY(RelayListItem, Variant::INT, relay_position);
        BIND_PROPERTY(RelayListItem, Variant::FLOAT, resistance);
        BIND_PROPERTY(RelayListItem, Variant::INT, branch_count);
        BIND_PROPERTY(RelayListItem, Variant::INT, motors_per_branch);
        BIND_PROPERTY(RelayListItem, Variant::BOOL, auto_switch);
        BIND_PROPERTY(RelayListItem, Variant::INT, shunt_index);
    }
} // namespace godot
