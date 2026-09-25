#pragma once
#include "VehicleEngine.hpp"
#include "macros.hpp"

namespace godot {
    class VehicleController;
    class VehicleElectricEngineBackend;

    class VehicleElectricEngine : public VehicleEngine {
            GDCLASS(VehicleElectricEngine, VehicleEngine)

            

        protected:
            /* The simulation answering these values, installed by the implementation that owns it */
            const VehicleElectricEngineBackend *electric_backend = nullptr;

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;

            /* Not every electric engine is fed the same way; these say whether the matching
             * values mean anything at all, so the dump can leave their keys out and a caller's
             * has() keeps meaning what it meant. */
            bool has_accumulator() const;
            bool has_power_cable() const;


            /* Fed from the catenary - a diesel has none of these */
            bool get_camshaft_available() const;
            bool get_converter_overload() const;
            double get_line_breaker_delay() const;
            double get_line_breaker_initial_delay() const;
            bool get_line_breaker_closes_at_no_power() const;

            /* Electric traction motors; a diesel-electric declares the same five
             * and forwards to the same delegate (VehicleElectricTraction.hpp) */
            virtual double get_motor_current() const = 0;
            virtual double get_circuit_imax() const = 0;
            virtual bool get_dynamic_brake_active() const = 0;
            virtual bool get_fuse_active() const = 0;
            virtual bool get_motor_connectors_open() const = 0;

            /* Traction commands - "zbij nadmiarowy" and the line contactors */
            virtual void fuse_reset() = 0;
            virtual void set_motor_connectors_open(bool p_open) = 0;

            /* Live state, read straight from the backend - nothing is stored. */
            bool get_converter_enabled() const;
            bool get_converted_allowed() const;
            double get_converter_time_to_start() const;
            double get_collector_max_voltage() const;
            double get_collector_max_current() const;
            double get_collector_max_lifting() const;
            double get_collector_min_lifting() const;
            double get_collector_sliding_width() const;
            double get_collector_min_main_switch_voltage() const;
            double get_collector_min_pantograph_tank_pressure() const;
            double get_collector_max_pantograph_tank_pressure() const;
            double get_collector_pantograph_tank_pressure() const;
            bool get_collector_pantograph_pressure_switch_armed() const;
            bool get_collector_pantograph_compressor_valve() const;
            /* The small compressor filling the pantograph tank is running (PantCompFlag) */
            bool get_collector_pantograph_compressor_enabled() const;
            bool get_collector_overvoltage_relay() const;
            double get_collector_required_main_switch_voltage() const;
            bool get_collector_valve_active() const;
            bool get_collector_valve_enabled() const;
            bool get_collector_pantographs_dropped() const;
            bool get_collector_pantograph_first_active() const;
            /* A pantograph's own valve (Pantographs[].valve.is_enabled) - what pantfront_sw and
             * pantrear_sw flip, together with is_active (Train.cpp:3161) */
            bool get_collector_pantograph_first_valve_enabled() const;
            bool get_collector_pantograph_second_valve_enabled() const;
            double get_collector_pantograph_first_voltage() const;
            bool get_collector_pantograph_second_active() const;
            double get_collector_pantograph_second_voltage() const;
            double get_collector_voltage() const;
            bool get_contactors_active() const;
            bool get_diff_relay_active() const;
            bool get_resistors_active() const;
            bool get_vent_overload_active() const;
            bool get_highcurrent_active() const;
            bool get_mainbreaker_active() const;
            double get_transducer_input_voltage() const;

            /* Which pantograph an individual command applies to - a vehicle has at most two
             * (the front and the rear one); named FIRST/SECOND here rather than
             * FRONT/REAR since which end is physically "front" depends on the active cab. */
            enum PantographSelector {
                PANTOGRAPH_FIRST,
                PANTOGRAPH_SECOND,
            };
            /* What an operation does to the pantographs' master valve - Maszyna's operation_t
             * (MOVER.h:177): ENABLE/DISABLE set it for a two-state switch, the *_ON/*_OFF pairs
             * press and release one side of an impulse switch, NONE lets both sides go. */
            enum ValveOperation {
                VALVE_OPERATION_NONE,
                VALVE_OPERATION_ENABLE,
                VALVE_OPERATION_DISABLE,
                VALVE_OPERATION_ENABLE_ON,
                VALVE_OPERATION_ENABLE_OFF,
                VALVE_OPERATION_DISABLE_ON,
                VALVE_OPERATION_DISABLE_OFF,
            };

