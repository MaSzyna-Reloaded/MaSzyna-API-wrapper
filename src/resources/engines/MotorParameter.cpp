#include "MotorParameter.hpp"
#include "macros.hpp"

namespace godot {
    void MotorParameter::_bind_methods() {
        BIND_PROPERTY(Variant::FLOAT, "shunting_up", "shunting_up", &MotorParameter::set_shunting_up, &MotorParameter::get_shunting_up, "shunting_up");
        BIND_PROPERTY(Variant::FLOAT, "shunting_down", "shunting_down", &MotorParameter::set_shunting_down, &MotorParameter::get_shunting_down, "shunting_down");
        BIND_PROPERTY(Variant::FLOAT, "voltage_constant_multiplier", "voltage_constant_multiplier", &MotorParameter::set_voltage_constant_multiplier, &MotorParameter::get_voltage_constant_multiplier, "voltage_constant_multiplier");
        BIND_PROPERTY(Variant::FLOAT, "saturation_current_multiplier", "saturation_current_multiplier", &MotorParameter::set_saturation_current_multiplier, &MotorParameter::get_saturation_current_multiplier, "saturation_current_multiplier");
        BIND_PROPERTY(Variant::FLOAT, "initial_voltage_constant_multiplier", "initial_voltage_constant_multiplier", &MotorParameter::set_initial_voltage_constant_multiplier, &MotorParameter::get_initial_voltage_constant_multiplier, "initial_voltage_constant_multiplier");
        BIND_PROPERTY(Variant::FLOAT, "voltage_constant", "voltage_constant", &MotorParameter::set_voltage_constant, &MotorParameter::get_voltage_constant, "voltage_constant");
        BIND_PROPERTY(Variant::FLOAT, "saturation_current", "saturation_current", &MotorParameter::set_saturation_current, &MotorParameter::get_saturation_current, "saturation_current");
        BIND_PROPERTY(Variant::FLOAT, "initial_voltage_constant", "initial_voltage_constant", &MotorParameter::set_initial_voltage_constant, &MotorParameter::get_initial_voltage_constant, "initial_voltage_constant");
        BIND_PROPERTY(Variant::BOOL, "auto_switch", "auto_switch", &MotorParameter::set_auto_switch, &MotorParameter::get_auto_switch, "auto_switch");
    }
} // namespace godot
