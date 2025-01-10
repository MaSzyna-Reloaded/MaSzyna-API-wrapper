#include "TrainLightning.hpp"

namespace godot {
    void TrainLightning::_bind_methods() {
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
    }
    void TrainLightning::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {}
    void TrainLightning::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {
        TrainPart::_do_fetch_config_from_mover(mover, config);
    }
    void TrainLightning::_register_commands() {
        TrainPart::_register_commands();
    }
    void TrainLightning::_unregister_commands() {
        TrainPart::_unregister_commands();
    }

    void TrainLightning::set_light_source(const TrainElectricEngine::TrainPowerSource p_light_source) {
        light_source = p_light_source;
        _dirty = true;
    }

    TrainElectricEngine::TrainPowerSource TrainLightning::get_light_source() const {
        return light_source;
    }

    void TrainLightning::set_generator_engine(const TEngineType p_generator_engine) {
        generator_engine = p_generator_engine;
        _dirty = true;
    }

    TEngineType TrainLightning::get_generator_engine() const {
        return generator_engine;
    }

    void TrainLightning::set_max_accumulator_voltage(const double p_max_accumulator_voltage) {
        max_accumulator_voltage = p_max_accumulator_voltage;
        _dirty = true;
    }

    double TrainLightning::get_max_accumulator_voltage() const {
        return max_accumulator_voltage;
    }

    void TrainLightning::set_alternative_light_source(const TrainElectricEngine::TrainPowerSource p_light_source) {
        alternative_light_source = p_light_source;
        _dirty = true;
    }

    TrainElectricEngine::TrainPowerSource TrainLightning::get_alternative_light_source() const {
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

    void TrainLightning::set_accumulator_recharge_source(const TrainElectricEngine::TrainPowerSource p_accumulator_recharge_source) {
        accumulator_recharge_source = p_accumulator_recharge_source;
        _dirty = true;
    }

    TrainElectricEngine::TrainPowerSource TrainLightning::get_accumulator_recharge_source() const {
        return accumulator_recharge_source;
    }

} // namespace godot
