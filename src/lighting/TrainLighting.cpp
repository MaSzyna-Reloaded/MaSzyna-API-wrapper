#include "TrainLighting.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
namespace godot {
    const char *TrainLighting::selector_position_changed_signal = "selector_position_changed";

    void TrainLighting::_bind_methods() {
        BIND_PROPERTY(
                Variant::COLOR, "head_light_color", "head_light/color", &TrainLighting::set_head_light_color,
                &TrainLighting::get_head_light_color, "color");
        BIND_PROPERTY(
                Variant::FLOAT, "head_light_dimmed_multiplier", "head_light/dimmed_multiplier",
                &TrainLighting::set_dimming_multiplier, &TrainLighting::get_dimming_multiplier, "multiplier");
        BIND_PROPERTY(
                Variant::FLOAT, "head_light_normal_multiplier", "head_light/normal_multiplier",
                &TrainLighting::set_normal_multiplier, &TrainLighting::get_normal_multiplier, "multiplier");
        BIND_PROPERTY(
                Variant::FLOAT, "high_beam_dimmed_multiplier", "head_light/high_beam/dimmed_multiplier",
                &TrainLighting::set_high_beam_dimmed_multiplier, &TrainLighting::get_high_beam_dimmed_multiplier,
                "multiplier");
        BIND_PROPERTY(
                Variant::FLOAT, "high_beam_normal_multiplier", "head_light/high_beam/normal_multiplier",
                &TrainLighting::set_high_beam_multiplier, &TrainLighting::get_high_beam_multiplier, "multiplier");
        BIND_PROPERTY(
                Variant::INT, "lights_default_selector_position", "lights/default_selector_position",
                &TrainLighting::set_default_selector_position, &TrainLighting::get_default_selector_position,
                "default_selector_position");
        BIND_PROPERTY(
                Variant::INT, "lights_selector_position", "lights/selector_position",
                &TrainLighting::set_selector_position, &TrainLighting::get_selector_position, "selector_position");
        BIND_PROPERTY(
                Variant::BOOL, "wrap_light_selector", "lights/wrap_selector", &TrainLighting::set_wrap_light_selector,
                &TrainLighting::get_wrap_light_selector, "wrap_selector");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                Variant::ARRAY, "light_position_list", "lights/list", &TrainLighting::set_light_position_list,
                &TrainLighting::get_light_position_list, "light_position_list", PROPERTY_HINT_TYPE_STRING,
                "LightListItem");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "light_source", "light/source", &TrainLighting::set_light_source,
                &TrainLighting::get_light_source, "source", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "generator_engine", "source/generator/engine", &TrainLighting::set_generator_engine,
                &TrainLighting::get_generator_engine, "generator_engine", PROPERTY_HINT_ENUM,
                "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                "DieselElectric,Main");
        BIND_PROPERTY(
                Variant::FLOAT, "max_accumulator_voltage", "source/accumulator/max_voltage",
                &TrainLighting::set_max_accumulator_voltage, &TrainLighting::get_max_accumulator_voltage,
                "max_voltage");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "alternative_light_source", "light/alternative/source",
                &TrainLighting::set_alternative_light_source, &TrainLighting::get_alternative_light_source, "source",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(
                Variant::FLOAT, "alternative_max_voltage", "light/alternative/max_voltage",
                &TrainLighting::set_alternative_max_voltage, &TrainLighting::get_alternative_max_voltage,
                "max_voltage");
        BIND_PROPERTY(
                Variant::FLOAT, "alternative_light_capacity", "light/alternative/capacity",
                &TrainLighting::set_alternative_light_capacity, &TrainLighting::get_alternative_light_capacity,
                "capacity");
        BIND_PROPERTY_W_HINT(
                Variant::INT, "accumulator_recharge_source", "source/accumulator/recharge_source",
                &TrainLighting::set_accumulator_recharge_source, &TrainLighting::get_accumulator_recharge_source,
                "recharge_source", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(
                Variant::INT, "instrument_type", "instrument_type", &TrainLighting::set_instrument_light_type,
                &TrainLighting::get_instrument_light_type, "instrument_type");
        ClassDB::bind_method(
                D_METHOD("increase_light_selector_position"), &TrainLighting::increase_light_selector_position);
        ClassDB::bind_method(
                D_METHOD("decrease_light_selector_position"), &TrainLighting::decrease_light_selector_position);
        ClassDB::bind_method(D_METHOD("light", "light", "enabled"), &TrainLighting::light);
        ClassDB::bind_method(D_METHOD("light_switch", "light", "enabled"), &TrainLighting::light_switch);
        ClassDB::bind_method(D_METHOD("roof_light", "enabled"), &TrainLighting::roof_light);
        ClassDB::bind_method(D_METHOD("devices_light", "enabled"), &TrainLighting::devices_light);
        ADD_SIGNAL(MethodInfo(selector_position_changed_signal, PropertyInfo(Variant::INT, "position")));
    }

