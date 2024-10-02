#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

#include "TrainElectricEngine.hpp"

namespace godot {
    TrainElectricEngine::TrainElectricEngine() = default;

    void TrainElectricEngine::_bind_methods() {

        ClassDB::bind_method(D_METHOD("set_engine_power_source"), &TrainElectricEngine::set_engine_power_source);
        ClassDB::bind_method(D_METHOD("get_engine_power_source"), &TrainElectricEngine::get_engine_power_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "power/source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_engine_power_source", "get_engine_power_source");

        ClassDB::bind_method(D_METHOD("set_number_of_collectors"), &TrainElectricEngine::set_number_of_collectors);
        ClassDB::bind_method(D_METHOD("get_number_of_collectors"), &TrainElectricEngine::get_number_of_collectors);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "power/current_collector/number_of_collectors"), "set_number_of_collectors",
                "get_number_of_collectors");

        ClassDB::bind_method(D_METHOD("set_max_voltage"), &TrainElectricEngine::set_max_voltage);
        ClassDB::bind_method(D_METHOD("get_max_voltage"), &TrainElectricEngine::get_max_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/max_voltage"), "set_max_voltage",
                "get_max_voltage");

        ClassDB::bind_method(D_METHOD("set_max_current"), &TrainElectricEngine::set_max_current);
        ClassDB::bind_method(D_METHOD("get_max_current"), &TrainElectricEngine::get_max_current);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/max_current"), "set_max_current",
                "get_max_current");

        ClassDB::bind_method(D_METHOD("set_max_collector_lifting"), &TrainElectricEngine::set_max_collector_lifting);
        ClassDB::bind_method(D_METHOD("get_max_collector_lifting"), &TrainElectricEngine::get_max_collector_lifting);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/max_collector_lifting"),
                "set_max_collector_lifting", "get_max_collector_lifting");

        ClassDB::bind_method(D_METHOD("set_min_collector_lifting"), &TrainElectricEngine::set_min_collector_lifting);
        ClassDB::bind_method(D_METHOD("get_min_collector_lifting"), &TrainElectricEngine::get_min_collector_lifting);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/min_collector_lifting"),
                "set_min_collector_lifting", "get_min_collector_lifting");

        ClassDB::bind_method(D_METHOD("set_csw"), &TrainElectricEngine::set_csw);
        ClassDB::bind_method(D_METHOD("get_csw"), &TrainElectricEngine::get_csw);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/collector_sliding_width"), "set_csw", "get_csw");

        ClassDB::bind_method(
                D_METHOD("set_min_main_switch_voltage"), &TrainElectricEngine::set_min_main_switch_voltage);
        ClassDB::bind_method(
                D_METHOD("get_min_main_switch_voltage"), &TrainElectricEngine::get_min_main_switch_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/min_main_switch_voltage"),
                "set_min_main_switch_voltage", "get_min_main_switch_voltage");

        ClassDB::bind_method(
                D_METHOD("set_min_pantograph_tank_pressure"), &TrainElectricEngine::set_min_pantograph_tank_pressure);
        ClassDB::bind_method(
                D_METHOD("get_min_pantograph_tank_pressure"), &TrainElectricEngine::get_min_pantograph_tank_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/min_pantograph_tank_pressure"),
                "set_min_pantograph_tank_pressure", "get_min_pantograph_tank_pressure");

        ClassDB::bind_method(
                D_METHOD("set_max_pantograph_tank_pressure"), &TrainElectricEngine::set_max_pantograph_tank_pressure);
        ClassDB::bind_method(
                D_METHOD("get_max_pantograph_tank_pressure"), &TrainElectricEngine::get_max_pantograph_tank_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/max_pantograph_tank_pressure"),
                "set_max_pantograph_tank_pressure", "get_max_pantograph_tank_pressure");

        ClassDB::bind_method(D_METHOD("set_overvoltage_relay"), &TrainElectricEngine::set_overvoltage_relay);
        ClassDB::bind_method(D_METHOD("get_overvoltage_relay"), &TrainElectricEngine::get_overvoltage_relay);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "power/current_collector/overvoltage_relay"), "set_overvoltage_relay",
                "get_overvoltage_relay");

        ClassDB::bind_method(
                D_METHOD("set_required_main_switch_voltage"), &TrainElectricEngine::set_required_main_switch_voltage);
        ClassDB::bind_method(
                D_METHOD("get_required_main_switch_voltage"), &TrainElectricEngine::get_required_main_switch_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/current_collector/required_main_switch_voltage"),
                "set_required_main_switch_voltage", "get_required_main_switch_voltage");

        ClassDB::bind_method(
                D_METHOD("set_transducer_input_voltage"), &TrainElectricEngine::set_transducer_input_voltage);
        ClassDB::bind_method(
                D_METHOD("get_transducer_input_voltage"), &TrainElectricEngine::get_transducer_input_voltage);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/transducer/input_voltage"), "set_transducer_input_voltage",
                "get_transducer_input_voltage");

        ClassDB::bind_method(
                D_METHOD("set_accumulator_recharge_source"), &TrainElectricEngine::set_accumulator_recharge_source);
        ClassDB::bind_method(
                D_METHOD("get_accumulator_recharge_source"), &TrainElectricEngine::get_accumulator_recharge_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "power/accumulator/recharge_source", PROPERTY_HINT_ENUM,
                        "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,"
                        "Main"),
                "set_accumulator_recharge_source", "get_accumulator_recharge_source");

        ClassDB::bind_method(
                D_METHOD("set_power_cable_power_source"), &TrainElectricEngine::set_power_cable_power_source);
        ClassDB::bind_method(
                D_METHOD("get_power_cable_power_source"), &TrainElectricEngine::get_power_cable_power_source);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::INT, "power/power_cable/source", PROPERTY_HINT_ENUM,
                        "NoPower,BioPower,MechPower,ElectricPower,SteamPower"
                        "Main"),
                "set_power_cable_power_source", "get_power_cable_power_source");

        ClassDB::bind_method(
                D_METHOD("set_power_cable_steam_pressure"), &TrainElectricEngine::set_power_cable_steam_pressure);
        ClassDB::bind_method(
                D_METHOD("get_power_cable_steam_pressure"), &TrainElectricEngine::get_power_cable_steam_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "power/power_cable/steam_pressure"), "set_power_cable_steam_pressure",
                "get_power_cable_steam_pressure");

        ClassDB::bind_method(D_METHOD("compressor", "enabled"), &TrainElectricEngine::compressor);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &TrainElectricEngine::converter);

        BIND_ENUM_CONSTANT(POWER_SOURCE_NOT_DEFINED);
        BIND_ENUM_CONSTANT(POWER_SOURCE_INTERNAL);
        BIND_ENUM_CONSTANT(POWER_SOURCE_TRANSDUCER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_GENERATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_ACCUMULATOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_CURRENTCOLLECTOR);
        BIND_ENUM_CONSTANT(POWER_SOURCE_POWERCABLE);
        BIND_ENUM_CONSTANT(POWER_SOURCE_HEATER);
        BIND_ENUM_CONSTANT(POWER_SOURCE_MAIN);

        BIND_ENUM_CONSTANT(POWER_TYPE_NONE);
        BIND_ENUM_CONSTANT(POWER_TYPE_BIO);
        BIND_ENUM_CONSTANT(POWER_TYPE_MECH);
        BIND_ENUM_CONSTANT(POWER_TYPE_ELECTRIC);
        BIND_ENUM_CONSTANT(POWER_TYPE_STEAM);
    }

    void TrainElectricEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        TrainEngine::_do_fetch_state_from_mover(mover, state);

        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;
        state["converter_enabled"] = mover->ConverterFlag;
        state["converted_allowed"] = mover->ConverterAllow;
        state["converter_time_to_start"] = mover->ConverterStartDelayTimer;
        state["power_source"] = static_cast<TrainPowerSource>(static_cast<int>(mover->EnginePowerSource.SourceType));
        state["accumulator/recharge_source"] =
                static_cast<TrainPowerSource>(static_cast<int>(mover->EnginePowerSource.RAccumulator.RechargeSource));
        state["current_collector/max_voltage"] = mover->EnginePowerSource.MaxVoltage;
        state["current_collector/max_current"] = mover->EnginePowerSource.MaxCurrent;
        state["current_collector/max_collector_lifting"] = mover->EnginePowerSource.CollectorParameters.MaxH;
        state["current_collector/min_collector_lifting"] = mover->EnginePowerSource.CollectorParameters.MinH;
        state["current_collector/collector_sliding_width"] = mover->EnginePowerSource.CollectorParameters.CSW;
        state["current_collector/min_main_switch_voltage"] = mover->EnginePowerSource.CollectorParameters.MinV;
        state["current_collector/min_pantograph_tank_pressure"] = mover->EnginePowerSource.CollectorParameters.MinPress;
        state["current_collector/max_pantograph_tank_pressure"] = mover->EnginePowerSource.CollectorParameters.MaxPress;
        state["current_collector/overvoltage_relay"] = mover->EnginePowerSource.CollectorParameters.OVP;
        state["current_collector/required_main_switch_voltage"] = mover->EnginePowerSource.CollectorParameters.InsetV;
        state["transducer/input_voltage"] = mover->EnginePowerSource.Transducer.InputVoltage;
        state["power_cable/source"] =
                static_cast<TrainPowerType>(static_cast<int>(mover->EnginePowerSource.RPowerCable.PowerTrans));
        state["power_cable/steam_pressure"] = mover->EnginePowerSource.RPowerCable.SteamPressure;
    }

    void TrainElectricEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainEngine::_do_update_internal_mover(mover);

        mover->EnginePowerSource.SourceType = static_cast<TPowerSource>(power_source);

        switch (power_source) {
            case TrainPowerSource::POWER_SOURCE_INTERNAL:; //@TODO: Implement
                                                           // mover->EnginePowerSource.PowerType =
                                                           // LoadFIZ_PowerDecode
            case TrainPowerSource::POWER_SOURCE_TRANSDUCER: {
                mover->EnginePowerSource.Transducer.InputVoltage = transducer_input_voltage;
            }
            case TrainPowerSource::POWER_SOURCE_GENERATOR: {
                engine_generator &GeneratorParams{mover->EnginePowerSource.EngineGenerator};
                // GeneratorParams.engine_revolutions = &enrot; @TODO: Figure what the fuck is &enrot
            }
            case TrainElectricEngine::POWER_SOURCE_ACCUMULATOR: {
                mover->EnginePowerSource.RAccumulator.RechargeSource =
                        static_cast<TPowerSource>(static_cast<int>(accumulator_recharge_source));
            }
            case TrainElectricEngine::POWER_SOURCE_CURRENTCOLLECTOR: {
                mover->EnginePowerSource.CollectorParameters.MinH = min_collector_lifting;
                mover->EnginePowerSource.CollectorParameters.MaxH = max_collector_lifting;
                mover->EnginePowerSource.CollectorParameters.CSW = collector_sliding_width;
                mover->EnginePowerSource.CollectorParameters.MinV = min_main_switch_voltage;
                mover->EnginePowerSource.CollectorParameters.MinPress = min_pantograph_tank_pressure;
                mover->EnginePowerSource.CollectorParameters.MaxPress = max_pantograph_tank_pressure;
                mover->EnginePowerSource.CollectorParameters.OVP = overvoltage_relay;
                mover->EnginePowerSource.CollectorParameters.CollectorsNo = collectors_no;
                mover->EnginePowerSource.MaxVoltage = max_voltage;
                mover->EnginePowerSource.MaxCurrent = max_current;
                mover->EnginePowerSource.CollectorParameters.InsetV = required_main_switch_voltage;
            }
            case TrainElectricEngine::POWER_SOURCE_POWERCABLE: {
                mover->EnginePowerSource.RPowerCable.PowerTrans =
                        static_cast<TPowerType>(static_cast<int>(power_cable_power_trans));
                if (mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    mover->EnginePowerSource.RPowerCable.SteamPressure = power_cable_steam_pressure;
                }
            }
            case TrainElectricEngine::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case TrainElectricEngine::POWER_SOURCE_NOT_DEFINED:;
            default:;
        }
    }

    void TrainElectricEngine::converter(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->ConverterSwitch(p_enabled);
    }

    void TrainElectricEngine::compressor(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->CompressorSwitch(p_enabled);
    }

    void TrainElectricEngine::_register_commands() {
        TrainEngine::_register_commands();
        register_command("converter", Callable(this, "converter"));
        register_command("compressor", Callable(this, "compressor"));
    }

    void TrainElectricEngine::_unregister_commands() {
        TrainEngine::_unregister_commands();
        unregister_command("converter", Callable(this, "converter"));
        unregister_command("compressor", Callable(this, "compressor"));
    }


    void TrainElectricEngine::set_engine_power_source(const TrainPowerSource p_source) {
        power_source = p_source;
        _dirty = true;
    }

    TrainElectricEngine::TrainPowerSource TrainElectricEngine::get_engine_power_source() const {
        return power_source;
    }

    void TrainElectricEngine::set_number_of_collectors(const int p_coll_no) {
        collectors_no = p_coll_no;
        _dirty = true;
    }

    int TrainElectricEngine::get_number_of_collectors() const {
        return collectors_no;
    }

    void TrainElectricEngine::set_max_voltage(const float p_max_voltage) {
        max_voltage = p_max_voltage;
        _dirty = true;
    }

    float TrainElectricEngine::get_max_voltage() const {
        return max_voltage;
    }

    void TrainElectricEngine::set_max_current(const float p_max_current) {
        max_current = p_max_current;
        _dirty = true;
    }

    float TrainElectricEngine::get_max_current() const {
        return max_current;
    }

    void TrainElectricEngine::set_max_collector_lifting(const float p_max_collector_lifting) {
        max_collector_lifting = p_max_collector_lifting;
        _dirty = true;
    }

    float TrainElectricEngine::get_max_collector_lifting() const {
        return max_collector_lifting;
    }

    void TrainElectricEngine::set_min_collector_lifting(const float p_min_collector_lifting) {
        min_collector_lifting = p_min_collector_lifting;
        _dirty = true;
    }

    float TrainElectricEngine::get_min_collector_lifting() const {
        return min_collector_lifting;
    }

    void TrainElectricEngine::set_csw(const float p_csw) {
        collector_sliding_width = p_csw;
        _dirty = true;
    }

    float TrainElectricEngine::get_csw() const {
        return collector_sliding_width;
    }

    void TrainElectricEngine::set_min_main_switch_voltage(const float p_min_main_switch_voltage) {
        min_main_switch_voltage = p_min_main_switch_voltage;
        _dirty = true;
    }

    float TrainElectricEngine::get_min_main_switch_voltage() const {
        return min_main_switch_voltage;
    }

    void TrainElectricEngine::set_min_pantograph_tank_pressure(const float p_min_pantograph_tank_pressure) {
        min_pantograph_tank_pressure = p_min_pantograph_tank_pressure;
        _dirty = true;
    }

    float TrainElectricEngine::get_min_pantograph_tank_pressure() const {
        return min_pantograph_tank_pressure;
    }

    void TrainElectricEngine::set_max_pantograph_tank_pressure(const float p_max_pantograph_tank_pressure) {
        max_pantograph_tank_pressure = p_max_pantograph_tank_pressure;
        _dirty = true;
    }

    float TrainElectricEngine::get_max_pantograph_tank_pressure() const {
        return max_pantograph_tank_pressure;
    }

    void TrainElectricEngine::set_overvoltage_relay(const bool p_overvoltage_relay_value) {
        overvoltage_relay = p_overvoltage_relay_value;
        _dirty = true;
    }

    bool TrainElectricEngine::get_overvoltage_relay() const {
        return overvoltage_relay;
    }

    void TrainElectricEngine::set_required_main_switch_voltage(const float p_required_main_switch_voltage) {
        required_main_switch_voltage = p_required_main_switch_voltage;
        _dirty = true;
    }

    float TrainElectricEngine::get_required_main_switch_voltage() const {
        return required_main_switch_voltage;
    }

    void TrainElectricEngine::set_transducer_input_voltage(const float p_transducer_input_voltage) {
        transducer_input_voltage = p_transducer_input_voltage;
        _dirty = true;
    }

    float TrainElectricEngine::get_transducer_input_voltage() const {
        return transducer_input_voltage;
    }

    void TrainElectricEngine::set_accumulator_recharge_source(const TrainPowerSource p_source) {
        accumulator_recharge_source = p_source;
        _dirty = true;
    }

    TrainElectricEngine::TrainPowerSource TrainElectricEngine::get_accumulator_recharge_source() const {
        return accumulator_recharge_source;
    }

    void TrainElectricEngine::set_power_cable_power_source(const TrainPowerType p_source) {
        power_cable_power_trans = p_source;
        _dirty = true;
    }

    TrainElectricEngine::TrainPowerType TrainElectricEngine::get_power_cable_power_source() const {
        return power_cable_power_trans;
    }

    void TrainElectricEngine::set_power_cable_steam_pressure(const float p_pressure) {
        power_cable_steam_pressure = p_pressure;
        _dirty = true;
    }

    float TrainElectricEngine::get_power_cable_steam_pressure() const {
        return power_cable_steam_pressure;
    }
} // namespace godot
