#include "TrainLighting.hpp"

namespace godot {
    const char *TrainLighting::SELECTOR_POSITION_CHANGED_SIGNAL = "selector_position_changed";

    void TrainLighting::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_head_light_color", "color"), &TrainLighting::set_head_light_color);
        ClassDB::bind_method(D_METHOD("get_head_light_color"), &TrainLighting::get_head_light_color);
        ADD_PROPERTY(PropertyInfo(Variant::COLOR, "head_light/color"), "set_head_light_color", "get_head_light_color");
        ClassDB::bind_method(D_METHOD("set_head_light_dimming_multiplier", "multiplier"), &TrainLighting::set_dimming_multiplier);
        ClassDB::bind_method(D_METHOD("get_head_light_dimming_multiplier"), &TrainLighting::get_dimming_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/dimming_multiplier"), "set_head_light_dimming_multiplier", "get_head_light_dimming_multiplier");
        ClassDB::bind_method(D_METHOD("set_head_light_normal_multiplier", "multiplier"), &TrainLighting::set_normal_multiplier);
        ClassDB::bind_method(D_METHOD("get_head_light_normal_multiplier"), &TrainLighting::get_normal_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/normal_multiplier"), "set_head_light_normal_multiplier", "get_head_light_normal_multiplier");
        ClassDB::bind_method(D_METHOD("set_high_beam_dimmed_multiplier", "multiplier"), &TrainLighting::set_high_beam_dimmed_multiplier);
        ClassDB::bind_method(D_METHOD("get_high_beam_dimmed_multiplier"), &TrainLighting::get_high_beam_dimmed_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/high_beam/dimming_multiplier"), "set_high_beam_dimmed_multiplier", "get_high_beam_dimmed_multiplier");
        ClassDB::bind_method(D_METHOD("set_high_beam_multiplier", "multiplier"), &TrainLighting::set_high_beam_multiplier);
        ClassDB::bind_method(D_METHOD("get_high_beam_multiplier"), &TrainLighting::get_high_beam_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/high_beam/normal_multiplier"), "set_high_beam_multiplier", "get_high_beam_multiplier");
        ClassDB::bind_method(
                D_METHOD("set_lights_selector_default_position", "position"), &TrainLighting::set_default_selector_position);
        ClassDB::bind_method(D_METHOD("get_lights_selector_default_position"), &TrainLighting::get_default_selector_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "lights/selector_default_position"),
                "set_lights_selector_default_position", "get_lights_selector_default_position");
        ClassDB::bind_method(D_METHOD("set_light_selector_position", "light_selector_position"), &TrainLighting::set_selector_position);
        ClassDB::bind_method(D_METHOD("get_light_selector_position"), &TrainLighting::get_selector_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "lights/selector_position"),
                "set_light_selector_position", "get_light_selector_position");
        ClassDB::bind_method(
                D_METHOD("set_wrap_light_selector", "wrap"), &TrainLighting::set_wrap_light_selector);
        ClassDB::bind_method(D_METHOD("get_wrap_light_selector"), &TrainLighting::get_wrap_light_selector);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "lights/wrap_selector"),
                "set_wrap_light_selector", "get_wrap_light_selector");
        ClassDB::bind_method(D_METHOD("set_light_position_list", "light_position_list"), &TrainLighting::set_light_position_list);
        ClassDB::bind_method(D_METHOD("get_light_position_list"), &TrainLighting::get_light_position_list);
        ADD_PROPERTY(
                PropertyInfo(Variant::ARRAY, "lights/list", PROPERTY_HINT_TYPE_STRING, String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":LightListItem", PROPERTY_USAGE_DEFAULT, "TypedArray<LightListItem>"),
                "set_light_position_list", "get_light_position_list");

