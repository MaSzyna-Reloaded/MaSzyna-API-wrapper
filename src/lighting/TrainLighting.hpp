#pragma once
#include "../core/TrainPart.hpp"
#include "../engines/TrainElectricEngine.hpp"
#include "macros.hpp"
#include "resources/lighting/LightListItem.hpp"
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

namespace godot {
    class TrainController;
    class TrainLighting : public TrainPart {
            GDCLASS(TrainLighting, TrainPart);

        public:
            enum LightEnd { LIGHT_END_FRONT, LIGHT_END_REAR };
            enum LightType {
                LIGHT_TYPE_HEADLIGHT_UPPER,
                LIGHT_TYPE_HEADLIGHT_LEFT,
                LIGHT_TYPE_HEADLIGHT_RIGHT,
                LIGHT_TYPE_REDMARKER_LEFT,
                LIGHT_TYPE_REDMARKER_RIGHT,
            };

        private:
            static void _bind_methods();
            TypedArray<LightListItem> light_position_list;
            bool roof_light_active = false;
            bool devices_light_active = false;

            // This wrapper's own types for vehicle end / light kind - internal logic works with
            // these, never with Maszyna::end / Maszyna::light directly. The maps below are the
            // only place that translates to/from the Maszyna:: side (mirrors e.g.
            // TrainDoors::Controls / door_controls_map, TrainController::StartMode /
            // start_mode_map).
            const std::unordered_map<LightEnd, Maszyna::end> light_end_map = {
                    {LIGHT_END_FRONT, Maszyna::end::front},
                    {LIGHT_END_REAR, Maszyna::end::rear},
            };

            const std::unordered_map<LightType, int> light_type_mask_map = {
                    {LIGHT_TYPE_HEADLIGHT_UPPER, Maszyna::light::headlight_upper},
                    {LIGHT_TYPE_HEADLIGHT_LEFT, Maszyna::light::headlight_left},
                    {LIGHT_TYPE_HEADLIGHT_RIGHT, Maszyna::light::headlight_right},
                    {LIGHT_TYPE_REDMARKER_LEFT, Maszyna::light::redmarker_left},
                    {LIGHT_TYPE_REDMARKER_RIGHT, Maszyna::light::redmarker_right},
            };

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) override;
            void _do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static const char *selector_position_changed_signal;
            MAKE_MEMBER_GS_DIRTY(int, selector_position, 0);
            MAKE_MEMBER_GS(bool, wrap_light_selector, false);
            MAKE_MEMBER_GS(int, default_selector_position, 0);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, light_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS_NR(TrainEngine::EngineType, generator_engine, TrainEngine::EngineType::MAIN);
            MAKE_MEMBER_GS(double, max_accumulator_voltage, 0.0);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, alternative_light_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_ACCUMULATOR);
            MAKE_MEMBER_GS(double, alternative_max_voltage, 24.0);
            MAKE_MEMBER_GS(double, alternative_light_capacity, 495.0);
            MAKE_MEMBER_GS_NR(
                    TrainController::TrainPowerSource, accumulator_recharge_source,
                    TrainController::TrainPowerSource::POWER_SOURCE_GENERATOR);
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
            // Direct per-light override, independent of the selector/"light programator"
            // (LightsPos + light_position_list) system above - sets/clears a single bit of
            // iLights directly, for debugging/testing individual bulbs regardless of what the
            // programator would normally compute. p_light matches the short name half of this
            // class's own state keys (state key = "lights/" + p_light + "_enabled"): e.g.
            // "front_headlight_left", "rear_redmarker_right".
            void light(const String &p_light, bool p_enabled);
            // Cab-relative toggle for the actual MMD cabin switches (upperlight_sw:/leftlight_sw:
            // /rightlight_sw:/leftend_sw:/rightend_sw:/rearupperlight_sw:/rearleftlight_sw:/
            // rearrightlight_sw:/rearleftend_sw:/rearrightend_sw:) - resolves which physical end
            // to toggle from the currently active cab, unlike light() above (a fixed-end direct
            // override for debugging). p_light is the MMD label's own suffix, e.g. "upper",
            // "left", "leftend", "rearupper", "rearleftend".
            void light_switch(const String &p_light, bool p_enabled);
            // Cab interior lamp ("cablight_sw:") and instrument/dashboard backlighting
            // ("instrumentlight_sw:") - both plain manual toggles with no counterpart on the
            // wrapped mover itself, gated only by 24V/110V power availability (mirrors the
            // original engine's own "cablightlevel"/"lightpower" power gating, vehicle/Train.cpp).
            void roof_light(bool p_enabled);
            void devices_light(bool p_enabled);
    };
} // namespace godot
