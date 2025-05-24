#pragma once
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class MotorParameter : public Resource {
            GDCLASS(MotorParameter, Resource);

        public:
            static void _bind_methods();

            double shunting_up = 0.0;
            double shunting_down = 0.0;
            double voltage_constant_multiplier = 0.0;
            double saturation_current_multiplier = 0.0;
            double initial_voltage_constant_multiplier = 0.0;
            double voltage_constant = 0.0;
            double saturation_current = 0.0;
            double initial_voltage_constant = 0.0;
            bool auto_switch = false;

            void set_shunting_up(double p_shunting_up);
            void set_shunting_down(double p_shunting_down);
            void set_voltage_constant_multiplier(double p_voltage_constant_multiplier);
            void set_saturation_current_multiplier(double p_saturation_current_multiplier);
            void set_initial_voltage_constant_multiplier(double p_initial_voltage_constant_multiplier);
            void set_voltage_constant(double p_voltage_constant);
            void set_saturation_current(double p_saturation_current);
            void set_initial_voltage_constant(double p_initial_voltage_constant);
            double get_initial_voltage_constant() const;
            void set_auto_switch(bool p_auto_switch);
            double get_shunting_up() const;
            double get_shunting_down() const;
            double get_voltage_constant_multiplier() const;
            double get_saturation_current_multiplier() const;
            double get_initial_voltage_constant_multiplier() const;
            double get_voltage_constant() const;
            double get_saturation_current() const;
            bool get_auto_switch() const;
    };
} // namespace godot