    void TrainLighting::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        TrainPart::_do_update_internal_mover(p_mover);
        p_mover->LightsPosNo =
                static_cast<int>(light_position_list.size()); // To fix narrowing conversion from int64_t to int
        p_mover->LightsWrap = wrap_light_selector;
        p_mover->LightsDefPos = default_selector_position;
        p_mover->LightPowerSource.SourceType = train_controller_node->power_source_map.at(light_source);
        p_mover->AlterLightPowerSource.SourceType =
                train_controller_node->power_source_map.at(alternative_light_source);
        p_mover->LightsPos = selector_position;
    }

    void TrainLighting::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        ASSERT_MOVER(p_mover);
        p_state["light_position"] = p_mover->LightsPosNo;
        p_state["light_power"] = p_mover->LightPower;
        p_state["power_source"] = train_controller_node->tpower_source_map.at(p_mover->LightPowerSource.SourceType);

        // Confirmed against vehicle/Train.cpp:9199-9208 - the i-upperlight/i-leftlight/etc.
        // indicator lamps read the mover's own already-resolved per-end light bitmask
        // (iLights[front]/iLights[rear]) directly, not the raw selector position or
        // LightListItem table - iLights is the final, live "which bulbs are actually lit"
        // result (accounts for light_power/selector position/etc. already).
        const int front_lights = p_mover->iLights[static_cast<int>(light_end_map.at(LIGHT_END_FRONT))];
        const int rear_lights = p_mover->iLights[static_cast<int>(light_end_map.at(LIGHT_END_REAR))];
        p_state["lights/front_headlight_upper_enabled"] =
                (front_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_UPPER)) != 0;
        p_state["lights/front_headlight_left_enabled"] =
                (front_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_LEFT)) != 0;
        p_state["lights/front_headlight_right_enabled"] =
                (front_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_RIGHT)) != 0;
        p_state["lights/front_redmarker_left_enabled"] =
                (front_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_LEFT)) != 0;
        p_state["lights/front_redmarker_right_enabled"] =
                (front_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_RIGHT)) != 0;
        p_state["lights/rear_headlight_upper_enabled"] =
                (rear_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_UPPER)) != 0;
        p_state["lights/rear_headlight_left_enabled"] =
                (rear_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_LEFT)) != 0;
        p_state["lights/rear_headlight_right_enabled"] =
                (rear_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_RIGHT)) != 0;
        p_state["lights/rear_redmarker_left_enabled"] =
                (rear_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_LEFT)) != 0;
        p_state["lights/rear_redmarker_right_enabled"] =
                (rear_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_RIGHT)) != 0;

        // Confirmed against vehicle/Train.h:220-227 (TTrain::cab_to_end(): `iCabn == 2 ? end::rear
        // : end::front`) and vehicle/Train.cpp:5267-5316 (OnCommand_headlighttoggleleft/enableleft
        // etc. all resolve `Train->cab_to_end()` before touching iLights) - the upperlight_sw/
        // leftlight_sw/etc. cabin switches (no "rear" prefix) toggle whichever physical end
        // corresponds to the CURRENTLY ACTIVE cab, not always the physical front - the
        // rearupperlight_sw/etc. switches toggle the opposite end. "active"/"opposite" here are
        // therefore cab-relative, unlike the i-upperlight/etc. indicator state above (which is
        // always the fixed physical front/rear, since a cab's own dashboard lamp doesn't move).
        // p_mover->CabActive (int, -1/0/1) mirrors iCabn's own front/rear meaning.
        const LightEnd active_end = (p_mover->CabActive < 0) ? LIGHT_END_REAR : LIGHT_END_FRONT;
        const LightEnd opposite_end = (active_end == LIGHT_END_FRONT) ? LIGHT_END_REAR : LIGHT_END_FRONT;
        const int active_lights = p_mover->iLights[static_cast<int>(light_end_map.at(active_end))];
        const int opposite_lights = p_mover->iLights[static_cast<int>(light_end_map.at(opposite_end))];
        p_state["lights/active_headlight_upper_enabled"] =
                (active_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_UPPER)) != 0;
        p_state["lights/active_headlight_left_enabled"] =
                (active_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_LEFT)) != 0;
        p_state["lights/active_headlight_right_enabled"] =
                (active_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_RIGHT)) != 0;
        p_state["lights/active_redmarker_left_enabled"] =
                (active_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_LEFT)) != 0;
        p_state["lights/active_redmarker_right_enabled"] =
                (active_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_RIGHT)) != 0;
        p_state["lights/opposite_headlight_upper_enabled"] =
                (opposite_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_UPPER)) != 0;
        p_state["lights/opposite_headlight_left_enabled"] =
                (opposite_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_LEFT)) != 0;
        p_state["lights/opposite_headlight_right_enabled"] =
                (opposite_lights & light_type_mask_map.at(LIGHT_TYPE_HEADLIGHT_RIGHT)) != 0;
        p_state["lights/opposite_redmarker_left_enabled"] =
                (opposite_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_LEFT)) != 0;
        p_state["lights/opposite_redmarker_right_enabled"] =
                (opposite_lights & light_type_mask_map.at(LIGHT_TYPE_REDMARKER_RIGHT)) != 0;

        // Cab interior lamp / instrument backlighting have no counterpart on the wrapped mover -
        // gated only by 24V/110V power availability, matching the original engine's own
        // "cablightlevel"/"lightpower" power gating (vehicle/Train.cpp).
        const bool is_powered = p_mover->Power24vIsAvailable || p_mover->Power110vIsAvailable;
        p_state["roof_light_enabled"] = roof_light_active && is_powered;
        p_state["devices_light_enabled"] = devices_light_active && is_powered;
    }

    void TrainLighting::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        TrainPart::_do_fetch_config_from_mover(p_mover, p_config);
    }

    void TrainLighting::_register_commands() {
        register_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        register_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        register_command("light", Callable(this, "light"));
        register_command("light_switch", Callable(this, "light_switch"));
        register_command("roof_light", Callable(this, "roof_light"));
        register_command("devices_light", Callable(this, "devices_light"));
        TrainPart::_register_commands();
    }

    void TrainLighting::_unregister_commands() {
        unregister_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        unregister_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        unregister_command("light", Callable(this, "light"));
        unregister_command("light_switch", Callable(this, "light_switch"));
        unregister_command("roof_light", Callable(this, "roof_light"));
        unregister_command("devices_light", Callable(this, "devices_light"));
        TrainPart::_unregister_commands();
    }

    void TrainLighting::increase_light_selector_position() {
        if ((selector_position + 1) < light_position_list.size()) {
            selector_position++;
        }
    }
    void TrainLighting::decrease_light_selector_position() {
        if ((selector_position + 1) > light_position_list.size()) {
            selector_position--;
        }
    }

    namespace {
        struct LightBit {
                const char *name;
                TrainLighting::LightEnd end;
                TrainLighting::LightType type;
        };

        const LightBit LIGHT_BITS[] = {
                {"front_headlight_upper", TrainLighting::LIGHT_END_FRONT, TrainLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"front_headlight_left", TrainLighting::LIGHT_END_FRONT, TrainLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"front_headlight_right", TrainLighting::LIGHT_END_FRONT, TrainLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"front_redmarker_left", TrainLighting::LIGHT_END_FRONT, TrainLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"front_redmarker_right", TrainLighting::LIGHT_END_FRONT, TrainLighting::LIGHT_TYPE_REDMARKER_RIGHT},
                {"rear_headlight_upper", TrainLighting::LIGHT_END_REAR, TrainLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"rear_headlight_left", TrainLighting::LIGHT_END_REAR, TrainLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"rear_headlight_right", TrainLighting::LIGHT_END_REAR, TrainLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"rear_redmarker_left", TrainLighting::LIGHT_END_REAR, TrainLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rear_redmarker_right", TrainLighting::LIGHT_END_REAR, TrainLighting::LIGHT_TYPE_REDMARKER_RIGHT},
        };
    } // namespace

    void TrainLighting::light(const String &p_light, const bool p_enabled) {
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
        UtilityFunctions::push_warning("TrainLighting::light() unknown light name: " + p_light);
    }

    namespace {
        struct LightSwitchMask {
                const char *suffix;
                TrainLighting::LightType type;
        };

        const LightSwitchMask LIGHT_SWITCH_MASKS[] = {
                {"upper", TrainLighting::LIGHT_TYPE_HEADLIGHT_UPPER},
                {"left", TrainLighting::LIGHT_TYPE_HEADLIGHT_LEFT},
                {"right", TrainLighting::LIGHT_TYPE_HEADLIGHT_RIGHT},
                {"leftend", TrainLighting::LIGHT_TYPE_REDMARKER_LEFT},
                {"rightend", TrainLighting::LIGHT_TYPE_REDMARKER_RIGHT},
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
    void TrainLighting::light_switch(const String &p_light, const bool p_enabled) {
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
        UtilityFunctions::push_warning("TrainLighting::light_switch() unknown light name: " + p_light);
    }

    void TrainLighting::roof_light(const bool p_enabled) {
        roof_light_active = p_enabled;
    }

    void TrainLighting::devices_light(const bool p_enabled) {
        devices_light_active = p_enabled;
    }

} // namespace godot
