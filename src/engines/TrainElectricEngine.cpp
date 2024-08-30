#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "TrainElectricEngine.hpp"
#include "maszyna/McZapkie/MOVER.h"

namespace godot {
    TrainElectricEngine::TrainElectricEngine() = default;

    void TrainElectricEngine::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("set_compressor_switch_pressed"), &TrainElectricEngine::set_compressor_switch_pressed);
        ClassDB::bind_method(
                D_METHOD("get_compressor_switch_pressed"), &TrainElectricEngine::get_compressor_switch_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/compressor"), "set_compressor_switch_pressed",
                "get_compressor_switch_pressed");

        ClassDB::bind_method(
                D_METHOD("set_converter_switch_pressed"), &TrainElectricEngine::set_converter_switch_pressed);
        ClassDB::bind_method(
                D_METHOD("get_converter_switch_pressed"), &TrainElectricEngine::get_converter_switch_pressed);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/converter"), "set_converter_switch_pressed",
                "get_converter_switch_pressed");

        ClassDB::bind_method(
                D_METHOD("set_engine_power_source"), &TrainElectricEngine::set_engine_power_source);
        ClassDB::bind_method(
                D_METHOD("get_engine_power_source"), &TrainElectricEngine::get_engine_power_source);
        ADD_PROPERTY(
        PropertyInfo(Variant::INT, "power/source", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main"), "set_engine_power_source",
                "get_engine_power_source");

        ClassDB::bind_method(
                D_METHOD("set_number_of_collectors"), &TrainElectricEngine::set_number_of_collectors);
        ClassDB::bind_method(
                D_METHOD("get_number_of_collectors"), &TrainElectricEngine::get_number_of_collectors);
        ADD_PROPERTY(
        PropertyInfo(Variant::INT, "power/number_of_collectors"), "set_number_of_collectors",
        "get_number_of_collectors");

        ClassDB::bind_method(
                D_METHOD("set_max_voltage"), &TrainElectricEngine::set_max_voltage);
        ClassDB::bind_method(
                D_METHOD("get_max_voltage"), &TrainElectricEngine::get_max_voltage);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/max_voltage"), "set_max_voltage",
        "get_max_voltage");

        ClassDB::bind_method(
                D_METHOD("set_max_current"), &TrainElectricEngine::set_max_current);
        ClassDB::bind_method(
                D_METHOD("get_max_current"), &TrainElectricEngine::get_max_current);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/max_current"), "set_max_current",
        "get_max_current");

        ClassDB::bind_method(
                D_METHOD("set_max_collector_lifting"), &TrainElectricEngine::set_max_collector_lifting);
        ClassDB::bind_method(
                D_METHOD("get_max_collector_lifting"), &TrainElectricEngine::get_max_collector_lifting);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/max_collector_lifting"), "set_max_collector_lifting",
        "get_max_collector_lifting");

        ClassDB::bind_method(
                D_METHOD("set_min_collector_lifting"), &TrainElectricEngine::set_min_collector_lifting);
        ClassDB::bind_method(
                D_METHOD("get_min_collector_lifting"), &TrainElectricEngine::get_min_collector_lifting);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/min_collector_lifting"), "set_min_collector_lifting",
        "get_min_collector_lifting");

        ClassDB::bind_method(
                D_METHOD("set_csw"), &TrainElectricEngine::set_csw);
        ClassDB::bind_method(
                D_METHOD("get_csw"), &TrainElectricEngine::get_csw);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/csw"), "set_csw",
        "get_csw");

        ClassDB::bind_method(
                D_METHOD("set_min_main_switch_voltage"), &TrainElectricEngine::set_min_main_switch_voltage);
        ClassDB::bind_method(
                D_METHOD("get_min_main_switch_voltage"), &TrainElectricEngine::get_min_main_switch_voltage);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/min_main_switch_voltage"), "set_min_main_switch_voltage",
        "get_min_main_switch_voltage");

        ClassDB::bind_method(
                D_METHOD("set_min_pantograph_tank_pressure"), &TrainElectricEngine::set_min_pantograph_tank_pressure);
        ClassDB::bind_method(
                D_METHOD("get_min_pantograph_tank_pressure"), &TrainElectricEngine::get_min_pantograph_tank_pressure);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/min_pantograph_tank_pressure"), "set_min_pantograph_tank_pressure",
        "get_min_pantograph_tank_pressure");

        ClassDB::bind_method(
                D_METHOD("set_max_pantograph_tank_pressure"), &TrainElectricEngine::set_max_pantograph_tank_pressure);
        ClassDB::bind_method(
                D_METHOD("get_max_pantograph_tank_pressure"), &TrainElectricEngine::get_max_pantograph_tank_pressure);
        ADD_PROPERTY(
        PropertyInfo(Variant::FLOAT, "power/max_pantograph_tank_pressure"), "set_max_pantograph_tank_pressure",
                "get_max_pantograph_tank_pressure");
    }

    void TrainElectricEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        TrainEngine::_do_fetch_state_from_mover(mover, state);

        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;
        state["converter_enabled"] = mover->ConverterFlag;
        state["converted_allowed"] = mover->ConverterAllow;
        state["converter_time_to_start"] = mover->ConverterStartDelayTimer;
        state["power_source"] = static_cast<int>(mover->EnginePowerSource.SourceType);
        state["max_voltage"] = mover->EnginePowerSource.MaxVoltage;
        state["max_current"] = mover->EnginePowerSource.MaxCurrent;
        state["max_collector_lifting"] = mover->EnginePowerSource.CollectorParameters.MaxH;
        state["min_collector_lifting"] = mover->EnginePowerSource.CollectorParameters.MinH;
        state["csw"] = mover->EnginePowerSource.CollectorParameters.CSW;
        state["min_main_switch_voltage"] = mover->EnginePowerSource.CollectorParameters.MinV;
        state["min_pantograph_tank_pressure"] = mover->EnginePowerSource.CollectorParameters.MinPress;
        state["max_pantograph_tank_pressure"] = mover->EnginePowerSource.CollectorParameters.MaxPress;
    }

    void TrainElectricEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainEngine::_do_update_internal_mover(mover);

        // FIXME: test data
        mover->EnginePowerSource.SourceType = static_cast<TPowerSource>(power_source);
        mover->EnginePowerSource.CollectorParameters.CollectorsNo = collectors_no;
        mover->EnginePowerSource.MaxVoltage = max_voltage;
        mover->EnginePowerSource.MaxCurrent = max_current;
        mover->CompressorSwitch(compressor_switch_pressed);
        mover->ConverterSwitch(converter_switch_pressed);
        mover->EnginePowerSource.CollectorParameters.MinH = min_collector_lifting;
        mover->EnginePowerSource.CollectorParameters.MaxH = max_collector_lifting;
        mover->EnginePowerSource.CollectorParameters.CSW = csw;
        mover->EnginePowerSource.CollectorParameters.MinV = min_main_switch_voltage;
        mover->EnginePowerSource.CollectorParameters.MinPress = min_pantograph_tank_pressure;
        mover->EnginePowerSource.CollectorParameters.MaxPress = max_pantograph_tank_pressure;
    }

    void TrainElectricEngine::set_converter_switch_pressed(const bool p_state) {
        converter_switch_pressed = p_state;
        _dirty = true;
    }

    bool TrainElectricEngine::get_converter_switch_pressed() const {
        return converter_switch_pressed;
    }

    void TrainElectricEngine::set_compressor_switch_pressed(const bool p_state) {
        compressor_switch_pressed = p_state;
        _dirty = true;
    }

    bool TrainElectricEngine::get_compressor_switch_pressed() const {
        return compressor_switch_pressed;
    }

    void TrainElectricEngine::set_engine_power_source(const int p_source) {
        power_source = p_source;
        _dirty = true;
    }

    int TrainElectricEngine::get_engine_power_source() const {
        return power_source;
    }

    void TrainElectricEngine::set_number_of_collectors(const int p_coll_no) {
        collectors_no = p_coll_no;
    }

    int TrainElectricEngine::get_number_of_collectors() const {
        return collectors_no;
    }

    void TrainElectricEngine::set_max_voltage(const float p_max_voltage) {
        max_voltage = p_max_voltage;
    }

    float TrainElectricEngine::get_max_voltage() const {
        return max_voltage;
    }

    void TrainElectricEngine::set_max_current(const float p_max_current) {
        max_current = p_max_current;
    }

    float TrainElectricEngine::get_max_current() const {
        return max_current;
    }

    void TrainElectricEngine::set_max_collector_lifting(const float p_max_collector_lifting) {
        max_collector_lifting = p_max_collector_lifting;
    }

    float TrainElectricEngine::get_max_collector_lifting() const {
        return max_collector_lifting;
    }

    void TrainElectricEngine::set_min_collector_lifting(const float p_min_collector_lifting) {
        min_collector_lifting = p_min_collector_lifting;
    }

    float TrainElectricEngine::get_min_collector_lifting() const {
        return min_collector_lifting;
    }

    void TrainElectricEngine::set_csw(const float p_csw) {
        csw = p_csw;
    }

    float TrainElectricEngine::get_csw() const {
        return csw;
    }

    void TrainElectricEngine::set_min_main_switch_voltage(const float p_min_main_switch_voltage) {
        min_main_switch_voltage = p_min_main_switch_voltage;
    }

    float TrainElectricEngine::get_min_main_switch_voltage() const {
        return min_main_switch_voltage;
    }

    void TrainElectricEngine::set_min_pantograph_tank_pressure(const float p_min_pantograph_tank_pressure) {
        min_pantograph_tank_pressure = p_min_pantograph_tank_pressure;
    }

    float TrainElectricEngine::get_min_pantograph_tank_pressure() const {
        return min_pantograph_tank_pressure;
    }

    void TrainElectricEngine::set_max_pantograph_tank_pressure(const float p_max_pantograph_tank_pressure) {
        max_pantograph_tank_pressure = p_max_pantograph_tank_pressure;
    }

    float TrainElectricEngine::get_max_pantograph_tank_pressure() const {
        return max_pantograph_tank_pressure;
    }

} // namespace godot
