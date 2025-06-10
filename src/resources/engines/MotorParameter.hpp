#pragma once
#include "macros.hpp"
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class MotorParameter : public Resource {
            GDCLASS(MotorParameter, Resource);

        public:
            static void _bind_methods();
            MAKE_MEMBER_GS(double, shunting_up, 0.0);
            MAKE_MEMBER_GS(double, shunting_down, 0.0);
            MAKE_MEMBER_GS(double, voltage_constant_multiplier, 0.0);
            MAKE_MEMBER_GS(double, saturation_current_multiplier, 0.0);
            MAKE_MEMBER_GS(double, initial_voltage_constant_multiplier, 0.0);
            MAKE_MEMBER_GS(double, voltage_constant, 0.0);
            MAKE_MEMBER_GS(double, saturation_current, 0.0);
            MAKE_MEMBER_GS(double, initial_voltage_constant, 0.0);
            MAKE_MEMBER_GS(bool, auto_switch, 0.0);
    };
} // namespace godot
