#include "LightListItem.hpp"

namespace godot {
    void LightListItem::_bind_methods() {
        //Cabin A
        ClassDB::bind_method(D_METHOD("set_cabin_a_head_light", "head_light"), &LightListItem::set_cabin_a_head_light);
        ClassDB::bind_method(D_METHOD("get_cabin_a_head_light"), &LightListItem::get_cabin_a_head_light);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cabin_a/head_light"), "set_cabin_a_head_light", "get_cabin_a_head_light");

        ClassDB::bind_method(
                D_METHOD("set_cabin_a_left_white_signal", "left_white_signal"), &LightListItem::set_cabin_a_left_white_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_a_left_white_signal"), &LightListItem::get_cabin_a_left_white_signal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabin_a/left/white_signal"), "set_cabin_a_left_white_signal", "get_cabin_a_left_white_signal");

        ClassDB::bind_method(D_METHOD("set_cabin_a_left_red_signal", "left_red_signal"), &LightListItem::set_cabin_a_left_red_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_a_left_red_signal"), &LightListItem::get_cabin_a_left_red_signal);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cabin_a/left/red_signal"), "set_cabin_a_left_red_signal", "get_cabin_a_left_red_signal");

        ClassDB::bind_method(
                D_METHOD("set_cabin_a_right_white_signal", "right_white_light"), &LightListItem::set_cabin_a_right_white_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_a_right_white_signal"), &LightListItem::get_cabin_a_right_white_signal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabin_a/right/white_signal"), "set_cabin_a_right_white_signal", "get_cabin_a_right_white_signal");

        ClassDB::bind_method(
                D_METHOD("set_cabin_a_right_red_signal", "right_red_signal"), &LightListItem::set_cabin_a_right_red_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_a_right_red_signal"), &LightListItem::get_cabin_a_right_red_signal);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cabin_a/right/red_signal"), "set_cabin_a_right_red_signal", "get_cabin_a_right_red_signal");

        //Cabin B

        ClassDB::bind_method(D_METHOD("set_cabin_b_head_light", "head_light"), &LightListItem::set_cabin_b_head_light);
        ClassDB::bind_method(D_METHOD("get_cabin_b_head_light"), &LightListItem::get_cabin_b_head_light);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cabin_b/head_light"), "set_cabin_b_head_light", "get_cabin_b_head_light");

        ClassDB::bind_method(
                D_METHOD("set_cabin_b_left_white_signal", "left_white_signal"), &LightListItem::set_cabin_b_left_white_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_b_left_white_signal"), &LightListItem::get_cabin_b_left_white_signal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabin_b/left/white_signal"), "set_cabin_b_left_white_signal", "get_cabin_b_left_white_signal");

        ClassDB::bind_method(D_METHOD("set_cabin_b_left_red_signal", "left_red_signal"), &LightListItem::set_cabin_b_left_red_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_b_left_red_signal"), &LightListItem::get_cabin_b_left_red_signal);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cabin_b/left/red_signal"), "set_cabin_b_left_red_signal", "get_cabin_b_left_red_signal");

        ClassDB::bind_method(
                D_METHOD("set_cabin_b_right_white_signal", "left_right_light"), &LightListItem::set_cabin_b_right_white_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_b_right_white_signal"), &LightListItem::get_cabin_b_right_white_signal);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "cabin_b/right/white_signal"), "set_cabin_b_right_white_signal", "get_cabin_b_right_white_signal");

        ClassDB::bind_method(
                D_METHOD("set_cabin_b_right_red_signal", "right_red_signal"), &LightListItem::set_cabin_b_right_red_signal);
        ClassDB::bind_method(D_METHOD("get_cabin_b_right_red_signal"), &LightListItem::get_cabin_b_right_red_signal);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cabin_b/right/red_signal"), "set_cabin_b_right_red_signal", "get_cabin_b_right_red_signal");
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
