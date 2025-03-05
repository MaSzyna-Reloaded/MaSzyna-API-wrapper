#pragma once
#include <godot_cpp/classes/resource.hpp>

namespace godot {
    class WWListItem : public Resource {
            GDCLASS(WWListItem, Resource);

        public:
            static void _bind_methods();

            double rpm = 0.0;
            double max_power = 0.0;
            double max_voltage = 0.0;
            double max_current = 0.0;

            /**
             * has shunting controller?
             */
            bool has_shunting = false;

            /**
             * voltage with minimal wake-up
             */
            double min_wakeup_voltage = 0.0;

            /**
             * voltage with maximum wake-up
             */
            double max_wakeup_voltage = 0.0;

            /**
             * power with maximum wake-up
             */
            double max_wakeup_power = 0.0;

            void set_rpm(double p_rpm);
            double get_rpm() const;
            void set_max_power(double p_max_power);
            double get_max_power() const;
            void set_max_voltage(double p_max_voltage);
            double get_max_voltage() const;
            void set_max_current(double p_max_current);
            double get_max_current() const;
            void set_shunting(bool p_shunting);
            bool get_shunting() const;
            void set_min_wakeup_voltage(double p_min_wakeup_voltage);
            double get_min_wakeup_voltage() const;
            void set_max_wakeup_voltage(double p_max_wakeup_voltage);
            double get_max_wakeup_voltage() const;
            void set_max_wakeup_power(double p_max_wakeup_power);
            double get_max_wakeup_power() const;
    };
} // namespace godot
