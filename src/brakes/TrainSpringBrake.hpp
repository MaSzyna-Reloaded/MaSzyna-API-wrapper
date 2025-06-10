#pragma once
#include "../core/TrainPart.hpp"
#include "macros.hpp"
namespace godot {
    class TrainController;
    class TrainSpringBrake: public TrainPart {
        GDCLASS(TrainSpringBrake, TrainPart);
        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override {};
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static void _bind_methods();
            MAKE_MEMBER_GS(float, spring_actuator_chamber_volume, 1.0f);
            MAKE_MEMBER_GS(float, max_spring_actuator_filling_force, 0.0f);
            MAKE_MEMBER_GS(float, pressure_force_coefficient, 0.0f);
            MAKE_MEMBER_GS(float, spring_actuator_preload_pressure, 0.0f);
            MAKE_MEMBER_GS(float, spring_full_balance_pressure, 0.0f);
            MAKE_MEMBER_GS(float, brake_signal_released_state_pressure, 0.0f);
            MAKE_MEMBER_GS(float, brake_signal_braked_state_pressure, 0.0f);
            MAKE_MEMBER_GS(float, actuator_discharge_valve_cross_section, 0.0f);
            MAKE_MEMBER_GS(float, actuator_charge_valve_cross_section, 0.0f);
            MAKE_MEMBER_GS(float, pneumatic_brake_valve_cross_section, 0.0f);
            MAKE_MEMBER_GS(int, required_coupler_connection_method, 0.0f);

            void activate_spring_brake();
            void deactivate_spring_brake();
            void spring_brake_on();
            void spring_brake_off();
            void spring_brake_release();
    };
}// namespace godot