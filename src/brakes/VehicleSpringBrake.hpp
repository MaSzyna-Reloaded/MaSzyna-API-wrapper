#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
namespace godot {
    class VehicleController;
    class VehicleSpringBrake : public VehicleComponent {
            GDCLASS(VehicleSpringBrake, VehicleComponent);

        private:
            int state_base_index = 0;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _declare_state_properties() override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override {};
            void _register_commands() override;
            void _unregister_commands() override;

            enum StateProperty {
                STATE_IS_READY,
                STATE_SHUT_OFF,
                STATE_ACTIVE,
                STATE_CYLINDER_PRESSURE,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

            static void _bind_methods();
            MAKE_MEMBER_GS(float, spring_actuator_chamber_volume, 1.0f);
            MAKE_MEMBER_GS(float, spring_actuator_max_filling_force, 0.0f);
            MAKE_MEMBER_GS(float, pressure_force_coefficient, 0.0f);
            MAKE_MEMBER_GS(float, spring_actuator_preload_pressure, 0.0f);
            MAKE_MEMBER_GS(float, spring_full_balance_pressure, 0.0f);
            MAKE_MEMBER_GS(float, brake_signal_released_state_pressure, 0.0f);
            MAKE_MEMBER_GS(float, brake_signal_braked_state_pressure, 0.0f);
            MAKE_MEMBER_GS(float, valve_cross_section_actuator_discharge, 0.0f);
            MAKE_MEMBER_GS(float, valve_cross_section_actuator_charge, 0.0f);
            MAKE_MEMBER_GS(float, valve_cross_section_pneumatic_brake, 0.0f);
            MAKE_MEMBER_GS(int, required_coupler_connection_method, 0.0f);

            void set_spring_brake_active(bool p_active);
            void set_spring_brake_enabled(bool p_active);
            void spring_brake_release();
    };
} // namespace godot