            static void _bind_methods();
            VehicleController::TrainPowerSource power_source = VehicleController::POWER_SOURCE_NOT_DEFINED;
            MAKE_MEMBER_GS(int, power_current_collector_number_of_collectors, 0);
            MAKE_MEMBER_GS(float, power_current_collector_max_voltage, 0.0);
            MAKE_MEMBER_GS(float, power_current_collector_max_current, 0.0);
            MAKE_MEMBER_GS(float, power_current_collector_min_collector_lifting, 0.0);
            MAKE_MEMBER_GS(float, power_current_collector_max_collector_lifting, 0.0);
            MAKE_MEMBER_GS(float, power_current_collector_sliding_width, 0.0);
            MAKE_MEMBER_GS(
                    float, power_current_collector_min_main_switch_voltage, 0.5f * power_current_collector_max_voltage);
            MAKE_MEMBER_GS(float, power_current_collector_min_pantograph_tank_pressure, 0.0);
            MAKE_MEMBER_GS(float, power_current_collector_max_pantograph_tank_pressure, 0.0);
            MAKE_MEMBER_GS(bool, power_current_collector_overvoltage_relay, false);
            MAKE_MEMBER_GS(
                    float, power_current_collector_required_main_switch_voltage,
                    0.6f * power_current_collector_max_voltage);
            MAKE_MEMBER_GS(float, power_transducer_input_voltage, 0.0f);
            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerSource, power_accumulator_recharge_source,
                    VehicleController::TrainPowerSource::POWER_SOURCE_NOT_DEFINED);
            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerType, power_cable_source,
                    VehicleController::TrainPowerType::POWER_TYPE_NONE);
            MAKE_MEMBER_GS(float, power_cable_steam_pressure, 0.0f);
            MAKE_MEMBER_GS(int, power_current_collector_physical_layout, 0);

            /* Circuit: (elektryczny obwod napedowy) */
            MAKE_MEMBER_GS(double, circuit_resistance, 0.0);
            MAKE_MEMBER_GS(int, circuit_imax_low, 0);
            MAKE_MEMBER_GS(int, circuit_imax_high, 0);
            MAKE_MEMBER_GS(int, circuit_imin_low, 0);
            MAKE_MEMBER_GS(int, circuit_imin_high, 0);
            MAKE_MEMBER_GS(double, circuit_tuhex_sum, 750.0);
            MAKE_MEMBER_GS(double, circuit_tuhex_diff, 10.0);
            MAKE_MEMBER_GS(double, circuit_tuhex_min_current, 60.0);
            MAKE_MEMBER_GS(double, circuit_tuhex_max_current, 400.0);
            MAKE_MEMBER_GS(int, circuit_tuhex_stages, 0);
            MAKE_MEMBER_GS(double, circuit_tuhex_sum_1, 750.0);
            MAKE_MEMBER_GS(double, circuit_tuhex_sum_2, 750.0);
            MAKE_MEMBER_GS(double, circuit_tuhex_sum_3, 750.0);

            /* Cntrl. (elektryczne) */
            MAKE_MEMBER_GS_NR(
                    VehicleEngine::StartMode, cntrl_converter_overload_relay_start_mode, VehicleEngine::START_MODE_MANUAL);
            MAKE_MEMBER_GS(bool, cntrl_converter_overload_relay_off_when_main_is_off, false);
            MAKE_MEMBER_GS_NR(
                    VehicleEngine::StartMode, cntrl_pantograph_compressor_start_mode, VehicleEngine::START_MODE_MANUAL);
            MAKE_MEMBER_GS(bool, cntrl_pantograph_auto_valve, false);
            /* The pantographs' valves (LoadFIZ_Cntrl, Mover.cpp:10927-10946): the master valve of
             * them all opens by itself unless the FIZ says otherwise - "there was no pantographs
             * valve" in older vehicles - while each pantograph's own valve is worked by hand */
            MAKE_MEMBER_GS_NR(
                    VehicleEngine::StartMode, cntrl_pantographs_valve_start_mode, VehicleEngine::START_MODE_AUTOMATIC);
            MAKE_MEMBER_GS(bool, cntrl_pantographs_valve_spring, true);
            MAKE_MEMBER_GS_NR(
                    VehicleEngine::StartMode, cntrl_pantograph_valve_start_mode, VehicleEngine::START_MODE_MANUAL);
            MAKE_MEMBER_GS(bool, cntrl_pantograph_valve_spring, true);
            MAKE_MEMBER_GS(bool, cntrl_pantograph_valve_solenoid, true);
            MAKE_MEMBER_GS_NR(VehicleEngine::StartMode, cntrl_main_switch_start_mode, VehicleEngine::START_MODE_MANUAL);

            /* Voltage of the overhead wire each pantograph is currently touching, fed in once
             * per frame from outside (RailVehicle3D's own geometric wire lookup against
             * TractionPowerServer - the simulation has no scenery/geometry access of its own).
             * 0.0 (the default) means "not touching a wire", same as a lowered pantograph. */
            float pantograph_first_wire_voltage = 0.0f;
            float pantograph_second_wire_voltage = 0.0f;

            static const char *pantograph_up_signal;
            static const char *pantograph_down_signal;

            void set_power_source(VehicleController::TrainPowerSource p_source);
            VehicleController::TrainPowerSource get_power_source() const;
            void compressor(bool p_enabled);
            void converter(bool p_enabled);
            void converter_fuse_reset();
            void pantographs_valve(bool p_enabled);
            void pantographs_valve_operate(ValveOperation p_operation);
            void pantographs_drop_all(bool p_enabled);
            void pantograph_compressor(bool p_enabled);
            void pantograph_compressor_valve(bool p_to_compressor);
            void pantograph(PantographSelector p_selector, bool p_enabled);
            /* One pantograph's own valve, as the cab operates it (OnCommand_pantographraisefront/
             * lowerfront, Train.cpp:3218-3300) - unlike pantograph(), it leaves the master valve alone */
            void pantograph_valve_operate(PantographSelector p_selector, ValveOperation p_operation);
            void set_pantograph_wire_voltage(PantographSelector p_selector, float p_voltage);
            void _register_commands() override;
            void _unregister_commands() override;

        private:
            /* Change detection for pantograph_up/pantograph_down, compared in
             * _do_process_component() against this part's own members */
            bool previous_pantograph_live[2] = {false, false};
            bool previous_pantograph_active[2] = {false, false};

        protected:
            void _apply_configuration() override;
            void _do_process_component(double p_delta) override;
    };
} // namespace godot

VARIANT_ENUM_CAST(VehicleElectricEngine::PantographSelector);
VARIANT_ENUM_CAST(VehicleElectricEngine::ValveOperation);
