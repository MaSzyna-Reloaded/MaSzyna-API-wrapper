#include "TrainLightning.hpp"

namespace godot {
    void TrainLightning::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_head_light_color", "color"), &TrainLightning::set_head_light_color);
        ClassDB::bind_method(D_METHOD("get_head_light_color"), &TrainLightning::get_head_light_color);
        ADD_PROPERTY(PropertyInfo(Variant::COLOR, "head_light/color"), "set_head_light_color", "get_head_light_color");
        ClassDB::bind_method(D_METHOD("set_head_light_dimming_multiplier", "multiplier"), &TrainLightning::set_dimming_multiplier);
        ClassDB::bind_method(D_METHOD("get_head_light_dimming_multiplier"), &TrainLightning::get_dimming_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/dimming_multiplier"), "set_head_light_dimming_multiplier", "get_head_light_dimming_multiplier");
        ClassDB::bind_method(D_METHOD("set_head_light_normal_multiplier", "multiplier"), &TrainLightning::set_normal_multiplier);
        ClassDB::bind_method(D_METHOD("get_head_light_normal_multiplier"), &TrainLightning::get_normal_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/normal_multiplier"), "set_head_light_normal_multiplier", "get_head_light_normal_multiplier");
        ClassDB::bind_method(D_METHOD("set_high_beam_dimmed_multiplier", "multiplier"), &TrainLightning::set_high_beam_dimmed_multiplier);
        ClassDB::bind_method(D_METHOD("get_high_beam_dimmed_multiplier"), &TrainLightning::get_high_beam_dimmed_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/high_beam/dimming_multiplier"), "set_high_beam_dimmed_multiplier", "get_high_beam_dimmed_multiplier");
        ClassDB::bind_method(D_METHOD("set_high_beam_multiplier", "multiplier"), &TrainLightning::set_high_beam_multiplier);
        ClassDB::bind_method(D_METHOD("get_high_beam_multiplier"), &TrainLightning::get_high_beam_multiplier);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_light/high_beam/normal_multiplier"), "set_high_beam_multiplier", "get_high_beam_multiplier");
        ClassDB::bind_method(
                D_METHOD("set_default_selector_position", "position"), &TrainLightning::set_default_selector_position);
        ClassDB::bind_method(D_METHOD("get_default_selector_position"), &TrainLightning::get_default_selector_position);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "lights/default_position"),
                "set_default_selector_position", "get_default_selector_position");
        ClassDB::bind_method(
                D_METHOD("set_wrap_light_selector", "wrap"), &TrainLightning::set_wrap_light_selector);
        ClassDB::bind_method(D_METHOD("get_wrap_light_selector"), &TrainLightning::get_wrap_light_selector);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "lights/wrap_selector"),
                "set_wrap_light_selector", "get_wrap_light_selector");
        ClassDB::bind_method(D_METHOD("set_light_position_list", "light_position_list"), &TrainLightning::set_light_position_list);
        ClassDB::bind_method(D_METHOD("get_light_position_list"), &TrainLightning::get_light_position_list);
        ADD_PROPERTY(
                PropertyInfo(Variant::ARRAY, "lights/list", PROPERTY_HINT_TYPE_STRING, String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":LightListItem", PROPERTY_USAGE_DEFAULT, "TypedArray<LightListItem>"),
                "set_light_position_list", "get_light_position_list");

        ClassDB::bind_method(D_METHOD("set_light_source", "light_source"), &TrainLightning::set_light_source);
        ClassDB::bind_method(D_METHOD("get_light_source"), &TrainLightning::get_light_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "light/source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_light_source", "get_light_source");
        ClassDB::bind_method(
                D_METHOD("set_generator_engine", "generator_engine"), &TrainLightning::set_generator_engine);
        ClassDB::bind_method(D_METHOD("get_generator_engine"), &TrainLightning::get_generator_engine);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "source/generator/engine", PROPERTY_HINT_ENUM,
                        "None,Dumb,WheelsDriven,ElectricSeriesMotor,ElectricInductionMotor,DieselEngine,SteamEngine,"
                        "DieselElectric,Main"),
                "set_generator_engine", "get_generator_engine");
        ClassDB::bind_method(
                D_METHOD("set_max_accumulator_voltage", "max_accumulator_voltage"),
                &TrainLightning::set_max_accumulator_voltage);
        ClassDB::bind_method(D_METHOD("get_max_accumulator_voltage"), &TrainLightning::get_max_accumulator_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "source/accumulator/max_voltage"), "set_max_accumulator_voltage",
                "get_max_accumulator_voltage");
        ClassDB::bind_method(
                D_METHOD("set_alternative_light_source", "light_source"),
                &TrainLightning::set_alternative_light_source);
        ClassDB::bind_method(D_METHOD("get_alternative_light_source"), &TrainLightning::get_alternative_light_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "light/alternative/source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_alternative_light_source", "get_alternative_light_source");
        ClassDB::bind_method(
                D_METHOD("set_alternative_max_voltage", "alternative_max_voltage"),
                &TrainLightning::set_alternative_max_voltage);
        ClassDB::bind_method(D_METHOD("get_alternative_max_voltage"), &TrainLightning::get_alternative_max_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "light/alternative/max_voltage"), "set_alternative_max_voltage",
                "get_alternative_max_voltage");
        ClassDB::bind_method(
                D_METHOD("set_alternative_light_capacity", "light_source"),
                &TrainLightning::set_alternative_light_capacity);
        ClassDB::bind_method(
                D_METHOD("get_alternative_light_capacity"), &TrainLightning::get_alternative_light_capacity);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "light/alternative/capacity"), "set_alternative_light_capacity",
                "get_alternative_light_capacity");
        ClassDB::bind_method(
                D_METHOD("set_accumulator_recharge_source", "recharge_source"),
                &TrainLightning::set_accumulator_recharge_source);
        ClassDB::bind_method(
                D_METHOD("get_accumulator_recharge_source"), &TrainLightning::get_accumulator_recharge_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "source/accumulator/recharge_source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_accumulator_recharge_source", "get_accumulator_recharge_source");
    }
    void TrainLightning::_do_update_internal_mover(TMoverParameters *mover) {
        TrainPart::_do_update_internal_mover(mover);
        mover->LightsPosNo = static_cast<int>(light_position_list.size());
        mover->LightsWrap = wrap_light_selector;
        mover->LightsDefPos = default_selector_position;
        mover->LightPowerSource.SourceType = static_cast<TPowerSource>(light_source);
        mover->AlterLightPowerSource.SourceType = static_cast<TPowerSource>(alternative_light_source);

        //FIXME: HeadLight section is supported there but https://github.com/eu07/maszyna/blob/9aad41c0ba5a200a2935b578201dfc2f10d70662/McZapkie/Mover.cpp#L10044C31-L10044C33 original mover is probably not using it anywhere since it's trying to extract config values into non-existing variables :)
        //FIXME: Might be incomplete, needs further testing
    }

    void TrainLightning::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {

    }

    void TrainLightning::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        TrainPart::_do_fetch_config_from_mover(mover, config);
    }

    void TrainLightning::_register_commands() {
        TrainPart::_register_commands();
    }

    void TrainLightning::_unregister_commands() {
        TrainPart::_unregister_commands();
    }

    void TrainLightning::set_light_source(const TrainController::TrainPowerSource p_light_source) {
        light_source = p_light_source;
        _dirty = true;
    }

    TrainController::TrainPowerSource TrainLightning::get_light_source() const {
        return light_source;
    }

    void TrainLightning::set_generator_engine(const TrainEngine::EngineType p_generator_engine) {
        generator_engine = p_generator_engine;
        _dirty = true;
    }

    TrainEngine::EngineType TrainLightning::get_generator_engine() const {
        return generator_engine;
    }

    void TrainLightning::set_max_accumulator_voltage(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLightning::get_max_accumulator_voltage() const {
        return max_accumulator_voltage;
    }

    void TrainLightning::set_alternative_light_source(const TrainController::TrainPowerSource p_light_source) {
        alternative_light_source = p_light_source;
        _dirty = true;
    }

    TrainController::TrainPowerSource TrainLightning::get_alternative_light_source() const {
        return alternative_light_source;
    }

    void TrainLightning::set_alternative_max_voltage(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLightning::get_alternative_max_voltage() const {
        return alternative_max_voltage;
    }

    void TrainLightning::set_alternative_light_capacity(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLightning::get_alternative_light_capacity() const {
        return alternative_max_voltage;
    }

    void TrainLightning::set_accumulator_recharge_source(
            const TrainController::TrainPowerSource p_accumulator_recharge_source) {
        accumulator_recharge_source = p_accumulator_recharge_source;
        _dirty = true;
    }

    TrainController::TrainPowerSource TrainLightning::get_accumulator_recharge_source() const {
        return accumulator_recharge_source;
    }

    void TrainLightning::set_light_position_list(const TypedArray<LightListItem> &p_light_position_list) {
        light_position_list.clear();
        light_position_list.append_array(p_light_position_list);
        _dirty = true;
    }

    TypedArray<LightListItem> TrainLightning::get_light_position_list() const {
        return light_position_list;
    }

    void TrainLightning::set_wrap_light_selector(const bool p_wrap_light_selector) {
        wrap_light_selector = p_wrap_light_selector;
        _dirty = true;
    }

    bool TrainLightning::get_wrap_light_selector() const {
        return wrap_light_selector;
    }

    void TrainLightning::set_default_selector_position(const int p_selector_position) {
        default_selector_position = p_selector_position;
        _dirty = true;
    }

    int TrainLightning::get_default_selector_position() const {
        return default_selector_position;
    }

    void TrainLightning::set_head_light_color(const Color p_head_light_color) {
        head_light_color = p_head_light_color;
        _dirty = true;
    }

    Color TrainLightning::get_head_light_color() const {
        return head_light_color;
    }

    void TrainLightning::set_dimming_multiplier(const double p_dimming_multiplier) {
        dimming_multiplier = p_dimming_multiplier;
        _dirty = true;
    }

    double TrainLightning::get_dimming_multiplier() const {
        return dimming_multiplier;
    }

    void TrainLightning::set_normal_multiplier(const double p_normal_multiplier) {
        normal_multiplier = p_normal_multiplier;
        _dirty = true;
    }

    double TrainLightning::get_normal_multiplier() const {
        return normal_multiplier;
    }

    void TrainLightning::set_high_beam_dimmed_multiplier(const double p_high_beam_dimmed_multiplier) {
        high_beam_dimmed_multiplier = p_high_beam_dimmed_multiplier;
        _dirty = true;
    }

    double TrainLightning::get_high_beam_dimmed_multiplier() const {
        return high_beam_dimmed_multiplier;
    }

    void TrainLightning::set_high_beam_multiplier(const double p_high_beam_multiplier) {
        high_beam_multiplier = p_high_beam_multiplier;
        _dirty = true;
    }

    double TrainLightning::get_high_beam_multiplier() const {
        return high_beam_multiplier;
    }

} // namespace godot
