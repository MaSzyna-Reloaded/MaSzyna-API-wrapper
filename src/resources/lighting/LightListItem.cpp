#include "LightListItem.hpp"
#include "macros.hpp"
namespace godot {
    void LightListItem::_bind_methods() { // Cabin A
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_a_head_light, "cabin_a");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_a_left_white_signal, "cabin_a/left");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_a_left_red_signal, "cabin_a/left");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_a_right_white_signal, "cabin_a/right");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_a_right_red_signal, "cabin_a/right");
        // Cabin B
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_b_head_light, "cabin_b");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_b_left_white_signal, "cabin_b/left");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_b_left_red_signal, "cabin_b/left");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_b_right_white_signal, "cabin_b/right");
        BIND_PROPERTY(LightListItem, Variant::BOOL, cabin_b_right_red_signal, "cabin_b/right");
    }
} // namespace godot
