#pragma once
#include "../core/TrainPart.hpp"
#include "../engines/TrainElectricEngine.hpp"
#include "../core/TrainNode.hpp"
#include "macros.hpp"
#include "resources/lighting/LightListItem.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class TrainLighting : public TrainPart {
            GDCLASS(TrainLighting, TrainPart);

        private:
            static void _bind_methods();
            const TrainNode *_controller = memnew(TrainNode);
            TypedArray<LightListItem> light_position_list;
        protected:
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) override;
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            static const char* SELECTOR_POSITION_CHANGED_SIGNAL;
            MAKE_MEMBER_GS_DIRTY(int, selector_position, 0);
            MAKE_MEMBER_GS(bool, wrap_light_selector, false);
            MAKE_MEMBER_GS(int, default_selector_position, 0);
            MAKE_MEMBER_GS_NR(TrainNode::TrainPowerSource, light_source, TrainNode::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS_NR(TrainEngine::EngineType, generator_engine, TrainEngine::EngineType::MAIN);
            MAKE_MEMBER_GS(double, max_accumulator_voltage, 0.0);
            MAKE_MEMBER_GS_NR(TrainNode::TrainPowerSource, alternative_light_source, TrainNode::TrainPowerSource::POWER_SOURCE_ACCUMULATOR);
            MAKE_MEMBER_GS(double, alternative_max_voltage, 24.0);
            MAKE_MEMBER_GS(double, alternative_light_capacity, 495.0);
            MAKE_MEMBER_GS_NR(TrainNode::TrainPowerSource, accumulator_recharge_source, TrainNode::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS(Color, head_light_color, Color(255, 255, 255));
            MAKE_MEMBER_GS(double, dimming_multiplier, 0.6);
            MAKE_MEMBER_GS(double, normal_multiplier, 1.0);
            MAKE_MEMBER_GS(double, high_beam_dimmed_multiplier, 2.5);
            MAKE_MEMBER_GS(double, high_beam_multiplier, 2.8);
            MAKE_MEMBER_GS(int, instrument_light_type, 0);
            TypedArray<LightListItem> get_light_position_list() {
                return light_position_list;
            };

            void set_light_position_list(const TypedArray<LightListItem> &p_list) {
                light_position_list.clear();
                light_position_list.append_array(p_list);
            };
            void increase_light_selector_position();
            void decrease_light_selector_position();
    };
} // namespace godot
