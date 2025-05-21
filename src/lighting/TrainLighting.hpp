#pragma once
#include "../core/TrainPart.hpp"
#include "../engines/TrainElectricEngine.hpp"
#include "resources/lighting/LightListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainController;
    class TrainLighting : public TrainPart {
            GDCLASS(TrainLighting, TrainPart);

        private:
            static void _bind_methods();
            const TrainController *_controller = memnew(TrainController);
        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            static const char* SELECTOR_POSITION_CHANGED_SIGNAL;

            TypedArray<LightListItem> light_position_list;

            int selector_position = 0;

            /**
             * Whether a selector can turn in 360 deg or not
             */
            bool wrap_light_selector = false;

            /**
             * Default position of the light selector
             */
            int default_selector_position = 0;

            /**
             * Generator for light power. Default value taken from internal mover
             */
            TrainController::TrainPowerSource light_source = TrainController::TrainPowerSource::POWER_SOURCE_GENERATOR;

            /**
             * Engine type of the generator
             */
            TrainEngine::EngineType generator_engine = TrainEngine::EngineType::MAIN;

            /**
             * Accumulator capacity for an alternative light source. Default value from internal mover
             */
            double max_accumulator_voltage = 0.0;

            /**
             * Alternative generator for light power. Default value taken from internal mover
             */
            TrainController::TrainPowerSource alternative_light_source = TrainController::TrainPowerSource::POWER_SOURCE_ACCUMULATOR;

            /**
             * Accumulator capacity for an alternative light power source. Default value from internal mover
             */
            double alternative_max_voltage = 24.0;

            /**
             * Accumulator capacity for an alternative light power source. Default value from internal mover
             */
            double alternative_light_capacity = 495.0;

            /**
             * Accumulator recharge source for an alternative light power source. Default value from internal mover
             */
            TrainController::TrainPowerSource accumulator_recharge_source = TrainController::TrainPowerSource::POWER_SOURCE_GENERATOR;

            Color head_light_color = Color(255, 255, 255);
            double dimming_multiplier = 0.6;
            double normal_multiplier = 1.0;
            double high_beam_dimmed_multiplier = 2.5;
            double high_beam_multiplier = 2.8;
            int instrument_light_type = 0; // ABu 030405 - swiecenie uzaleznione od: 0-nic, 1-obw.gl, 2-przetw., 3-rozrzad, 4-external lights
            void set_light_source(TrainController::TrainPowerSource p_light_source);
            TrainController::TrainPowerSource get_light_source() const;
            void set_generator_engine(TrainEngine::EngineType p_generator_engine);
            TrainEngine::EngineType get_generator_engine() const;
            void set_max_accumulator_voltage(double p_max_accumulator_voltage);
            double get_max_accumulator_voltage() const;
            void set_alternative_light_source(TrainController::TrainPowerSource p_light_source);
            TrainController::TrainPowerSource get_alternative_light_source() const;
            void set_alternative_max_voltage(double p_max_accumulator_voltage);
            double get_alternative_max_voltage() const;
            void set_alternative_light_capacity(double p_max_accumulator_voltage);
            double get_alternative_light_capacity() const;
            void set_accumulator_recharge_source(TrainController::TrainPowerSource p_accumulator_recharge_source);
            TrainController::TrainPowerSource get_accumulator_recharge_source() const;
            void set_light_position_list(const TypedArray<LightListItem> &p_light_position_list);
            TypedArray<LightListItem> get_light_position_list() const;
            void set_wrap_light_selector(bool p_wrap_light_selector);
            bool get_wrap_light_selector() const;
            void set_default_selector_position(int p_selector_position);
            int get_default_selector_position() const;
            void set_selector_position(int p_selector_position);
            int get_selector_position() const;
            void set_head_light_color(Color p_head_light_color);
            Color get_head_light_color() const;
            void set_dimming_multiplier(double p_dimming_multiplier);
            double get_dimming_multiplier() const;
            void set_normal_multiplier(double p_normal_multiplier);
            double get_normal_multiplier() const;
            void set_high_beam_dimmed_multiplier(double p_high_beam_dimmed_multiplier);
            double get_high_beam_dimmed_multiplier() const;
            void set_high_beam_multiplier(double p_high_beam_multiplier);
            double get_high_beam_multiplier() const;
            void increase_light_selector_position();
            void decrease_light_selector_position();

    };
} // namespace godot
