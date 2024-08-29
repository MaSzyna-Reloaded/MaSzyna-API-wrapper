#include "DieselEngineMasterControllerPowerItem.hpp"

DieselEngineMasterControllerPowerItem::DieselEngineMasterControllerPowerItem() :
    rpm(0.0f), generator_power(0.0f), voltage_max(0.0f), current_max(0.0f) {}

float DieselEngineMasterControllerPowerItem::get_rpm() const {
    return rpm;
}

void DieselEngineMasterControllerPowerItem::set_rpm(float p_rpm) {
    rpm = p_rpm;
}

float DieselEngineMasterControllerPowerItem::get_generator_power() const {
    return generator_power;
}

void DieselEngineMasterControllerPowerItem::set_generator_power(float p_generator_power) {
    generator_power = p_generator_power;
}

float DieselEngineMasterControllerPowerItem::get_voltage_max() const {
    return voltage_max;
}

void DieselEngineMasterControllerPowerItem::set_voltage_max(float p_voltage_max) {
    voltage_max = p_voltage_max;
}

float DieselEngineMasterControllerPowerItem::get_current_max() const {
    return current_max;
}

void DieselEngineMasterControllerPowerItem::set_current_max(float p_current_max) {
    current_max = p_current_max;
}

void DieselEngineMasterControllerPowerItem::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_rpm"), &DieselEngineMasterControllerPowerItem::get_rpm);
    ClassDB::bind_method(D_METHOD("set_rpm", "rpm"), &DieselEngineMasterControllerPowerItem::set_rpm);
    ADD_PROPERTY(
            PropertyInfo(Variant::FLOAT, "rpm", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_rpm", "get_rpm");

    ClassDB::bind_method(D_METHOD("get_generator_power"), &DieselEngineMasterControllerPowerItem::get_generator_power);
    ClassDB::bind_method(
            D_METHOD("set_generator_power", "generator_power"),
            &DieselEngineMasterControllerPowerItem::set_generator_power);
    ADD_PROPERTY(
            PropertyInfo(Variant::FLOAT, "generator_power", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT),
            "set_generator_power", "get_generator_power");

    ClassDB::bind_method(D_METHOD("get_voltage_max"), &DieselEngineMasterControllerPowerItem::get_voltage_max);
    ClassDB::bind_method(
            D_METHOD("set_voltage_max", "voltage_max"), &DieselEngineMasterControllerPowerItem::set_voltage_max);
    ADD_PROPERTY(
            PropertyInfo(Variant::FLOAT, "voltage_max", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT),
            "set_voltage_max", "get_voltage_max");

    ClassDB::bind_method(D_METHOD("get_current_max"), &DieselEngineMasterControllerPowerItem::get_current_max);
    ClassDB::bind_method(
            D_METHOD("set_current_max", "current_max"), &DieselEngineMasterControllerPowerItem::set_current_max);
    ADD_PROPERTY(
            PropertyInfo(Variant::FLOAT, "current_max", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT),
            "set_current_max", "get_current_max");
}
