#include "LightListItem.hpp"

namespace godot {
    void LightListItem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_head_light", "head_light"), &LightListItem::set_head_light);
        ClassDB::bind_method(D_METHOD("get_head_light"), &LightListItem::get_head_light);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "head_light"), "set_head_light", "get_head_light");

        ClassDB::bind_method(
                D_METHOD("set_left_white_signal", "left_white_signal"), &LightListItem::set_left_white_signal);
        ClassDB::bind_method(D_METHOD("get_left_white_signal"), &LightListItem::get_left_white_signal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "left_white_signal"), "set_left_white_signal", "get_left_white_signal");

        ClassDB::bind_method(D_METHOD("set_left_red_signal", "left_red_signal"), &LightListItem::set_left_red_signal);
        ClassDB::bind_method(D_METHOD("get_left_red_signal"), &LightListItem::get_left_red_signal);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "left_red_signal"), "set_left_red_signal", "get_left_red_signal");

        ClassDB::bind_method(
                D_METHOD("set_right_white_signal", "left_White_light"), &LightListItem::set_right_white_signal);
        ClassDB::bind_method(D_METHOD("get_right_white_signal"), &LightListItem::get_right_white_signal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "right_white_signal"), "set_right_white_signal", "get_right_white_signal");

        ClassDB::bind_method(
                D_METHOD("set_right_red_signal", "right_red_signal"), &LightListItem::set_right_red_signal);
        ClassDB::bind_method(D_METHOD("get_right_red_signal"), &LightListItem::get_right_red_signal);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "right_red_signal"), "set_right_red_signal", "get_right_red_signal");
    }

    void LightListItem::set_head_light(const bool p_head_light) {
        head_light = p_head_light;
    }

    bool LightListItem::get_head_light() const {
        return head_light;
    }

    void LightListItem::set_left_white_signal(const bool p_left_white_signal) {
        left_white_signal = p_left_white_signal;
    }

    bool LightListItem::get_left_white_signal() const {
        return left_white_signal;
    }

    void LightListItem::set_left_red_signal(const bool p_left_red_signal) {
        left_red_signal = p_left_red_signal;
    }

    bool LightListItem::get_left_red_signal() const {
        return left_red_signal;
    }

    void LightListItem::set_right_white_signal(const bool p_right_white_signal) {
        right_white_signal = p_right_white_signal;
    }

    bool LightListItem::get_right_white_signal() const {
        return right_white_signal;
    }

    void LightListItem::set_right_red_signal(const bool p_right_red_signal) {
        right_red_signal = p_right_red_signal;
    }

    bool LightListItem::get_right_red_signal() const {
        return right_red_signal;
    }
} // namespace godot
