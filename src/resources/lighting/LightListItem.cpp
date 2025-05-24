#include "LightListItem.hpp"
#include "macros.hpp"
namespace godot {
    void LightListItem::_bind_methods() {
        //Cabin A
        BIND_PROPERTY(Variant::BOOL, "cabin_a_head_light", "cabin_a/head_light", &LightListItem::set_cabin_a_head_light, &LightListItem::get_cabin_a_head_light, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_a_left_white_signal", "cabin_a/left/white_signal", &LightListItem::set_cabin_a_left_white_signal, &LightListItem::get_cabin_a_left_white_signal, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_a_left_red_signal", "cabin_a/left/red_signal", &LightListItem::set_cabin_a_left_red_signal, &LightListItem::get_cabin_a_left_red_signal, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_a_right_white_signal", "cabin_a/right/white_signal", &LightListItem::set_cabin_a_right_white_signal, &LightListItem::get_cabin_a_right_white_signal, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_a_right_red_signal", "cabin_a/right/red_signal", &LightListItem::set_cabin_a_right_red_signal, &LightListItem::get_cabin_a_right_red_signal, "enabled");
        //Cabin B
        BIND_PROPERTY(Variant::BOOL, "cabin_b_head_light", "cabin_b/head_light", &LightListItem::set_cabin_b_head_light, &LightListItem::get_cabin_b_head_light, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_b_left_white_signal", "cabin_b/left/white_signal", &LightListItem::set_cabin_b_left_white_signal, &LightListItem::get_cabin_b_left_white_signal, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_b_left_red_signal", "cabin_b/left/red_signal", &LightListItem::set_cabin_b_left_red_signal, &LightListItem::get_cabin_b_left_red_signal, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_b_right_white_signal", "cabin_b/right/white_signal", &LightListItem::set_cabin_b_right_white_signal, &LightListItem::get_cabin_b_right_white_signal, "enabled");
        BIND_PROPERTY(Variant::BOOL, "cabin_b_right_red_signal", "cabin_b/right/red_signal", &LightListItem::set_cabin_b_right_red_signal, &LightListItem::get_cabin_b_right_red_signal, "enabled");
    }

    void LightListItem::set_cabin_a_head_light(const bool p_head_light) {
        cabin_a.head_light = p_head_light;
    }

    bool LightListItem::get_cabin_a_head_light() const {
        return cabin_a.head_light;
    }

    void LightListItem::set_cabin_a_left_white_signal(const bool p_left_white_signal) {
        cabin_a.left_white_signal = p_left_white_signal;
    }

    bool LightListItem::get_cabin_a_left_white_signal() const {
        return cabin_a.left_white_signal;
    }

    void LightListItem::set_cabin_a_left_red_signal(const bool p_left_red_signal) {
        cabin_a.left_red_signal = p_left_red_signal;
    }

    bool LightListItem::get_cabin_a_left_red_signal() const {
        return cabin_a.left_red_signal;
    }

    void LightListItem::set_cabin_a_right_white_signal(const bool p_right_white_signal) {
        cabin_a.right_white_signal = p_right_white_signal;
    }

    bool LightListItem::get_cabin_a_right_white_signal() const {
        return cabin_a.right_white_signal;
    }

    void LightListItem::set_cabin_a_right_red_signal(const bool p_right_red_signal) {
        cabin_a.right_red_signal = p_right_red_signal;
    }

    bool LightListItem::get_cabin_a_right_red_signal() const {
        return cabin_a.right_red_signal;
    }

    void LightListItem::set_cabin_b_head_light(const bool p_head_light) {
        cabin_b.head_light = p_head_light;
    }

    bool LightListItem::get_cabin_b_head_light() const {
        return cabin_b.head_light;
    }

    void LightListItem::set_cabin_b_left_white_signal(const bool p_left_white_signal) {
        cabin_b.left_white_signal = p_left_white_signal;
    }

    bool LightListItem::get_cabin_b_left_white_signal() const {
        return cabin_b.left_white_signal;
    }

    void LightListItem::set_cabin_b_left_red_signal(const bool p_left_red_signal) {
        cabin_b.left_red_signal = p_left_red_signal;
    }

    bool LightListItem::get_cabin_b_left_red_signal() const {
        return cabin_b.left_red_signal;
    }

    void LightListItem::set_cabin_b_right_white_signal(const bool p_right_white_signal) {
        cabin_b.right_white_signal = p_right_white_signal;
    }

    bool LightListItem::get_cabin_b_right_white_signal() const {
        return cabin_b.right_white_signal;
    }

    void LightListItem::set_cabin_b_right_red_signal(const bool p_right_red_signal) {
        cabin_b.right_red_signal = p_right_red_signal;
    }

    bool LightListItem::get_cabin_b_right_red_signal() const {
        return cabin_b.right_red_signal;
    }
} // namespace godot
