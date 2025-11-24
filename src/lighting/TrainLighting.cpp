#include "TrainLighting.hpp"
namespace godot {
    TrainLighting::TrainLighting() {
        set_capability("lighting");
    }
    const char *TrainLighting::SELECTOR_POSITION_CHANGED_SIGNAL = "selector_position_changed";

    void TrainLighting::_bind_methods() {BIND_PROPERTY(Variant::COLOR, "head_light_color", "head_light/color", &TrainLighting::set_head_light_color, &TrainLighting::get_head_light_color, "color");
    BIND_PROPERTY(Variant::FLOAT, "head_light_dimmed_multiplier", "head_light/dimmed_multiplier", &TrainLighting::set_dimming_multiplier, &TrainLighting::get_dimming_multiplier, "multiplier");
    BIND_PROPERTY(Variant::FLOAT, "head_light_normal_multiplier", "head_light/normal_multiplier", &TrainLighting::set_normal_multiplier, &TrainLighting::get_normal_multiplier, "multiplier");
    BIND_PROPERTY(Variant::FLOAT, "high_beam_dimmed_multiplier", "head_light/high_beam/dimmed_multiplier", &TrainLighting::set_high_beam_dimmed_multiplier, &TrainLighting::get_high_beam_dimmed_multiplier, "multiplier");
    BIND_PROPERTY(Variant::FLOAT, "high_beam_normal_multiplier", "head_light/high_beam/normal_multiplier", &TrainLighting::set_high_beam_multiplier, &TrainLighting::get_high_beam_multiplier, "multiplier");
    BIND_PROPERTY(Variant::INT, "lights_default_selector_position", "lights/default_selector_position", &TrainLighting::set_default_selector_position, &TrainLighting::get_default_selector_position, "default_selector_position");
    BIND_PROPERTY(Variant::INT, "lights_selector_position", "lights/selector_position", &TrainLighting::set_selector_position, &TrainLighting::get_selector_position, "selector_position");
    BIND_PROPERTY(Variant::BOOL, "wrap_light_selector", "lights/wrap_selector", &TrainLighting::set_wrap_light_selector, &TrainLighting::get_wrap_light_selector, "wrap_selector");
    BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "light_position_list", "lights/list", &TrainLighting::set_light_position_list, &TrainLighting::get_light_position_list, "light_position_list", PROPERTY_HINT_TYPE_STRING, "LighListItem");
    BIND_PROPERTY_W_HINT(Variant::INT, "light_source", "light/source", &TrainLighting::set_light_source, &TrainLighting::get_light_source, "source", PROPERTY_HINT_ENUM, "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
    BIND_PROPERTY_W_HINT(Variant::INT, "generator_engine", "source/generator/engine", &TrainLighting::set_generator_engine, &TrainLighting::get_generator_engine, "generator_engine", PROPERTY_HINT_ENUM, "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,DieselElectric,Main");
    BIND_PROPERTY(Variant::FLOAT, "max_accumulator_voltage", "source/accumulator/max_voltage", &TrainLighting::set_max_accumulator_voltage, &TrainLighting::get_max_accumulator_voltage, "max_voltage");
    BIND_PROPERTY_W_HINT(Variant::INT, "alternative_light_source", "light/alternative/source", &TrainLighting::set_alternative_light_source, &TrainLighting::get_alternative_light_source, "source", PROPERTY_HINT_ENUM, "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
    BIND_PROPERTY(Variant::FLOAT, "alternative_max_voltage", "light/alternative/max_voltage", &TrainLighting::set_alternative_max_voltage, &TrainLighting::get_alternative_max_voltage, "max_voltage");
    BIND_PROPERTY(Variant::FLOAT, "alternative_light_capacity", "light/alternative/capacity", &TrainLighting::set_alternative_light_capacity, &TrainLighting::get_alternative_light_capacity, "capacity");
    BIND_PROPERTY_W_HINT(Variant::INT, "accumulator_recharge_source", "source/accumulator/recharge_source", &TrainLighting::set_accumulator_recharge_source, &TrainLighting::get_accumulator_recharge_source, "recharge_source", PROPERTY_HINT_ENUM, "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
    BIND_PROPERTY(Variant::INT, "instrument_type", "instrument_type", &TrainLighting::set_instrument_light_type, &TrainLighting::get_instrument_light_type, "instrument_type");
        ClassDB::bind_method(D_METHOD("increase_light_selector_position"), &TrainLighting::increase_light_selector_position);
        ClassDB::bind_method(D_METHOD("decrease_light_selector_position"), &TrainLighting::decrease_light_selector_position);
        ADD_SIGNAL(MethodInfo(SELECTOR_POSITION_CHANGED_SIGNAL, PropertyInfo(Variant::INT, "position")));
    }

    void TrainLighting::_do_update_internal_mover(TMoverParameters *mover) {
        ASSERT_MOVER(mover);
        TrainPart::_do_update_internal_mover(mover);
        mover->LightsPosNo = static_cast<int>(light_position_list.size()); // To fix narrowing conversion from int64_t to int
        mover->LightsWrap = wrap_light_selector;
        mover->LightsDefPos = default_selector_position;
        mover->LightPower = 0; //LightPower is used there but declared in the Param section in the .fiz file
        mover->LightPowerSource.SourceType = _controller->power_source_map.at(light_source);
        mover->AlterLightPowerSource.SourceType = _controller->power_source_map.at(alternative_light_source);
        mover->LightsPos = selector_position;
    }

    void TrainLighting::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        ASSERT_MOVER(mover);
        state["light_position"] = mover->LightsPosNo;
        state["light_power"] = mover->LightPower;
        state["power_source"] = _controller->tpower_source_map.at(mover->LightPowerSource.SourceType);
    }

    void TrainLighting::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        TrainPart::_do_fetch_config_from_mover(mover, config);
    }

    void TrainLighting::_register_commands() {
        register_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        register_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
        TrainPart::_register_commands();
    }

    void TrainLighting::_unregister_commands() {
        unregister_command("increase_light_selector_position", Callable(this, "increase_light_selector_position"));
        unregister_command("decrease_light_selector_position", Callable(this, "decrease_light_selector_position"));
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
        }}

} // namespace godot
