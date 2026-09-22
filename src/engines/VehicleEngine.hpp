#pragma once
#include "../core/VehicleComponent.hpp"
#include "../maszyna/McZapkie/MOVER.h"
#include "macros.hpp"
#include "resources/engines/MotorParameter.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleController;
    class VehicleEngine : public VehicleComponent {
            GDCLASS(VehicleEngine, VehicleComponent)
            enum StateProperty {
                STATE_MAIN_SWITCH_ENABLED,
                STATE_MAIN_SWITCH_CLOSABLE,
                STATE_ENGINE_TYPE,
                STATE_MM,
                STATE_MW,
                STATE_FW,
                STATE_FT,
                STATE_IM,
                STATE_COMPRESSOR_ENABLED,
                STATE_COMPRESSOR_ALLOWED,
                STATE_ENGINE_POWER,
                STATE_DYNAMIC_BRAKE_ACTIVE,
                STATE_ENGINE_RPM_COUNT,
                STATE_ENGINE_RPM_RATIO,
                STATE_ENGINE_CURRENT,
                STATE_CIRCUIT_IMAX,
                STATE_CIRCUIT_NMAX_RPM,
                STATE_ENGINE_DAMAGE,
                STATE_MAIN_SWITCH_TIME,
                STATE_MAIN_NO_POWER_POS,
                STATE_CAMSHAFT_AVAILABLE,
                STATE_CONVERTER_OVERLOAD,
                STATE_LINE_BREAKER_DELAY,
                STATE_LINE_BREAKER_INITIAL_DELAY,
                STATE_LINE_BREAKER_CLOSES_AT_NO_POWER,
                STATE_FUSE_ACTIVE,
                STATE_MOTOR_CONNECTORS_OPEN,
            };

        public:
            Variant _get_state_property(int p_local_index) const override;

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

            const std::map<StartMode, Maszyna::start_t> start_mode_map = {
                    {START_MODE_DISABLED, Maszyna::start_t::disabled},
                    {START_MODE_MANUAL, Maszyna::start_t::manual},
                    {START_MODE_AUTOMATIC, Maszyna::start_t::automatic},
                    {START_MODE_MANUAL_WITH_AUTO_FALLBACK, Maszyna::start_t::manualwithautofallback},
                    {START_MODE_CONVERTER, Maszyna::start_t::converter},
                    {START_MODE_BATTERY, Maszyna::start_t::battery},
                    {START_MODE_DIRECTION, Maszyna::start_t::direction},
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

            const std::map<EngineType, TEngineType> engine_type_map = {
                    {NONE, TEngineType::None},
                    {DUMB, TEngineType::Dumb},
                    {WHEELS_DRIVEN, TEngineType::WheelsDriven},
                    {ELECTRIC_SERIES_MOTOR, TEngineType::ElectricSeriesMotor},
                    {ELECTRIC_INDUCTION_MOTOR, TEngineType::ElectricInductionMotor},
                    {DIESEL, TEngineType::DieselEngine},
                    {STEAM, TEngineType::SteamEngine},
                    {DIESEL_ELECTRIC, TEngineType::DieselElectric},
                    {MAIN, TEngineType::Main}};


            TypedArray<MotorParameter> get_motor_param_table() {
                return motor_param_table;
            }

            void set_motor_param_table(const TypedArray<MotorParameter> &p_motor_param_table) {
                motor_param_table.clear();
                motor_param_table.append_array(p_motor_param_table);
            }

            bool main_switch(bool p_enabled);
            void fuse_reset();
            void motor_connectors_open(bool p_open);
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
            int state_base_index = 0;

        protected:
            /// Change detection for engine_start/engine_stop, compared in _do_process_mover().
            /// Starts false so a vehicle coming up with the main switch already closed reports
            /// engine_start on its first tick, as it did when this was compared against the
            /// not-yet-filled state dictionary.
            bool previous_main_switch = false;

            virtual EngineType get_engine_type() const = 0;
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;
            void _declare_state_properties() override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleEngine::EngineType);
VARIANT_ENUM_CAST(VehicleEngine::StartMode);
VARIANT_ENUM_CAST(VehicleEngine::EimControlType);
VARIANT_ENUM_CAST(VehicleEngine::AutoRelayMode);
