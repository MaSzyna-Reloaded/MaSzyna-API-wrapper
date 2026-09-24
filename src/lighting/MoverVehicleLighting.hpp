#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleLighting.hpp"

namespace godot {
    /* VehicleLighting on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleLighting : public VehicleLighting, public MoverComponent {
            GDCLASS(MoverVehicleLighting, VehicleLighting);

        private:
            static void _bind_methods();
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            int get_position() const override;
            double get_power() const override;
            int get_power_source() const override;
            bool get_front_headlight_upper_enabled() const override;
            bool get_front_headlight_left_enabled() const override;
            bool get_front_headlight_right_enabled() const override;
            bool get_front_redmarker_left_enabled() const override;
            bool get_front_redmarker_right_enabled() const override;
            bool get_rear_headlight_upper_enabled() const override;
            bool get_rear_headlight_left_enabled() const override;
            bool get_rear_headlight_right_enabled() const override;
            bool get_rear_redmarker_left_enabled() const override;
            bool get_rear_redmarker_right_enabled() const override;
            bool get_active_headlight_upper_enabled() const override;
            bool get_active_headlight_left_enabled() const override;
            bool get_active_headlight_right_enabled() const override;
            bool get_active_redmarker_left_enabled() const override;
            bool get_active_redmarker_right_enabled() const override;
            bool get_opposite_headlight_upper_enabled() const override;
            bool get_opposite_headlight_left_enabled() const override;
            bool get_opposite_headlight_right_enabled() const override;
            bool get_opposite_redmarker_left_enabled() const override;
            bool get_opposite_redmarker_right_enabled() const override;
            bool get_devices_light_enabled() const override;
            double get_roof_light_level() const override;
            bool get_roof_light_enabled() const override;
        private:
            TypedArray<LightListItem> light_position_list;
            bool roof_light_active = false;
            bool devices_light_active = false;
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
            bool _light_enabled(const TMoverParameters *p_mover, LightEnd p_end, LightType p_type) const;
            static LightEnd _active_end(const TMoverParameters *p_mover);
            static LightEnd _opposite_end(const TMoverParameters *p_mover);
            static bool _is_powered(const TMoverParameters *p_mover);
        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
        public:
            TypedArray<LightListItem> get_lights_list() override {
                return light_position_list;
            };
            void set_lights_list(const TypedArray<LightListItem> &p_list) override {
                light_position_list.clear();
                light_position_list.append_array(p_list);
            };
            void increase_light_selector_position() override;
            void decrease_light_selector_position() override;
            void light(const String &p_light, bool p_enabled) override;
            void light_switch(const String &p_light, bool p_enabled) override;
            void roof_light(bool p_enabled) override;
            void devices_light(bool p_enabled) override;
    };
} // namespace godot