        ClassDB::bind_method(D_METHOD("set_light_source", "light_source"), &TrainLighting::set_light_source);
        ClassDB::bind_method(D_METHOD("get_light_source"), &TrainLighting::get_light_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "light/source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_light_source", "get_light_source");
        ClassDB::bind_method(
                D_METHOD("set_generator_engine", "generator_engine"), &TrainLighting::set_generator_engine);
        ClassDB::bind_method(D_METHOD("get_generator_engine"), &TrainLighting::get_generator_engine);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "source/generator/engine", PROPERTY_HINT_ENUM,
                        "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                        "DieselElectric,Main"),
                "set_generator_engine", "get_generator_engine");
        ClassDB::bind_method(
                D_METHOD("set_max_accumulator_voltage", "max_accumulator_voltage"),
                &TrainLighting::set_max_accumulator_voltage);
        ClassDB::bind_method(D_METHOD("get_max_accumulator_voltage"), &TrainLighting::get_max_accumulator_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "source/accumulator/max_voltage"), "set_max_accumulator_voltage",
                "get_max_accumulator_voltage");
        ClassDB::bind_method(
                D_METHOD("set_alternative_light_source", "light_source"),
                &TrainLighting::set_alternative_light_source);
        ClassDB::bind_method(D_METHOD("get_alternative_light_source"), &TrainLighting::get_alternative_light_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "light/alternative/source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_alternative_light_source", "get_alternative_light_source");
        ClassDB::bind_method(
                D_METHOD("set_alternative_max_voltage", "alternative_max_voltage"),
                &TrainLighting::set_alternative_max_voltage);
        ClassDB::bind_method(D_METHOD("get_alternative_max_voltage"), &TrainLighting::get_alternative_max_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "light/alternative/max_voltage"), "set_alternative_max_voltage",
                "get_alternative_max_voltage");
        ClassDB::bind_method(
                D_METHOD("set_alternative_light_capacity", "light_source"),
                &TrainLighting::set_alternative_light_capacity);
        ClassDB::bind_method(
                D_METHOD("get_alternative_light_capacity"), &TrainLighting::get_alternative_light_capacity);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "light/alternative/capacity"), "set_alternative_light_capacity",
                "get_alternative_light_capacity");
        ClassDB::bind_method(
                D_METHOD("set_accumulator_recharge_source", "recharge_source"),
                &TrainLighting::set_accumulator_recharge_source);
        ClassDB::bind_method(
                D_METHOD("get_accumulator_recharge_source"), &TrainLighting::get_accumulator_recharge_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "source/accumulator/recharge_source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_accumulator_recharge_source", "get_accumulator_recharge_source");
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

    void TrainLighting::set_light_source(const TrainController::TrainPowerSource p_light_source) {
        light_source = p_light_source;
        _dirty = true;
    }

    TrainController::TrainPowerSource TrainLighting::get_light_source() const {
        return light_source;
    }

    void TrainLighting::set_generator_engine(const TrainEngine::EngineType p_generator_engine) {
        generator_engine = p_generator_engine;
        _dirty = true;
    }

    TrainEngine::EngineType TrainLighting::get_generator_engine() const {
        return generator_engine;
    }

    void TrainLighting::set_max_accumulator_voltage(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLighting::get_max_accumulator_voltage() const {
        return max_accumulator_voltage;
    }

    void TrainLighting::set_alternative_light_source(const TrainController::TrainPowerSource p_light_source) {
        alternative_light_source = p_light_source;
        _dirty = true;
    }

    TrainController::TrainPowerSource TrainLighting::get_alternative_light_source() const {
        return alternative_light_source;
    }

    void TrainLighting::set_alternative_max_voltage(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLighting::get_alternative_max_voltage() const {
        return alternative_max_voltage;
    }

    void TrainLighting::set_alternative_light_capacity(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLighting::get_alternative_light_capacity() const {
        return alternative_max_voltage;
    }

    void TrainLighting::set_accumulator_recharge_source(
            const TrainController::TrainPowerSource p_accumulator_recharge_source) {
        accumulator_recharge_source = p_accumulator_recharge_source;
        _dirty = true;
    }

    TrainController::TrainPowerSource TrainLighting::get_accumulator_recharge_source() const {
        return accumulator_recharge_source;
    }

    void TrainLighting::set_light_position_list(const TypedArray<LightListItem> &p_light_position_list) {
        light_position_list.clear();
        light_position_list.append_array(p_light_position_list);
        _dirty = true;
    }

    TypedArray<LightListItem> TrainLighting::get_light_position_list() const {
        return light_position_list;
    }

    void TrainLighting::set_wrap_light_selector(const bool p_wrap_light_selector) {
        wrap_light_selector = p_wrap_light_selector;
        _dirty = true;
    }

    bool TrainLighting::get_wrap_light_selector() const {
        return wrap_light_selector;
    }

    void TrainLighting::set_default_selector_position(const int p_selector_position) {
        default_selector_position = p_selector_position;
        _dirty = true;
    }

    int TrainLighting::get_default_selector_position() const {
        return default_selector_position;
    }

    void TrainLighting::set_selector_position(const int p_selector_position) {
        selector_position = p_selector_position;
        emit_signal(SELECTOR_POSITION_CHANGED_SIGNAL, p_selector_position);
        _dirty = true;
    }

    int TrainLighting::get_selector_position() const {
        return selector_position;
    }

    void TrainLighting::set_head_light_color(const Color p_head_light_color) {
        head_light_color = p_head_light_color;
        _dirty = true;
    }

    Color TrainLighting::get_head_light_color() const {
        return head_light_color;
    }

    void TrainLighting::set_dimming_multiplier(const double p_dimming_multiplier) {
        dimming_multiplier = p_dimming_multiplier;
        _dirty = true;
    }

    double TrainLighting::get_dimming_multiplier() const {
        return dimming_multiplier;
    }

    void TrainLighting::set_normal_multiplier(const double p_normal_multiplier) {
        normal_multiplier = p_normal_multiplier;
        _dirty = true;
    }

    double TrainLighting::get_normal_multiplier() const {
        return normal_multiplier;
    }

    void TrainLighting::set_high_beam_dimmed_multiplier(const double p_high_beam_dimmed_multiplier) {
        high_beam_dimmed_multiplier = p_high_beam_dimmed_multiplier;
        _dirty = true;
    }

    double TrainLighting::get_high_beam_dimmed_multiplier() const {
        return high_beam_dimmed_multiplier;
    }

    void TrainLighting::set_high_beam_multiplier(const double p_high_beam_multiplier) {
        high_beam_multiplier = p_high_beam_multiplier;
        _dirty = true;
    }

    double TrainLighting::get_high_beam_multiplier() const {
        return high_beam_multiplier;
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
