#pragma once
#include "TrainEngine.hpp"
#include "macros.hpp"

namespace godot {
    class TrainController;

    class TrainElectricEngine : public TrainEngine {
            GDCLASS(TrainElectricEngine, TrainEngine)
        private:
            const TrainController *_controller = memnew(TrainController);

        public:
            static void _bind_methods();
            TrainController::TrainPowerSource power_source = TrainController::POWER_SOURCE_NOT_DEFINED;
            MAKE_MEMBER_GS(int, collectors_no, 0);
            MAKE_MEMBER_GS(float, max_voltage, 0.0);
            MAKE_MEMBER_GS(float, max_current, 0.0);
            MAKE_MEMBER_GS(float, min_collector_lifting, 0.0);
            MAKE_MEMBER_GS(float, max_collector_lifting, 0.0);
            MAKE_MEMBER_GS(float, collector_sliding_width, 0.0);
            MAKE_MEMBER_GS(float, min_main_switch_voltage, 0.5f * max_voltage);
            MAKE_MEMBER_GS(float, min_pantograph_tank_pressure, 0.0);
            MAKE_MEMBER_GS(float, max_pantograph_tank_pressure, 0.0);
            MAKE_MEMBER_GS_DIRTY(bool, overvoltage_relay, false);
            MAKE_MEMBER_GS(float, required_main_switch_voltage, 0.6f * max_voltage);
            MAKE_MEMBER_GS(float, transducer_input_voltage, 0.0f);
            MAKE_MEMBER_GS_NR(TrainController::TrainPowerSource, accumulator_recharge_source, TrainController::TrainPowerSource::POWER_SOURCE_NOT_DEFINED);
            MAKE_MEMBER_GS_NR(TrainController::TrainPowerType, power_cable_power_source, TrainController::TrainPowerType::POWER_TYPE_NONE);
            MAKE_MEMBER_GS(float, power_cable_steam_pressure, 0.0f);

            void set_engine_power_source(TrainController::TrainPowerSource p_source);
            TrainController::TrainPowerSource get_engine_power_source() const;
            void compressor(bool p_enabled);
            void converter(bool p_enabled);
            void _register_commands() override;
            void _unregister_commands() override;
            //@TODO: Implement bitmask for PhysicalLayout

        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
    };
} // namespace godot
