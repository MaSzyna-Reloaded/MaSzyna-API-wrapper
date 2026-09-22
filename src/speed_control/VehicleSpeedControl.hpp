#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleController;
    class VehicleSpeedControl : public VehicleComponent {
            GDCLASS(VehicleSpeedControl, VehicleComponent);

        private:
            static void _bind_methods();

        private:
            int state_base_index = 0;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _declare_state_properties() override;

            enum StateProperty {
                STATE_ACTIVE,
                STATE_DESIRED_VELOCITY,
                STATE_DESIRED_POWER,
                STATE_SELECTED_VELOCITY,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

            MAKE_MEMBER_GS(bool, speed_control_enabled, false);
            MAKE_MEMBER_GS(double, delay, 0.0);
            MAKE_MEMBER_GS(bool, impulse_lever, false);
            MAKE_MEMBER_GS(int, disables_on, 0);
            MAKE_MEMBER_GS(PackedFloat64Array, preset_speeds, PackedFloat64Array());
            MAKE_MEMBER_GS(bool, override_manual_power, false);
            MAKE_MEMBER_GS(double, initial_power, 1.0);
            MAKE_MEMBER_GS(double, full_power_velocity, -1.0);
            MAKE_MEMBER_GS(double, start_velocity, -1.0);
            MAKE_MEMBER_GS(double, velocity_step, 5.0);
            MAKE_MEMBER_GS(double, power_step, 0.1);
            MAKE_MEMBER_GS(double, min_power, 0.0);
            MAKE_MEMBER_GS(double, max_power, 1.0);
            MAKE_MEMBER_GS(double, min_velocity, 0.0);
            MAKE_MEMBER_GS(double, max_velocity, 120.0);
            MAKE_MEMBER_GS(double, offset, -0.5);
            MAKE_MEMBER_GS(double, proportional_gain_positive, 0.5);
            MAKE_MEMBER_GS(double, proportional_gain_negative, 0.5);
            MAKE_MEMBER_GS(double, integral_gain_positive, 0.0);
            MAKE_MEMBER_GS(double, integral_gain_negative, 0.0);
            MAKE_MEMBER_GS(bool, brake_intervention, false);
            MAKE_MEMBER_GS(double, brake_intervention_max_velocity, 30.0);
            MAKE_MEMBER_GS(double, power_up_speed, 1000.0);
            MAKE_MEMBER_GS(double, power_down_speed, 1000.0);
    };
} // namespace godot
