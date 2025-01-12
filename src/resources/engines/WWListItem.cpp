#include "WWListItem.hpp"

namespace godot {
    void WWListItem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_rpm", "rpm"), &WWListItem::set_rpm);
        ClassDB::bind_method(D_METHOD("get_rpm"), &WWListItem::get_rpm);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rpm"), "set_rpm", "get_rpm");

        ClassDB::bind_method(D_METHOD("set_max_power", "max_power"), &WWListItem::set_max_power);
        ClassDB::bind_method(D_METHOD("get_max_power"), &WWListItem::get_max_power);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_power"), "set_max_power", "get_max_power");

        ClassDB::bind_method(D_METHOD("set_max_voltage", "max_voltage"), &WWListItem::set_max_voltage);
        ClassDB::bind_method(D_METHOD("get_max_voltage"), &WWListItem::get_max_voltage);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_voltage"), "set_max_voltage", "get_max_voltage");

        ClassDB::bind_method(D_METHOD("set_max_current", "max_current"), &WWListItem::set_max_current);
        ClassDB::bind_method(D_METHOD("get_max_current"), &WWListItem::get_max_current);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_current"), "set_max_current", "get_max_current");

        ClassDB::bind_method(D_METHOD("set_shunting", "shunting_mode"), &WWListItem::set_shunting);
        ClassDB::bind_method(D_METHOD("get_shunting"), &WWListItem::get_shunting);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "has_shunting"), "set_shunting", "get_shunting");

        ClassDB::bind_method(
                D_METHOD("set_min_wakeup_voltage", "min_wakeup_voltage"), &WWListItem::set_min_wakeup_voltage);
        ClassDB::bind_method(D_METHOD("get_min_wakeup_voltage"), &WWListItem::get_min_wakeup_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "min_wakeup_voltage"), "set_min_wakeup_voltage", "get_min_wakeup_voltage");

        ClassDB::bind_method(
                D_METHOD("set_max_wakeup_voltage", "max_wakeup_voltage"), &WWListItem::set_max_wakeup_voltage);
        ClassDB::bind_method(D_METHOD("get_max_wakeup_voltage"), &WWListItem::get_max_wakeup_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "max_wakeup_voltage"), "set_max_wakeup_voltage", "get_max_wakeup_voltage");

        ClassDB::bind_method(D_METHOD("set_max_wakeup_power", "max_wakeup_power"), &WWListItem::set_max_wakeup_power);
        ClassDB::bind_method(D_METHOD("get_max_wakeup_power"), &WWListItem::get_max_wakeup_power);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_wakeup_power"), "set_max_wakeup_power", "get_max_wakeup_power");
    }

    void WWListItem::set_rpm(const double p_rpm) {
        rpm = p_rpm;
    }

    double WWListItem::get_rpm() const {
        return rpm;
    }

    void WWListItem::set_max_power(const double p_max_power) {
        max_power = p_max_power;
    }

    double WWListItem::get_max_power() const {
        return max_power;
    }

    void WWListItem::set_max_voltage(const double p_max_voltage) {
        max_voltage = p_max_voltage;
    }

    double WWListItem::get_max_voltage() const {
        return max_voltage;
    }

    void WWListItem::set_max_current(const double p_max_current) {
        max_current = p_max_current;
    }

    double WWListItem::get_max_current() const {
        return max_current;
    }

    void WWListItem::set_shunting(const bool p_shunting) {
        has_shunting = p_shunting;
    }

    bool WWListItem::get_shunting() const {
        return has_shunting;
    }

    void WWListItem::set_min_wakeup_voltage(const double p_min_wakeup_voltage) {
        min_wakeup_voltage = p_min_wakeup_voltage;
    }

    double WWListItem::get_min_wakeup_voltage() const {
        return min_wakeup_voltage;
    }

    void WWListItem::set_max_wakeup_voltage(const double p_max_wakeup_voltage) {
        max_wakeup_voltage = p_max_wakeup_voltage;
    }

    double WWListItem::get_max_wakeup_voltage() const {
        return max_wakeup_voltage;
    }

    void WWListItem::set_max_wakeup_power(const double p_max_wakeup_power) {
        max_wakeup_power = p_max_wakeup_power;
    }

    double WWListItem::get_max_wakeup_power() const {
        return max_wakeup_power;
    }
} // namespace godot
