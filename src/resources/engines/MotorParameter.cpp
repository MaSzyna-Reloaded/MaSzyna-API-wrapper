#include "MotorParameter.hpp"
#include "macros.hpp"

namespace godot {
    void MotorParameter::_bind_methods() {
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, shunting_up);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, shunting_down);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, voltage_constant_multiplier);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, saturation_current_multiplier);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, initial_voltage_constant_multiplier);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, voltage_constant);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, saturation_current);
        BIND_PROPERTY(MotorParameter, Variant::FLOAT, initial_voltage_constant);
        BIND_PROPERTY(MotorParameter, Variant::BOOL, auto_switch);
    }
} // namespace godot
