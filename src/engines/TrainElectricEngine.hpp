#pragma once
#include "TrainEngine.hpp"


namespace godot {
    class TrainController;

    class TrainElectricEngine : public TrainEngine {
            GDCLASS(TrainElectricEngine, TrainEngine)
        private:
            const TrainController *_controller = memnew(TrainController);

        public:
            static void _bind_methods();
            TrainController::TrainPowerSource power_source = TrainController::POWER_SOURCE_NOT_DEFINED;
            int collectors_no = 0;
            float max_voltage = 0;
            float max_current = 0;
            /**
             * Minimal collector lifting from the folded position, in meters.
             */
            float min_collector_lifting = 0;
            /**
             * Maximum collector lifting, in meters.
             */
            float max_collector_lifting = 0;
            /**
             * Working width of the collector/pantograph slide, in meters
             */
            float collector_sliding_width = 0;
            /**
             * The minimum closing voltage of the main switch, in volts.
             * By default, it is 0.5*max_voltage
             */
            float min_main_switch_voltage = 0.5f * max_voltage;
            /**
             * Minimum pressure in the Pantograph Tank (PT) to raise the pantograph, in MPa
             */
            float min_pantograph_tank_pressure = 0;
            /**
             * Maximum pressure in the Pantograph Tank
             */
            float max_pantograph_tank_pressure = 0;
            /**
             * Determines if we do have an overvoltage relay on the train
             */
            bool overvoltage_relay = false;
            /**
             * The voltage required to turn on the main switch
             * By default, it is 0.6*max_voltage
             */
            float required_main_switch_voltage = 0.6f * max_voltage;
            float transducer_input_voltage = 0;
            TrainController::TrainPowerSource accumulator_recharge_source =
                    TrainController::TrainPowerSource::POWER_SOURCE_NOT_DEFINED;
            TrainController::TrainPowerType power_cable_power_trans = TrainController::TrainPowerType::POWER_TYPE_NONE;
            float power_cable_steam_pressure = 0;
            void set_engine_power_source(TrainController::TrainPowerSource p_source);
            TrainController::TrainPowerSource get_engine_power_source() const;
            int get_number_of_collectors() const;
            void set_number_of_collectors(int p_coll_no);
            void set_max_voltage(float p_max_voltage);
            float get_max_voltage() const;
            void set_max_current(float p_max_current);
            float get_max_current() const;
            void set_max_collector_lifting(float p_max_collector_lifting);
            float get_max_collector_lifting() const;
            void set_min_collector_lifting(float p_min_collector_lifting);
            float get_min_collector_lifting() const;
            float get_csw() const;
            void set_min_main_switch_voltage(float p_min_main_switch_voltage);
            float get_min_main_switch_voltage() const;
            void set_min_pantograph_tank_pressure(float p_min_pantograph_tank_pressure);
            float get_min_pantograph_tank_pressure() const;
            void set_max_pantograph_tank_pressure(float p_max_pantograph_tank_pressure);
            float get_max_pantograph_tank_pressure() const;
            void set_overvoltage_relay(bool p_overvoltage_relay_value);
            bool get_overvoltage_relay() const;
            void set_required_main_switch_voltage(float p_required_main_switch_voltage);
            float get_required_main_switch_voltage() const;
            void set_csw(float p_csw);
            void set_transducer_input_voltage(float p_transducer_input_voltage);
            float get_transducer_input_voltage() const;
            void set_accumulator_recharge_source(TrainController::TrainPowerSource p_source);
            TrainController::TrainPowerSource get_accumulator_recharge_source() const;
            void set_power_cable_power_source(TrainController::TrainPowerType p_source);
            TrainController::TrainPowerType get_power_cable_power_source() const;
            void set_power_cable_steam_pressure(float p_pressure);
            float get_power_cable_steam_pressure() const;
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
