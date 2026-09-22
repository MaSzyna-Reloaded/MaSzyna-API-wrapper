#include "VehicleLighting.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
namespace godot {
    const char *VehicleLighting::selector_position_changed_signal = "selector_position_changed";

    void VehicleLighting::_bind_methods() {
        BIND_PROPERTY(VehicleLighting, Variant::COLOR, head_light_color, "head_light");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_dimmed_multiplier, "head_light");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_normal_multiplier, "head_light");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_high_beam_dimmed_multiplier, "head_light/high_beam");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, head_light_high_beam_normal_multiplier, "head_light/high_beam");
        BIND_PROPERTY(VehicleLighting, Variant::INT, lights_default_selector_position, "lights");
        BIND_PROPERTY(VehicleLighting, Variant::INT, lights_selector_position, "lights");
        BIND_PROPERTY(VehicleLighting, Variant::BOOL, lights_wrap_selector, "lights");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleLighting, Variant::ARRAY, lights_list, "lights", PROPERTY_HINT_TYPE_STRING, "LightListItem");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, light_source, "light", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, source_generator_engine, "source/generator", PROPERTY_HINT_ENUM,
                "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                "DieselElectric,Main");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, source_accumulator_max_voltage, "source/accumulator");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, light_alternative_source, "light/alternative", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, light_alternative_max_voltage, "light/alternative");
        BIND_PROPERTY(VehicleLighting, Variant::FLOAT, light_alternative_capacity, "light/alternative");
        BIND_PROPERTY_W_HINT(
                VehicleLighting, Variant::INT, source_accumulator_recharge_source, "source/accumulator",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(VehicleLighting, Variant::INT, instrument_type);
        ClassDB::bind_method(
                D_METHOD("increase_light_selector_position"), &VehicleLighting::increase_light_selector_position);
        ClassDB::bind_method(
                D_METHOD("decrease_light_selector_position"), &VehicleLighting::decrease_light_selector_position);
        ClassDB::bind_method(D_METHOD("light", "light", "enabled"), &VehicleLighting::light);
        ClassDB::bind_method(D_METHOD("light_switch", "light", "enabled"), &VehicleLighting::light_switch);
        ClassDB::bind_method(D_METHOD("roof_light", "enabled"), &VehicleLighting::roof_light);
        ClassDB::bind_method(D_METHOD("devices_light", "enabled"), &VehicleLighting::devices_light);
        ADD_SIGNAL(MethodInfo(selector_position_changed_signal, PropertyInfo(Variant::INT, "position")));
    }

    void VehicleLighting::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        VehicleComponent::_do_update_internal_mover(p_mover);
        p_mover->LightsPosNo =
                static_cast<int>(light_position_list.size()); // To fix narrowing conversion from int64_t to int
        p_mover->LightsWrap = lights_wrap_selector;
        p_mover->LightsDefPos = lights_default_selector_position;
        p_mover->LightPowerSource.SourceType = train_controller_node->power_source_map.at(light_source);
        p_mover->AlterLightPowerSource.SourceType =
                train_controller_node->power_source_map.at(light_alternative_source);
        p_mover->LightsPos = lights_selector_position;
    }

    bool VehicleLighting::_light_enabled(
            const TMoverParameters *p_mover, const LightEnd p_end, const LightType p_type) const {
        const int lights = p_mover->iLights[static_cast<int>(light_end_map.at(p_end))];
        return (lights & light_type_mask_map.at(p_type)) != 0;
    }

    VehicleLighting::LightEnd VehicleLighting::_active_end(const TMoverParameters *p_mover) {
        return p_mover->CabActive < 0 ? LIGHT_END_REAR : LIGHT_END_FRONT;
    }

    VehicleLighting::LightEnd VehicleLighting::_opposite_end(const TMoverParameters *p_mover) {
        return _active_end(p_mover) == LIGHT_END_FRONT ? LIGHT_END_REAR : LIGHT_END_FRONT;
    }

    bool VehicleLighting::_is_powered(const TMoverParameters *p_mover) {
        return p_mover->Power24vIsAvailable || p_mover->Power110vIsAvailable;
    }

    void VehicleLighting::_declare_state_properties() {
        state_base_index = get_state_property_count();
        declare_state_property("light_position", Variant::INT);
        declare_state_property("light_power", Variant::FLOAT);
        declare_state_property("light_power_source", Variant::INT);
        declare_state_property("lights/front_headlight_upper_enabled", Variant::BOOL);
        declare_state_property("lights/front_headlight_left_enabled", Variant::BOOL);
        declare_state_property("lights/front_headlight_right_enabled", Variant::BOOL);
        declare_state_property("lights/front_redmarker_left_enabled", Variant::BOOL);
        declare_state_property("lights/front_redmarker_right_enabled", Variant::BOOL);
        declare_state_property("lights/rear_headlight_upper_enabled", Variant::BOOL);
        declare_state_property("lights/rear_headlight_left_enabled", Variant::BOOL);
        declare_state_property("lights/rear_headlight_right_enabled", Variant::BOOL);
        declare_state_property("lights/rear_redmarker_left_enabled", Variant::BOOL);
        declare_state_property("lights/rear_redmarker_right_enabled", Variant::BOOL);
        declare_state_property("lights/active_headlight_upper_enabled", Variant::BOOL);
        declare_state_property("lights/active_headlight_left_enabled", Variant::BOOL);
        declare_state_property("lights/active_headlight_right_enabled", Variant::BOOL);
        declare_state_property("lights/active_redmarker_left_enabled", Variant::BOOL);
        declare_state_property("lights/active_redmarker_right_enabled", Variant::BOOL);
        declare_state_property("lights/opposite_headlight_upper_enabled", Variant::BOOL);
        declare_state_property("lights/opposite_headlight_left_enabled", Variant::BOOL);
        declare_state_property("lights/opposite_headlight_right_enabled", Variant::BOOL);
        declare_state_property("lights/opposite_redmarker_left_enabled", Variant::BOOL);
        declare_state_property("lights/opposite_redmarker_right_enabled", Variant::BOOL);
        declare_state_property("roof_light_enabled", Variant::BOOL);
        declare_state_property("devices_light_enabled", Variant::BOOL);
        declare_state_property("roof_light_level", Variant::FLOAT);
    }

    Variant VehicleLighting::_get_state_property(const int p_local_index) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        switch (p_local_index - state_base_index) {
            case STATE_LIGHT_POSITION:
                return mover->LightsPosNo;
            case STATE_LIGHT_POWER:
                return mover->LightPower;
            case STATE_LIGHT_POWER_SOURCE:
                return train_controller_node->tpower_source_map.at(mover->LightPowerSource.SourceType);
            case STATE_FRONT_HEADLIGHT_UPPER:
                return _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_UPPER);
            case STATE_FRONT_HEADLIGHT_LEFT:
                return _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_LEFT);
            case STATE_FRONT_HEADLIGHT_RIGHT:
                return _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_HEADLIGHT_RIGHT);
            case STATE_FRONT_REDMARKER_LEFT:
                return _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_REDMARKER_LEFT);
            case STATE_FRONT_REDMARKER_RIGHT:
                return _light_enabled(mover, LIGHT_END_FRONT, LIGHT_TYPE_REDMARKER_RIGHT);
            case STATE_REAR_HEADLIGHT_UPPER:
                return _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_UPPER);
            case STATE_REAR_HEADLIGHT_LEFT:
                return _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_LEFT);
            case STATE_REAR_HEADLIGHT_RIGHT:
                return _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_HEADLIGHT_RIGHT);
            case STATE_REAR_REDMARKER_LEFT:
                return _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_REDMARKER_LEFT);
            case STATE_REAR_REDMARKER_RIGHT:
                return _light_enabled(mover, LIGHT_END_REAR, LIGHT_TYPE_REDMARKER_RIGHT);
            case STATE_ACTIVE_HEADLIGHT_UPPER:
                return _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_UPPER);
            case STATE_ACTIVE_HEADLIGHT_LEFT:
                return _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_LEFT);
            case STATE_ACTIVE_HEADLIGHT_RIGHT:
                return _light_enabled(mover, _active_end(mover), LIGHT_TYPE_HEADLIGHT_RIGHT);
            case STATE_ACTIVE_REDMARKER_LEFT:
                return _light_enabled(mover, _active_end(mover), LIGHT_TYPE_REDMARKER_LEFT);
            case STATE_ACTIVE_REDMARKER_RIGHT:
                return _light_enabled(mover, _active_end(mover), LIGHT_TYPE_REDMARKER_RIGHT);
            case STATE_OPPOSITE_HEADLIGHT_UPPER:
                return _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_UPPER);
            case STATE_OPPOSITE_HEADLIGHT_LEFT:
                return _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_LEFT);
            case STATE_OPPOSITE_HEADLIGHT_RIGHT:
                return _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_HEADLIGHT_RIGHT);
            case STATE_OPPOSITE_REDMARKER_LEFT:
                return _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_REDMARKER_LEFT);
            case STATE_OPPOSITE_REDMARKER_RIGHT:
                return _light_enabled(mover, _opposite_end(mover), LIGHT_TYPE_REDMARKER_RIGHT);
            case STATE_ROOF_LIGHT_ENABLED:
                return roof_light_active && _is_powered(mover);
            case STATE_DEVICES_LIGHT_ENABLED:
                return devices_light_active && _is_powered(mover);
            case STATE_ROOF_LIGHT_LEVEL:
                return roof_light_active && _is_powered(mover) ? (mover->Power110vIsAvailable ? 1.0 : 0.5) : 0.0;
            default:
                return Variant();
        }
    }

    void VehicleLighting::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        VehicleComponent::_do_fetch_config_from_mover(p_mover, p_config);
    }

    void VehicleLighting::_register_commands() {
        register_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        register_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        register_command("light", Callable(this, "light"));
        register_command("light_switch", Callable(this, "light_switch"));
        register_command("roof_light", Callable(this, "roof_light"));
        register_command("devices_light", Callable(this, "devices_light"));
        VehicleComponent::_register_commands();
    }

    void VehicleLighting::_unregister_commands() {
        unregister_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        unregister_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        unregister_command("light", Callable(this, "light"));
        unregister_command("light_switch", Callable(this, "light_switch"));
        unregister_command("roof_light", Callable(this, "roof_light"));
        unregister_command("devices_light", Callable(this, "devices_light"));
        VehicleComponent::_unregister_commands();
    }

    void VehicleLighting::increase_light_selector_position() {
        if ((lights_selector_position + 1) < light_position_list.size()) {
            lights_selector_position++;
        }
    }
    void VehicleLighting::decrease_light_selector_position() {
        if ((lights_selector_position + 1) > light_position_list.size()) {
            lights_selector_position--;
        }
    }

    namespace {
        struct LightBit {
                const char *name;
                VehicleLighting::LightEnd end;
                VehicleLighting::LightType type;
        };

        const LightBit LIGHT_BITS[] = {
                {"front_headlight_upper", VehicleLighting::LIGHT_END_FRONT, VehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"front_headlight_left", VehicleLighting::LIGHT_END_FRONT, VehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"front_headlight_right", VehicleLighting::LIGHT_END_FRONT, VehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"front_redmarker_left", VehicleLighting::LIGHT_END_FRONT, VehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"front_redmarker_right", VehicleLighting::LIGHT_END_FRONT, VehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
                {"rear_headlight_upper", VehicleLighting::LIGHT_END_REAR, VehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"rear_headlight_left", VehicleLighting::LIGHT_END_REAR, VehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"rear_headlight_right", VehicleLighting::LIGHT_END_REAR, VehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"rear_redmarker_left", VehicleLighting::LIGHT_END_REAR, VehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rear_redmarker_right", VehicleLighting::LIGHT_END_REAR, VehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
        };
    } // namespace

    void VehicleLighting::light(const String &p_light, const bool p_enabled) {
        TMoverParameters *mover = get_mover();
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
        UtilityFunctions::push_warning("VehicleLighting::light() unknown light name: " + p_light);
    }

    namespace {
        struct LightSwitchMask {
                const char *suffix;
                VehicleLighting::LightType type;
        };

        const LightSwitchMask LIGHT_SWITCH_MASKS[] = {
                {"upper", VehicleLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"left", VehicleLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"right", VehicleLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"leftend", VehicleLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rightend", VehicleLighting::LIGHT_TYPE_REDMARKER_RIGHT},
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
    void VehicleLighting::light_switch(const String &p_light, const bool p_enabled) {
        TMoverParameters *mover = get_mover();
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
        UtilityFunctions::push_warning("VehicleLighting::light_switch() unknown light name: " + p_light);
    }

    void VehicleLighting::roof_light(const bool p_enabled) {
        roof_light_active = p_enabled;
    }

    void VehicleLighting::devices_light(const bool p_enabled) {
        devices_light_active = p_enabled;
    }

} // namespace godot
