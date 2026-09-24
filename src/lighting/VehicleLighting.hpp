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
            VehicleComponentType::Type get_component_type() const override {
                return VehicleComponentType::COMPONENT_LIGHTING;
            }

        private:
            static void _bind_methods();
        public:
            /* Live state, read straight from the backend - nothing is stored. */
            virtual int get_position() const = 0;
            virtual double get_power() const = 0;
            virtual int get_power_source() const = 0;
            virtual bool get_front_headlight_upper_enabled() const = 0;
            virtual bool get_front_headlight_left_enabled() const = 0;
            virtual bool get_front_headlight_right_enabled() const = 0;
            virtual bool get_front_redmarker_left_enabled() const = 0;
            virtual bool get_front_redmarker_right_enabled() const = 0;
            virtual bool get_rear_headlight_upper_enabled() const = 0;
            virtual bool get_rear_headlight_left_enabled() const = 0;
            virtual bool get_rear_headlight_right_enabled() const = 0;
            virtual bool get_rear_redmarker_left_enabled() const = 0;
            virtual bool get_rear_redmarker_right_enabled() const = 0;
            virtual bool get_active_headlight_upper_enabled() const = 0;
            virtual bool get_active_headlight_left_enabled() const = 0;
            virtual bool get_active_headlight_right_enabled() const = 0;
            virtual bool get_active_redmarker_left_enabled() const = 0;
            virtual bool get_active_redmarker_right_enabled() const = 0;
            virtual bool get_opposite_headlight_upper_enabled() const = 0;
            virtual bool get_opposite_headlight_left_enabled() const = 0;
            virtual bool get_opposite_headlight_right_enabled() const = 0;
            virtual bool get_opposite_redmarker_left_enabled() const = 0;
            virtual bool get_opposite_redmarker_right_enabled() const = 0;
            virtual bool get_devices_light_enabled() const = 0;
            virtual double get_roof_light_level() const = 0;
            /* Compartment (roof) light, as the vehicle's own signal reports it: lit only while the
             * lighting circuit is fed. Read straight from the backend - nothing is stored. */
            virtual bool get_roof_light_enabled() const = 0;
            enum LightEnd { LIGHT_END_FRONT, LIGHT_END_REAR };
            enum LightType {
                LIGHT_TYPE_HEADLIGHT_UPPER,
                LIGHT_TYPE_HEADLIGHT_LEFT,
                LIGHT_TYPE_HEADLIGHT_RIGHT,
                LIGHT_TYPE_REDMARKER_LEFT,
                LIGHT_TYPE_REDMARKER_RIGHT,
            };
        protected:
            void _register_commands() override;
            void _unregister_commands() override;
        public:
            static const char *selector_position_changed_signal;
            MAKE_MEMBER_GS(int, lights_selector_position, 0);
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
            virtual TypedArray<LightListItem> get_lights_list() = 0;
            virtual void set_lights_list(const TypedArray<LightListItem> &p_list) = 0;
            virtual void increase_light_selector_position() = 0;
            virtual void decrease_light_selector_position() = 0;
            // Direct per-light override, independent of the selector/"light programator"
            // (LightsPos + light_position_list) system above - sets/clears a single bit of
            // iLights directly, for debugging/testing individual bulbs regardless of what the
            // programator would normally compute. p_light matches the short name half of this
            // class's own state keys (state key = "lights/" + p_light + "_enabled"): e.g.
            // "front_headlight_left", "rear_redmarker_right".
            virtual void light(const String &p_light, bool p_enabled) = 0;
            // Cab-relative toggle for the actual MMD cabin switches (upperlight_sw:/leftlight_sw:
            // /rightlight_sw:/leftend_sw:/rightend_sw:/rearupperlight_sw:/rearleftlight_sw:/
            // rearrightlight_sw:/rearleftend_sw:/rearrightend_sw:) - resolves which physical end
            // to toggle from the currently active cab, unlike light() above (a fixed-end direct
            // override for debugging). p_light is the MMD label's own suffix, e.g. "upper",
            // "left", "leftend", "rearupper", "rearleftend".
            virtual void light_switch(const String &p_light, bool p_enabled) = 0;
            // Cab interior lamp ("cablight_sw:") and instrument/dashboard backlighting
            // ("instrumentlight_sw:") - both plain manual toggles with no counterpart in the
            // simulation itself, gated only by 24V/110V power availability (mirrors the
            // original engine's own "cablightlevel"/"lightpower" power gating, vehicle/Train.cpp).
            virtual void roof_light(bool p_enabled) = 0;
            virtual void devices_light(bool p_enabled) = 0;
    };
} // namespace godot
