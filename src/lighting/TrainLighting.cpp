#include "TrainLighting.hpp"
namespace godot {
    const char *TrainLighting::selector_position_changed_signal = "selector_position_changed";

    void TrainLighting::_bind_methods() {
        BIND_PROPERTY(TrainLighting, Variant::COLOR, head_light_color, "head_light");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, head_light_dimmed_multiplier, "head_light");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, head_light_normal_multiplier, "head_light");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, head_light_high_beam_dimmed_multiplier, "head_light/high_beam");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, head_light_high_beam_normal_multiplier, "head_light/high_beam");
        BIND_PROPERTY(TrainLighting, Variant::INT, lights_default_selector_position, "lights");
        BIND_PROPERTY(TrainLighting, Variant::INT, lights_selector_position, "lights");
        BIND_PROPERTY(TrainLighting, Variant::BOOL, lights_wrap_selector, "lights");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                TrainLighting, Variant::ARRAY, lights_list, "lights", PROPERTY_HINT_TYPE_STRING, "LightListItem");
        BIND_PROPERTY_W_HINT(
                TrainLighting, Variant::INT, light_source, "light", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                TrainLighting, Variant::INT, source_generator_engine, "source/generator", PROPERTY_HINT_ENUM,
                "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                "DieselElectric,Main");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, source_accumulator_max_voltage, "source/accumulator");
        BIND_PROPERTY_W_HINT(
                TrainLighting, Variant::INT, light_alternative_source, "light/alternative", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, light_alternative_max_voltage, "light/alternative");
        BIND_PROPERTY(TrainLighting, Variant::FLOAT, light_alternative_capacity, "light/alternative");
        BIND_PROPERTY_W_HINT(
                TrainLighting, Variant::INT, source_accumulator_recharge_source, "source/accumulator",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(TrainLighting, Variant::INT, instrument_type);
        ClassDB::bind_method(
                D_METHOD("increase_light_selector_position"), &TrainLighting::increase_light_selector_position);
        ClassDB::bind_method(
                D_METHOD("decrease_light_selector_position"), &TrainLighting::decrease_light_selector_position);
        ADD_SIGNAL(MethodInfo(selector_position_changed_signal, PropertyInfo(Variant::INT, "position")));
    }

    void TrainLighting::_do_update_internal_mover(TMoverParameters *p_mover) {
        ASSERT_MOVER(p_mover);
        TrainPart::_do_update_internal_mover(p_mover);
        p_mover->LightsPosNo = static_cast<int>(lights_list.size()); // To fix narrowing conversion from int64_t to int
        p_mover->LightsWrap = lights_wrap_selector;
        p_mover->LightsDefPos = lights_default_selector_position;
        p_mover->LightPower = 0; // LightPower is used there but declared in the Param section in the .fiz file
        p_mover->LightPowerSource.SourceType = train_controller_node->power_source_map.at(light_source);
        p_mover->AlterLightPowerSource.SourceType =
                train_controller_node->power_source_map.at(light_alternative_source);
        p_mover->LightsPos = lights_selector_position;
    }

    void TrainLighting::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        ASSERT_MOVER(p_mover);
        p_state["light_position"] = p_mover->LightsPosNo;
        p_state["light_power"] = p_mover->LightPower;
        p_state["power_source"] = train_controller_node->tpower_source_map.at(p_mover->LightPowerSource.SourceType);
    }

    void TrainLighting::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        TrainPart::_do_fetch_config_from_mover(p_mover, p_config);
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
        if ((lights_selector_position + 1) < lights_list.size()) {
            lights_selector_position++;
        }
    }
    void TrainLighting::decrease_light_selector_position() {
        if ((lights_selector_position + 1) > lights_list.size()) {
            lights_selector_position--;
        }
    }

} // namespace godot
