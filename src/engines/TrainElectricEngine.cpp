#include "TrainElectricEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot {
    void TrainElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, power_source, "power", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::INT, power_current_collector_number_of_collectors,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_max_voltage, "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_max_current, "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_max_collector_lifting,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_min_collector_lifting,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_sliding_width, "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_min_main_switch_voltage,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_min_pantograph_tank_pressure,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_max_pantograph_tank_pressure,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::BOOL, power_current_collector_overvoltage_relay,
                "power/current_collector");
        BIND_PROPERTY(
                TrainElectricEngine, Variant::FLOAT, power_current_collector_required_main_switch_voltage,
                "power/current_collector");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, power_transducer_input_voltage, "power/transducer");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, power_accumulator_recharge_source, "power/accumulator",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, power_cable_source, "power/power_cable", PROPERTY_HINT_ENUM,
                "NoPower,BioPower,MechPower,ElectricPower,SteamPower,Main");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, power_cable_steam_pressure, "power/power_cable");
        ClassDB::bind_method(D_METHOD("compressor", "enabled"), &TrainElectricEngine::compressor);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &TrainElectricEngine::converter);
    }

    void TrainElectricEngine::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        TrainEngine::_do_fetch_state_from_mover(p_mover, p_state);
        p_state["compressor_enabled"] = p_mover->CompressorFlag;
        p_state["compressor_allowed"] = p_mover->CompressorAllow;
        p_state["converter_enabled"] = p_mover->ConverterFlag;
        p_state["converted_allowed"] = p_mover->ConverterAllow;
        p_state["converter_time_to_start"] = p_mover->ConverterStartDelayTimer;
        p_state["power_source"] = train_controller_node->tpower_source_map.at(p_mover->EnginePowerSource.SourceType);
        p_state["accumulator/recharge_source"] =
                train_controller_node->tpower_source_map.at(p_mover->EnginePowerSource.RAccumulator.RechargeSource);
        p_state["current_collector/max_voltage"] = p_mover->EnginePowerSource.MaxVoltage;
        p_state["current_collector/max_current"] = p_mover->EnginePowerSource.MaxCurrent;
        p_state["current_collector/max_collector_lifting"] = p_mover->EnginePowerSource.CollectorParameters.MaxH;
        p_state["current_collector/min_collector_lifting"] = p_mover->EnginePowerSource.CollectorParameters.MinH;
        p_state["current_collector/collector_sliding_width"] = p_mover->EnginePowerSource.CollectorParameters.CSW;
        p_state["current_collector/min_main_switch_voltage"] = p_mover->EnginePowerSource.CollectorParameters.MinV;
        p_state["current_collector/min_pantograph_tank_pressure"] =
                p_mover->EnginePowerSource.CollectorParameters.MinPress;
        p_state["current_collector/max_pantograph_tank_pressure"] =
                p_mover->EnginePowerSource.CollectorParameters.MaxPress;
        p_state["current_collector/overvoltage_relay"] = p_mover->EnginePowerSource.CollectorParameters.OVP;
        p_state["current_collector/required_main_switch_voltage"] =
                p_mover->EnginePowerSource.CollectorParameters.InsetV;
        p_state["transducer/input_voltage"] = p_mover->EnginePowerSource.Transducer.InputVoltage;
        p_state["power_cable/source"] =
                train_controller_node->tpower_type_map.at(p_mover->EnginePowerSource.RPowerCable.PowerTrans);
        p_state["power_cable/steam_pressure"] = p_mover->EnginePowerSource.RPowerCable.SteamPressure;
    }

    void TrainElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        TrainEngine::_do_update_internal_mover(p_mover);
        p_mover->EnginePowerSource.SourceType = train_controller_node->power_source_map.at(power_source);

        switch (power_source) {
            case TrainController::POWER_SOURCE_INTERNAL: {
                const std::map<TrainController::TrainPowerType, TPowerType>::const_iterator lookup =
                        train_controller_node->power_type_map.find(power_cable_source);
                p_mover->EnginePowerSource.PowerType =
                        lookup != train_controller_node->power_type_map.end() ? lookup->second : TPowerType::NoPower;
            }
            case TrainController::POWER_SOURCE_TRANSDUCER: {
                p_mover->EnginePowerSource.Transducer.InputVoltage = power_transducer_input_voltage;
            }
            case TrainController::POWER_SOURCE_GENERATOR: {
                engine_generator &generator_params{p_mover->EnginePowerSource.EngineGenerator};
                // GeneratorParams.engine_revolutions = &enrot; @TODO: Figure what the fuck is &enrot
            }
            case TrainController::POWER_SOURCE_ACCUMULATOR: {
                p_mover->EnginePowerSource.RAccumulator.RechargeSource =
                        train_controller_node->power_source_map.at(power_accumulator_recharge_source);
            }
            case TrainController::POWER_SOURCE_CURRENTCOLLECTOR: {
                p_mover->EnginePowerSource.CollectorParameters.MinH = power_current_collector_min_collector_lifting;
                p_mover->EnginePowerSource.CollectorParameters.MaxH = power_current_collector_max_collector_lifting;
                p_mover->EnginePowerSource.CollectorParameters.CSW = power_current_collector_sliding_width;
                p_mover->EnginePowerSource.CollectorParameters.MinV = power_current_collector_min_main_switch_voltage;
                p_mover->EnginePowerSource.CollectorParameters.MinPress =
                        power_current_collector_min_pantograph_tank_pressure;
                p_mover->EnginePowerSource.CollectorParameters.MaxPress =
                        power_current_collector_max_pantograph_tank_pressure;
                p_mover->EnginePowerSource.CollectorParameters.OVP = power_current_collector_overvoltage_relay;
                p_mover->EnginePowerSource.CollectorParameters.CollectorsNo =
                        power_current_collector_number_of_collectors;
                p_mover->EnginePowerSource.MaxVoltage = power_current_collector_max_voltage;
                p_mover->EnginePowerSource.MaxCurrent = power_current_collector_max_current;
                p_mover->EnginePowerSource.CollectorParameters.InsetV =
                        power_current_collector_required_main_switch_voltage;
            }
            case TrainController::POWER_SOURCE_POWERCABLE: {
                p_mover->EnginePowerSource.RPowerCable.PowerTrans =
                        train_controller_node->power_type_map.at(power_cable_source);
                if (p_mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    p_mover->EnginePowerSource.RPowerCable.SteamPressure = power_cable_steam_pressure;
                }
            }
            case TrainController::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case TrainController::POWER_SOURCE_NOT_DEFINED:;
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


    void TrainElectricEngine::set_power_source(const TrainController::TrainPowerSource p_source) {
        power_source = p_source;
        dirty = true;
    }

    TrainController::TrainPowerSource TrainElectricEngine::get_power_source() const {
        return power_source;
    }
} // namespace godot
