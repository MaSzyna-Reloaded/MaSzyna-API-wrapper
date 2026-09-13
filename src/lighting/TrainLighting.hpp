#pragma once
#include "../core/TrainPart.hpp"
#include "../engines/TrainElectricEngine.hpp"
#include "macros.hpp"
#include "resources/lighting/LightListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainController;
    class TrainLighting : public TrainPart {
            GDCLASS(TrainLighting, TrainPart);

        private:
            static void _bind_methods();
            TypedArray<LightListItem> lights_list;

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static const char *selector_position_changed_signal;
            MAKE_MEMBER_GS_DIRTY(int, lights_selector_position, 0);
            MAKE_MEMBER_GS(bool, lights_wrap_selector, false);
            MAKE_MEMBER_GS(int, lights_default_selector_position, 0);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, light_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS_NR(TrainEngine::EngineType, source_generator_engine, TrainEngine::EngineType::MAIN);
            MAKE_MEMBER_GS(double, source_accumulator_max_voltage, 0.0);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, light_alternative_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_ACCUMULATOR);
            MAKE_MEMBER_GS(double, light_alternative_max_voltage, 24.0);
            MAKE_MEMBER_GS(double, light_alternative_capacity, 495.0);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, source_accumulator_recharge_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS(Color, head_light_color, Color(255, 255, 255));
            MAKE_MEMBER_GS(double, head_light_dimmed_multiplier, 0.6);
            MAKE_MEMBER_GS(double, head_light_normal_multiplier, 1.0);
            MAKE_MEMBER_GS(double, head_light_high_beam_dimmed_multiplier, 2.5);
            MAKE_MEMBER_GS(double, head_light_high_beam_normal_multiplier, 2.8);
            MAKE_MEMBER_GS(int, instrument_type, 0);
            TypedArray<LightListItem> get_lights_list() {
                return lights_list;
            };

            void set_lights_list(const TypedArray<LightListItem> &p_list) {
                lights_list.clear();
                lights_list.append_array(p_list);
            };
            void increase_light_selector_position();
            void decrease_light_selector_position();
    };
} // namespace godot
