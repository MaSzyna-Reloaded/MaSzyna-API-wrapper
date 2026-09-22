#pragma once
#include "../core/VehicleComponent.hpp"
#include "../engines/VehicleElectricEngine.hpp"
#include "macros.hpp"
#include "resources/lighting/LightListItem.hpp"
#include <godot_cpp/classes/node.hpp>
#include <unordered_map>

namespace godot {
    class VehicleController;
    class VehicleLighting : public VehicleComponent {
            GDCLASS(VehicleLighting, VehicleComponent);

            
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;

            /* Compartment (roof) light, as the vehicle's own signal reports it: lit only while the
             * lighting circuit is fed. Read straight from the backend - nothing is stored. */
            bool get_roof_light_enabled() const;

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
            // VehicleDoors::Controls / door_controls_map, VehicleController::StartMode /
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

        private:
            /* Confirmed against vehicle/Train.cpp:9199-9208 - the i-upperlight/i-leftlight lamps
             * read the mover's own already-resolved per-end bitmask (iLights), not the selector
             * position or the LightListItem table. */
            bool _light_enabled(const TMoverParameters *p_mover, LightEnd p_end, LightType p_type) const;
            /* Confirmed against vehicle/Train.h:220-227 (TTrain::cab_to_end()) and
             * Train.cpp:5267-5316: the upperlight_sw/leftlight_sw switches (no "rear" prefix)
             * toggle whichever physical end the ACTIVE cab faces, so "active"/"opposite" are
             * cab-relative - unlike the fixed physical front/rear the indicator lamps read.
             * CabActive (-1/0/1) mirrors iCabn's own front/rear meaning. */
            static LightEnd _active_end(const TMoverParameters *p_mover);
            static LightEnd _opposite_end(const TMoverParameters *p_mover);
            /* Cab interior lamp and instrument backlighting have no counterpart on the mover -
             * they are gated only by 24V/110V availability, as the original gates
             * "cablightlevel"/"lightpower" (vehicle/Train.cpp). */
            static bool _is_powered(const TMoverParameters *p_mover);

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _register_commands() override;
            void _unregister_commands() override;

        public:
            static const char *selector_position_changed_signal;
            MAKE_MEMBER_GS_DIRTY(int, lights_selector_position, 0);
            MAKE_MEMBER_GS(bool, lights_wrap_selector, false);
            MAKE_MEMBER_GS(int, lights_default_selector_position, 0);
            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerSource, light_source,
                    VehicleController::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS_NR(VehicleEngine::EngineType, source_generator_engine, VehicleEngine::EngineType::MAIN);
            MAKE_MEMBER_GS(double, source_accumulator_max_voltage, 0.0);
            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerSource, light_alternative_source,
                    VehicleController::TrainPowerSource::POWER_SOURCE_ACCUMULATOR);
            MAKE_MEMBER_GS(double, light_alternative_max_voltage, 24.0);
            MAKE_MEMBER_GS(double, light_alternative_capacity, 495.0);
            MAKE_MEMBER_GS_NR(
                    VehicleController::TrainPowerSource, source_accumulator_recharge_source,
                    VehicleController::TrainPowerSource::POWER_SOURCE_GENERATOR);
            MAKE_MEMBER_GS(Color, head_light_color, Color(255, 255, 255));
            MAKE_MEMBER_GS(double, head_light_dimmed_multiplier, 0.6);
            MAKE_MEMBER_GS(double, head_light_normal_multiplier, 1.0);
            MAKE_MEMBER_GS(double, head_light_high_beam_dimmed_multiplier, 2.5);
            MAKE_MEMBER_GS(double, head_light_high_beam_normal_multiplier, 2.8);
            MAKE_MEMBER_GS(int, instrument_type, 0);
            TypedArray<LightListItem> get_lights_list() {
                return light_position_list;
            };

            void set_lights_list(const TypedArray<LightListItem> &p_list) {
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
