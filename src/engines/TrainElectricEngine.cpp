#include "TrainElectricEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot {
    TrainElectricEngine::TrainElectricEngine() {
        set_capability("engine");
        set_type_id("electric");
    }

    void TrainElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT(Variant::INT, "engine_power_source", "power/source", &TrainElectricEngine::set_engine_power_source, &TrainElectricEngine::get_engine_power_source, "power_source", PROPERTY_HINT_ENUM, "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(Variant::INT, "number_of_collectors", "power/current_collector/number_of_collectors", &TrainElectricEngine::set_collectors_no, &TrainElectricEngine::get_collectors_no, "number_of_collectors");
        BIND_PROPERTY(Variant::FLOAT, "max_voltage", "power/current_collector/max_voltage", &TrainElectricEngine::set_max_voltage, &TrainElectricEngine::get_max_voltage, "max_voltage");
        BIND_PROPERTY(Variant::FLOAT, "max_current", "power/current_collector/max_current", &TrainElectricEngine::set_max_current, &TrainElectricEngine::get_max_current, "max_current");
        BIND_PROPERTY(Variant::FLOAT, "max_collector_lifting", "power/current_collector/max_collector_lifting", &TrainElectricEngine::set_max_collector_lifting, &TrainElectricEngine::get_max_collector_lifting, "max_collector_lifting");
        BIND_PROPERTY(Variant::FLOAT, "min_collector_lifting", "power/current_collector/min_collector_lifting", &TrainElectricEngine::set_min_collector_lifting, &TrainElectricEngine::get_min_collector_lifting, "min_collector_lifting");
        BIND_PROPERTY(Variant::FLOAT, "collector_sliding_width", "power/current_collector/collector_sliding_width", &TrainElectricEngine::set_collector_sliding_width, &TrainElectricEngine::get_collector_sliding_width, "collector_sliding_width");
        BIND_PROPERTY(Variant::FLOAT, "min_main_switch_voltage", "power/current_collector/min_main_switch_voltage", &TrainElectricEngine::set_min_main_switch_voltage, &TrainElectricEngine::get_min_main_switch_voltage, "min_main_switch_voltage");
        BIND_PROPERTY(Variant::FLOAT, "min_pantograph_tank_pressure", "power/current_collector/min_pantograph_tank_pressure", &TrainElectricEngine::set_min_pantograph_tank_pressure, &TrainElectricEngine::get_min_pantograph_tank_pressure, "min_pantograph_tank_pressure");
        BIND_PROPERTY(Variant::FLOAT, "max_pantograph_tank_pressure", "power/current_collector/max_pantograph_tank_pressure", &TrainElectricEngine::set_max_pantograph_tank_pressure, &TrainElectricEngine::get_max_pantograph_tank_pressure, "max_pantograph_tank_pressure");
        BIND_PROPERTY(Variant::BOOL, "overvoltage_relay", "power/current_collector/overvoltage_relay", &TrainElectricEngine::set_overvoltage_relay, &TrainElectricEngine::get_overvoltage_relay, "overvoltage_relay");
        BIND_PROPERTY(Variant::FLOAT, "required_main_switch_voltage", "power/current_collector/required_main_switch_voltage", &TrainElectricEngine::set_required_main_switch_voltage, &TrainElectricEngine::get_required_main_switch_voltage, "required_main_switch_voltage");
        BIND_PROPERTY(Variant::FLOAT, "transducer_input_voltage", "power/transducer/input_voltage", &TrainElectricEngine::set_transducer_input_voltage, &TrainElectricEngine::get_transducer_input_voltage, "transducer_input_voltage");
        BIND_PROPERTY_W_HINT(Variant::INT, "accumulator_recharge_source", "power/accumulator/recharge_source", &TrainElectricEngine::set_accumulator_recharge_source, &TrainElectricEngine::get_accumulator_recharge_source, "accumulator_recharge_source", PROPERTY_HINT_ENUM, "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(Variant::INT, "power_cable_power_source", "power/power_cable/source", &TrainElectricEngine::set_power_cable_power_source, &TrainElectricEngine::get_power_cable_power_source, "power_cable_power_source", PROPERTY_HINT_ENUM, "NoPower,BioPower,MechPower,ElectricPower,SteamPower,Main");
        BIND_PROPERTY(Variant::FLOAT, "power_cable_steam_pressure", "power/power_cable/steam_pressure", &TrainElectricEngine::set_power_cable_steam_pressure, &TrainElectricEngine::get_power_cable_steam_pressure, "power_cable_steam_pressure");
        ClassDB::bind_method(D_METHOD("compressor", "enabled"), &TrainElectricEngine::compressor);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &TrainElectricEngine::converter);
    }

    void TrainElectricEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        TrainEngine::_do_fetch_state_from_mover(mover, state);
        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;
        state["converter_enabled"] = mover->ConverterFlag;
        state["converted_allowed"] = mover->ConverterAllow;
        state["converter_time_to_start"] = mover->ConverterStartDelayTimer;
        state["power_source"] = _controller->tpower_source_map.at(mover->EnginePowerSource.SourceType);
        state["accumulator/recharge_source"] = _controller->tpower_source_map.at(mover->EnginePowerSource.RAccumulator.RechargeSource);
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
        state["power_cable/source"] = _controller->tpower_type_map.at(mover->EnginePowerSource.RPowerCable.PowerTrans);
        state["power_cable/steam_pressure"] = mover->EnginePowerSource.RPowerCable.SteamPressure;
    }

    void TrainElectricEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainEngine::_do_update_internal_mover(mover);
        mover->EnginePowerSource.SourceType = _controller->power_source_map.at(power_source);

        switch (power_source) {
            case TrainNode::POWER_SOURCE_INTERNAL: {
                const std::map<TrainNode::TrainPowerType, TPowerType>::const_iterator lookup =
                        _controller->power_type_map.find(power_cable_power_source);
                mover->EnginePowerSource.PowerType = lookup != _controller->power_type_map.end() ? lookup->second : TPowerType::NoPower;
            }
            case TrainNode::POWER_SOURCE_TRANSDUCER: {
                mover->EnginePowerSource.Transducer.InputVoltage = transducer_input_voltage;
            }
            case TrainNode::POWER_SOURCE_GENERATOR: {
                engine_generator &GeneratorParams{mover->EnginePowerSource.EngineGenerator};
                // GeneratorParams.engine_revolutions = &enrot; @TODO: Figure what the fuck is &enrot
            }
            case TrainNode::POWER_SOURCE_ACCUMULATOR: {
                mover->EnginePowerSource.RAccumulator.RechargeSource =
                        _controller->power_source_map.at(accumulator_recharge_source);
            }
            case TrainNode::POWER_SOURCE_CURRENTCOLLECTOR: {
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
            case TrainNode::POWER_SOURCE_POWERCABLE: {
                mover->EnginePowerSource.RPowerCable.PowerTrans =
                    _controller->power_type_map.at(power_cable_power_source);
                if (mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    mover->EnginePowerSource.RPowerCable.SteamPressure = power_cable_steam_pressure;
                }
            }
            case TrainNode::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case TrainNode::POWER_SOURCE_NOT_DEFINED:;
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


    void TrainElectricEngine::set_engine_power_source(const TrainNode::TrainPowerSource p_source) {
        power_source = p_source;
        _dirty = true;
    }

    TrainNode::TrainPowerSource TrainElectricEngine::get_engine_power_source() const {
        return power_source;
    }
} // namespace godot
