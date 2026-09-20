#pragma once
#include "TrainEngine.hpp"
#include "macros.hpp"

namespace godot {
    class TrainController;

    class TrainElectricEngine : public TrainEngine {
            GDCLASS(TrainElectricEngine, TrainEngine)

        public:
            /* Which pantograph an individual command applies to - the mover supports at most
             * two (Maszyna::end::front / ::rear); named FIRST/SECOND here rather than
             * FRONT/REAR since which end is physically "front" depends on the active cab. */
            enum PantographSelector {
                PANTOGRAPH_FIRST,
                PANTOGRAPH_SECOND,
            };

            static void _bind_methods();
            TrainController::TrainPowerSource power_source = TrainController::POWER_SOURCE_NOT_DEFINED;
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
            MAKE_MEMBER_GS_DIRTY(bool, power_current_collector_overvoltage_relay, false);
            MAKE_MEMBER_GS(
                    float, power_current_collector_required_main_switch_voltage,
                    0.6f * power_current_collector_max_voltage);
            MAKE_MEMBER_GS(float, power_transducer_input_voltage, 0.0f);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, power_accumulator_recharge_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_NOT_DEFINED);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerType, power_cable_source,
                    TrainController::TrainPowerType::POWER_TYPE_NONE);
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
            MAKE_MEMBER_GS_NR(TrainEngine::StartMode, cntrl_converter_start_mode, TrainEngine::START_MODE_MANUAL);
            MAKE_MEMBER_GS(double, cntrl_converter_start_delay, 0.0);
            MAKE_MEMBER_GS_NR(
                    TrainEngine::StartMode, cntrl_converter_overload_relay_start_mode, TrainEngine::START_MODE_MANUAL);
            MAKE_MEMBER_GS(bool, cntrl_converter_overload_relay_off_when_main_is_off, false);
            MAKE_MEMBER_GS_NR(
                    TrainEngine::StartMode, cntrl_pantograph_compressor_start_mode, TrainEngine::START_MODE_MANUAL);
            MAKE_MEMBER_GS(bool, cntrl_pantograph_auto_valve, false);
            MAKE_MEMBER_GS_NR(TrainEngine::StartMode, cntrl_main_switch_start_mode, TrainEngine::START_MODE_MANUAL);

            /* Voltage of the overhead wire each pantograph is currently touching, fed in once
             * per frame from outside (RailVehicle3D's own geometric wire lookup against
             * TractionPowerServer - the mover has no scenery/geometry access of its own).
             * 0.0 (the default) means "not touching a wire", same as a lowered pantograph. */
            float pantograph_first_wire_voltage = 0.0f;
            float pantograph_second_wire_voltage = 0.0f;

            void set_power_source(TrainController::TrainPowerSource p_source);
            TrainController::TrainPowerSource get_power_source() const;
            void compressor(bool p_enabled);
            void converter(bool p_enabled);
            void converter_fuse_reset();
            void pantographs_valve(bool p_enabled);
            void pantographs_drop_all(bool p_enabled);
            void pantograph_compressor(bool p_enabled);
            void pantograph_compressor_valve(bool p_to_compressor);
            void pantograph(PantographSelector p_selector, bool p_enabled);
            void set_pantograph_wire_voltage(PantographSelector p_selector, float p_voltage);
            void _register_commands() override;
            void _unregister_commands() override;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
    };
} // namespace godot

VARIANT_ENUM_CAST(TrainElectricEngine::PantographSelector);
