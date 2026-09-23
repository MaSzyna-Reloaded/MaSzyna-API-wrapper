#include "MoverVehicleLighting.hpp"
#include "../mover/MoverBackend.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void MoverVehicleLighting::_bind_methods() {}


    void MoverVehicleLighting::_apply_configuration() {
        TMoverParameters *p_mover = mover_of(this);
        ASSERT_MOVER(p_mover);
        VehicleComponent::_apply_configuration();
        p_mover->LightsPosNo =
                static_cast<int>(light_position_list.size()); // To fix narrowing conversion from int64_t to int
        p_mover->LightsWrap = get_lights_wrap_selector();
        p_mover->LightsDefPos = get_lights_default_selector_position();
        p_mover->LightPowerSource.SourceType = train_controller_node->power_source_map.at(get_light_source());
        p_mover->AlterLightPowerSource.SourceType =
                train_controller_node->power_source_map.at(get_light_alternative_source());
        p_mover->LightsPos = get_lights_selector_position();
    }

    bool MoverVehicleLighting::_light_enabled(
            const TMoverParameters *p_mover, const LightEnd p_end, const LightType p_type) const {
        const int lights = p_mover->iLights[static_cast<int>(light_end_map.at(p_end))];
        return (lights & light_type_mask_map.at(p_type)) != 0;
    }

    MoverVehicleLighting::LightEnd MoverVehicleLighting::_active_end(const TMoverParameters *p_mover) {
        return p_mover->CabActive < 0 ? LIGHT_END_REAR : LIGHT_END_FRONT;
    }

    MoverVehicleLighting::LightEnd MoverVehicleLighting::_opposite_end(const TMoverParameters *p_mover) {
        return _active_end(p_mover) == LIGHT_END_FRONT ? LIGHT_END_REAR : LIGHT_END_FRONT;
    }

    bool MoverVehicleLighting::_is_powered(const TMoverParameters *p_mover) {
        return p_mover->Power24vIsAvailable || p_mover->Power110vIsAvailable;
    }


    bool MoverVehicleLighting::get_roof_light_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr && roof_light_active && _is_powered(mover);
    }

    int MoverVehicleLighting::get_position() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->LightsPosNo : 0;
    }

    double MoverVehicleLighting::get_power() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->LightPower : 0.0;
    }

    int MoverVehicleLighting::get_power_source() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? train_controller_node->tpower_source_map.at(mover->LightPowerSource.SourceType) : 0;
    }

    bool MoverVehicleLighting::get_front_headlight_upper_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_front_headlight_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_front_headlight_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_front_redmarker_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_front_redmarker_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_rear_headlight_upper_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_rear_headlight_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_rear_headlight_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_rear_redmarker_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_rear_redmarker_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_active_headlight_upper_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_active_headlight_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_active_headlight_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_active_redmarker_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_active_redmarker_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _active_end(mover), LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_opposite_headlight_upper_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_UPPER) : false;
    }

    bool MoverVehicleLighting::get_opposite_headlight_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_LEFT) : false;
    }

    bool MoverVehicleLighting::get_opposite_headlight_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_opposite_redmarker_left_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_REDMARKER_LEFT) : false;
    }

    bool MoverVehicleLighting::get_opposite_redmarker_right_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_REDMARKER_RIGHT) : false;
    }

    bool MoverVehicleLighting::get_devices_light_enabled() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? devices_light_active && _is_powered(mover) : false;
    }

    double MoverVehicleLighting::get_roof_light_level() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? roof_light_active && _is_powered(mover) ? (mover->Power110vIsAvailable ? 1.0 : 0.5) : 0.0 : 0.0;
    }

    void MoverVehicleLighting::_fill_state_dictionary(Dictionary &p_state) const {
        TMoverParameters *mover = mover_of(this);
        if (mover == nullptr) {
            return;
        }
        p_state["light_position"] = get_position();
        p_state["light_power"] = get_power();
        p_state["light_power_source"] = get_power_source();
        p_state["lights/front_headlight_upper_enabled"] = get_front_headlight_upper_enabled();
        p_state["lights/front_headlight_left_enabled"] = get_front_headlight_left_enabled();
        p_state["lights/front_headlight_right_enabled"] = get_front_headlight_right_enabled();
        p_state["lights/front_redmarker_left_enabled"] = get_front_redmarker_left_enabled();
        p_state["lights/front_redmarker_right_enabled"] = get_front_redmarker_right_enabled();
        p_state["lights/rear_headlight_upper_enabled"] = get_rear_headlight_upper_enabled();
        p_state["lights/rear_headlight_left_enabled"] = get_rear_headlight_left_enabled();
        p_state["lights/rear_headlight_right_enabled"] = get_rear_headlight_right_enabled();
        p_state["lights/rear_redmarker_left_enabled"] = get_rear_redmarker_left_enabled();
        p_state["lights/rear_redmarker_right_enabled"] = get_rear_redmarker_right_enabled();
        p_state["lights/active_headlight_upper_enabled"] = get_active_headlight_upper_enabled();
        p_state["lights/active_headlight_left_enabled"] = get_active_headlight_left_enabled();
        p_state["lights/active_headlight_right_enabled"] = get_active_headlight_right_enabled();
        p_state["lights/active_redmarker_left_enabled"] = get_active_redmarker_left_enabled();
        p_state["lights/active_redmarker_right_enabled"] = get_active_redmarker_right_enabled();
        p_state["lights/opposite_headlight_upper_enabled"] = get_opposite_headlight_upper_enabled();
        p_state["lights/opposite_headlight_left_enabled"] = get_opposite_headlight_left_enabled();
        p_state["lights/opposite_headlight_right_enabled"] = get_opposite_headlight_right_enabled();
        p_state["lights/opposite_redmarker_left_enabled"] = get_opposite_redmarker_left_enabled();
        p_state["lights/opposite_redmarker_right_enabled"] = get_opposite_redmarker_right_enabled();
        p_state["roof_light_enabled"] = get_roof_light_enabled();
        p_state["devices_light_enabled"] = get_devices_light_enabled();
        p_state["roof_light_level"] = get_roof_light_level();
    }

    void MoverVehicleLighting::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = mover_of(this);
        if (mover == nullptr) {
            return;
        }
        VehicleComponent::_fill_config_dictionary(p_config);
    }



    void MoverVehicleLighting::increase_light_selector_position() {
        if ((get_lights_selector_position() + 1) < light_position_list.size()) {
            set_lights_selector_position(get_lights_selector_position() + 1);
        }
    }
    void MoverVehicleLighting::decrease_light_selector_position() {
        if ((get_lights_selector_position() + 1) > light_position_list.size()) {
            set_lights_selector_position(get_lights_selector_position() - 1);
        }
    }

    namespace {
        struct LightBit {
                const char *name;
                MoverVehicleLighting::LightEnd end;
                MoverVehicleLighting::LightType type;
        };

        const LightBit LIGHT_BITS[] = {
                {"front_headlight_upper", MoverVehicleLighting::LIGHT_END_FRONT, MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"front_headlight_left", MoverVehicleLighting::LIGHT_END_FRONT, MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"front_headlight_right", MoverVehicleLighting::LIGHT_END_FRONT, MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"front_redmarker_left", MoverVehicleLighting::LIGHT_END_FRONT, MoverVehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"front_redmarker_right", MoverVehicleLighting::LIGHT_END_FRONT, MoverVehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
                {"rear_headlight_upper", MoverVehicleLighting::LIGHT_END_REAR, MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"rear_headlight_left", MoverVehicleLighting::LIGHT_END_REAR, MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"rear_headlight_right", MoverVehicleLighting::LIGHT_END_REAR, MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"rear_redmarker_left", MoverVehicleLighting::LIGHT_END_REAR, MoverVehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rear_redmarker_right", MoverVehicleLighting::LIGHT_END_REAR, MoverVehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
        };
    } // namespace

    void MoverVehicleLighting::light(const String &p_light, const bool p_enabled) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);

        for (const LightBit &bit: LIGHT_BITS) {
            if (p_light == bit.name) {
                const int end = static_cast<int>(light_end_map.at(bit.end));
                const int mask = light_type_mask_map.at(bit.type);
                if (p_enabled) {
                    mover->iLights[end] |= mask;
                } else {
                    mover->iLights[end] &= ~mask;
                }
                return;
            }
        }
        UtilityFunctions::push_warning("MoverVehicleLighting::light() unknown light name: " + p_light);
    }

    namespace {
        struct LightSwitchMask {
                const char *suffix;
                MoverVehicleLighting::LightType type;
        };

        const LightSwitchMask LIGHT_SWITCH_MASKS[] = {
                {"upper", MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"left", MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"right", MoverVehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"leftend", MoverVehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rightend", MoverVehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
        };
    } // namespace

    // Confirmed against vehicle/Train.cpp:5267-5316 (OnCommand_headlighttoggleleft/enableleft) -
    // upperlight_sw:/leftlight_sw:/rightlight_sw:/leftend_sw:/rightend_sw: (p_light without a
    // "rear" prefix) toggle whichever end is the CURRENTLY ACTIVE cab's own front
    // (Train->cab_to_end()); rearupperlight_sw:/etc. (p_light with a "rear" prefix, stripped
    // here) toggle the opposite end - see this class's own _do_fetch_state_from_mover() for the
    // matching active_end/opposite_end state this mirrors. NOTE: a real nuance from the original
    // is deliberately NOT reproduced here - OnCommand_headlightenableleft also clears the
    // matching redmarker when the vehicle has no separate *end_sw: declared (a 3-way switch
    // subsuming the marker light), which would require cross-widget awareness this catalog-driven
    // instancer doesn't have.
    void MoverVehicleLighting::light_switch(const String &p_light, const bool p_enabled) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);

        const bool is_rear = p_light.begins_with("rear");
        const String suffix = is_rear ? p_light.substr(4) : p_light;
        const LightEnd active_end = (mover->CabActive < 0) ? LIGHT_END_REAR : LIGHT_END_FRONT;
        const LightEnd opposite_end = (active_end == LIGHT_END_FRONT) ? LIGHT_END_REAR : LIGHT_END_FRONT;
        const LightEnd target_end = is_rear ? opposite_end : active_end;

        for (const LightSwitchMask &entry: LIGHT_SWITCH_MASKS) {
            if (suffix == entry.suffix) {
                const int end = static_cast<int>(light_end_map.at(target_end));
                const int mask = light_type_mask_map.at(entry.type);
                if (p_enabled) {
                    mover->iLights[end] |= mask;
                } else {
                    mover->iLights[end] &= ~mask;
                }
                return;
            }
        }
        UtilityFunctions::push_warning("MoverVehicleLighting::light_switch() unknown light name: " + p_light);
    }

    void MoverVehicleLighting::roof_light(const bool p_enabled) {
        roof_light_active = p_enabled;
    }

    void MoverVehicleLighting::devices_light(const bool p_enabled) {
        devices_light_active = p_enabled;
    }

} // namespace godot
