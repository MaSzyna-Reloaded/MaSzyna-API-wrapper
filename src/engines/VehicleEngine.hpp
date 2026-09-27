#pragma once
#include "../core/VehicleComponent.hpp"
#include "VehicleEngineBackend.hpp"
#include "macros.hpp"
#include "resources/engines/MotorParameter.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleController;
    class VehicleEngine : public VehicleComponent {
            GDCLASS(VehicleEngine, VehicleComponent)


        public:
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_ENGINE;
            }

        protected:
            /* The simulation answering this engine's live values, installed by the implementation
             * that owns it. The interface never names one. */
            const VehicleEngineBackend *engine_backend = nullptr;

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;

            /* Live state, read straight from the backend - nothing is stored. */
            bool get_main_switch_enabled() const;
            bool get_main_switch_closable() const;
            int get_type() const;
            double get_motor_torque() const;
            double get_wheel_torque() const;
            double get_wheel_force() const;
            double get_tractive_force() const;
            bool get_compressor_enabled() const;
            bool get_compressor_allowed() const;
            double get_power() const;
            double get_rpm_count() const;
            double get_rpm_ratio() const;
            double get_circuit_nmax_rpm() const;
            int get_damage() const;
            double get_main_switch_time() const;
            bool get_main_no_power_pos() const;

            enum EngineType {
                NONE,
                DUMB,
                WHEELS_DRIVEN,
                ELECTRIC_SERIES_MOTOR,
                ELECTRIC_INDUCTION_MOTOR,
                DIESEL,
                STEAM,
                DIESEL_ELECTRIC,
                MAIN
            };

            /* shared enum for every FIZ "...Start=" device activation mode field (Cntrl. section) */
            enum StartMode {
                START_MODE_DISABLED,
                START_MODE_MANUAL,
                START_MODE_AUTOMATIC,
                START_MODE_MANUAL_WITH_AUTO_FALLBACK,
                START_MODE_CONVERTER,
                START_MODE_BATTERY,
                START_MODE_DIRECTION,
            };

            /* EIMCtrlType= : traction lever variant, for vehicles with an EIM-style controller */
            enum EimControlType {
                EIM_CONTROL_TYPE_0,
                EIM_CONTROL_TYPE_1,
                EIM_CONTROL_TYPE_2,
                EIM_CONTROL_TYPE_3,
            };

            /* AutoRelay= : automatic starting relay presence */
            enum AutoRelayMode {
                AUTO_RELAY_NO,
                AUTO_RELAY_YES,
                AUTO_RELAY_OPTIONAL,
            };


            TypedArray<MotorParameter> get_motor_param_table() const {
                return motor_param_table;
            }

            void set_motor_param_table(const TypedArray<MotorParameter> &p_motor_param_table) {
                motor_param_table.clear();
                motor_param_table.append_array(p_motor_param_table);
            }

            bool main_switch(bool p_enabled);
            void compressor(bool p_enabled);
            static void _bind_methods();
            TypedArray<MotorParameter> motor_param_table;

            /* Engine: (wspolne pola dla wszystkich typow napedu) */
            MAKE_MEMBER_GS(int, transmission_gear_teeth_motor, 0);
            MAKE_MEMBER_GS(int, transmission_gear_teeth_wheel, 0);
            MAKE_MEMBER_GS(double, transmission_efficiency, 1.0);
            MAKE_MEMBER_GS(double, maximum_traction_force, 0.0);
            MAKE_MEMBER_GS(double, motor_blowers_speed, 0.0);
            MAKE_MEMBER_GS(double, motor_blowers_sustain_time, 0.0);
            MAKE_MEMBER_GS(double, motor_blowers_start_velocity, -1.0);
            MAKE_MEMBER_GS(bool, pressure_switch_present, false);
            MAKE_MEMBER_GS(int, inverters_count, 0);
            MAKE_MEMBER_GS_NR(StartMode, motor_blowers_start_mode, START_MODE_MANUAL);

            /* Cntrl. (wspolne pola sterowania nastawnikiem i rozrusznikiem) */
            MAKE_MEMBER_GS(int, cntrl_main_controller_position_count, 0);
            MAKE_MEMBER_GS(int, cntrl_shunt_controller_position_count, 0);
            MAKE_MEMBER_GS(int, cntrl_direction_change_max_position, 0);
            MAKE_MEMBER_GS(bool, cntrl_eim_control_additional_zeros, false);
            MAKE_MEMBER_GS(bool, cntrl_eim_control_emergency, false);
            MAKE_MEMBER_GS_NR(EimControlType, cntrl_eim_control_type, EIM_CONTROL_TYPE_0);
            MAKE_MEMBER_GS_NR(AutoRelayMode, cntrl_auto_relay_mode, AUTO_RELAY_NO);
            MAKE_MEMBER_GS(bool, cntrl_coupled_controllers, false);
            MAKE_MEMBER_GS(bool, cntrl_has_camshaft, false);
            MAKE_MEMBER_GS(bool, cntrl_series_shunt_on_series_position, false);
            MAKE_MEMBER_GS(double, cntrl_initial_controller_delay, 0.0);
            MAKE_MEMBER_GS(double, cntrl_controller_step_delay, 0.0);
            MAKE_MEMBER_GS(double, cntrl_controller_step_down_delay, 0.0);
            MAKE_MEMBER_GS(bool, cntrl_fast_series_circuit, false);

        private:
        protected:
            /// Change detection for engine_start/engine_stop, compared in _do_process_component().
            /// Starts false so a vehicle coming up with the main switch already closed reports
            /// engine_start on its first tick, as it did when this was compared against the
            /// not-yet-filled state dictionary.
            bool previous_main_switch = false;


        public:
            /* Which kind of engine this is - part of the contract, and what the simulation keys
             * its own configuration off */
            virtual EngineType get_engine_type() const = 0;

        protected:
            void _apply_configuration() override;
            void _do_process_component(double p_delta) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleEngine::EngineType);
VARIANT_ENUM_CAST(VehicleEngine::StartMode);
VARIANT_ENUM_CAST(VehicleEngine::EimControlType);
VARIANT_ENUM_CAST(VehicleEngine::AutoRelayMode);
