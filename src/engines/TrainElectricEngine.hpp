#pragma once
#include "TrainEngine.hpp"
#include "macros.hpp"

namespace godot {
    class TrainController;

    class TrainElectricEngine : public TrainEngine {
            GDCLASS(TrainElectricEngine, TrainEngine)

        public:
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

            void set_power_source(TrainController::TrainPowerSource p_source);
            TrainController::TrainPowerSource get_power_source() const;
            void compressor(bool p_enabled);
            void converter(bool p_enabled);
            void _register_commands() override;
            void _unregister_commands() override;
            //@TODO: Implement bitmask for PhysicalLayout

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
    };
} // namespace godot
