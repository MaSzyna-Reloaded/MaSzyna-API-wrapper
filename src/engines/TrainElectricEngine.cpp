#include "TrainElectricEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

#include <algorithm>
#include <cmath>

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
                enum_hint(
                        {{"NoPower", TrainController::POWER_TYPE_NONE},
                         {"BioPower", TrainController::POWER_TYPE_BIO},
                         {"MechPower", TrainController::POWER_TYPE_MECH},
                         {"ElectricPower", TrainController::POWER_TYPE_ELECTRIC},
                         {"SteamPower", TrainController::POWER_TYPE_STEAM}}));
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, power_cable_steam_pressure, "power/power_cable");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, power_current_collector_physical_layout, "power/current_collector",
                PROPERTY_HINT_FLAGS, "Front,Rear");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_resistance, "circuit");
        BIND_PROPERTY(TrainElectricEngine, Variant::INT, circuit_imax_low, "circuit");
        BIND_PROPERTY(TrainElectricEngine, Variant::INT, circuit_imax_high, "circuit");
        BIND_PROPERTY(TrainElectricEngine, Variant::INT, circuit_imin_low, "circuit");
        BIND_PROPERTY(TrainElectricEngine, Variant::INT, circuit_imin_high, "circuit");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_sum, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_diff, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_min_current, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_max_current, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::INT, circuit_tuhex_stages, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_sum_1, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_sum_2, "circuit/tuhex");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, circuit_tuhex_sum_3, "circuit/tuhex");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, cntrl_converter_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(TrainElectricEngine, Variant::FLOAT, cntrl_converter_start_delay, "cntrl");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, cntrl_converter_overload_relay_start_mode, "cntrl",
                PROPERTY_HINT_ENUM, "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(TrainElectricEngine, Variant::BOOL, cntrl_converter_overload_relay_off_when_main_is_off, "cntrl");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, cntrl_pantograph_compressor_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(TrainElectricEngine, Variant::BOOL, cntrl_pantograph_auto_valve, "cntrl");
        BIND_PROPERTY_W_HINT(
                TrainElectricEngine, Variant::INT, cntrl_main_switch_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        ClassDB::bind_method(D_METHOD("compressor", "enabled"), &TrainElectricEngine::compressor);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &TrainElectricEngine::converter);
        ClassDB::bind_method(D_METHOD("converter_fuse_reset"), &TrainElectricEngine::converter_fuse_reset);
        ClassDB::bind_method(D_METHOD("pantographs_valve", "enabled"), &TrainElectricEngine::pantographs_valve);
        ClassDB::bind_method(D_METHOD("pantograph", "selector", "enabled"), &TrainElectricEngine::pantograph);
        ClassDB::bind_method(
                D_METHOD("set_pantograph_wire_voltage", "selector", "voltage"),
                &TrainElectricEngine::set_pantograph_wire_voltage);

        BIND_ENUM_CONSTANT(PANTOGRAPH_FIRST);
        BIND_ENUM_CONSTANT(PANTOGRAPH_SECOND);
    }

    void TrainElectricEngine::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        TrainEngine::_do_fetch_state_from_mover(p_mover, p_state);
        p_state["compressor_enabled"] = p_mover->CompressorFlag;
        p_state["compressor_allowed"] = p_mover->CompressorAllow;
        p_state["converter_enabled"] = p_mover->ConverterFlag;
        p_state["converted_allowed"] = p_mover->ConverterAllow;
        p_state["converter_time_to_start"] = p_mover->ConverterStartDelayTimer;
        p_state["power_source"] = train_controller_node->tpower_source_map.at(p_mover->EnginePowerSource.SourceType);
        // RAccumulator/RPowerCable are only initialized by _do_update_internal_mover() when
        // SourceType is the matching variant (see the switch below) - reading them
        // unconditionally reads uninitialized memory for every other source type.
        if (p_mover->EnginePowerSource.SourceType == TPowerSource::Accumulator) {
            p_state["accumulator/recharge_source"] =
                    train_controller_node->tpower_source_map.at(p_mover->EnginePowerSource.RAccumulator.RechargeSource);
        }
        p_state["current_collector/max_voltage"] = p_mover->EnginePowerSource.MaxVoltage;
        p_state["current_collector/max_current"] = p_mover->EnginePowerSource.MaxCurrent;
        p_state["current_collector/max_collector_lifting"] =
                p_mover->EnginePowerSource.CollectorParameters.MaxH;
        p_state["current_collector/min_collector_lifting"] =
                p_mover->EnginePowerSource.CollectorParameters.MinH;
        p_state["current_collector/collector_sliding_width"] =
                p_mover->EnginePowerSource.CollectorParameters.CSW;
        p_state["current_collector/min_main_switch_voltage"] =
                p_mover->EnginePowerSource.CollectorParameters.MinV;
        p_state["current_collector/min_pantograph_tank_pressure"] =
                p_mover->EnginePowerSource.CollectorParameters.MinPress;
        p_state["current_collector/max_pantograph_tank_pressure"] =
                p_mover->EnginePowerSource.CollectorParameters.MaxPress;
        p_state["current_collector/overvoltage_relay"] =
                p_mover->EnginePowerSource.CollectorParameters.OVP;
        p_state["current_collector/required_main_switch_voltage"] =
                p_mover->EnginePowerSource.CollectorParameters.InsetV;
        // Live pantograph state - the mover only tracks a raised/lowered flag per pantograph
        // (no continuous extension height), matching PantographsCheck()'s own boolean state
        // machine; any raise/lower animation should tween in response to this flag changing,
        // not read a position value from the mover.
        p_state["current_collector/valve_active"] = p_mover->PantsValve.is_active;
        p_state["current_collector/pantograph_first_active"] = p_mover->Pantographs[0].is_active;
        p_state["current_collector/pantograph_first_voltage"] = p_mover->Pantographs[0].voltage;
        p_state["current_collector/pantograph_second_active"] = p_mover->Pantographs[1].is_active;
        p_state["current_collector/pantograph_second_voltage"] = p_mover->Pantographs[1].voltage;
        // Matches the original engine's own "hvoltage:" cabin gauge source (Train.cpp:6944-6946,
        // fHVoltage = max(PantographVoltage, GetTrainsetHighVoltage())) for every engine type
        // that isn't DieselElectric/ElectricInductionMotor - GetTrainsetHighVoltage() only
        // matters for a multi-unit consist sharing line voltage across couplers, so it's omitted
        // here rather than guessed at.
        p_state["current_collector/voltage"] = p_mover->PantographVoltage;
        p_state["transducer/input_voltage"] = p_mover->EnginePowerSource.Transducer.InputVoltage;
        if (p_mover->EnginePowerSource.SourceType == TPowerSource::PowerCable) {
            p_state["power_cable/source"] =
                    train_controller_node->tpower_type_map.at(p_mover->EnginePowerSource.RPowerCable.PowerTrans);
            p_state["power_cable/steam_pressure"] = p_mover->EnginePowerSource.RPowerCable.SteamPressure;
        }
    }

    void TrainElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        TrainEngine::_do_update_internal_mover(p_mover);
        // Pantographs[*].voltage/PantFrontVolt/PantRearVolt/PantographVoltage are NOT set here:
        // this only runs when the controller is dirty (effectively once, at startup), but wire
        // voltage changes every frame as the vehicle moves - see set_pantograph_wire_voltage(),
        // which writes them straight to the mover instead.
        p_mover->EnginePowerSource.SourceType = train_controller_node->power_source_map.at(power_source);

        switch (power_source) {
            case TrainController::POWER_SOURCE_INTERNAL: {
                const std::map<TrainController::TrainPowerType, TPowerType>::const_iterator lookup =
                        train_controller_node->power_type_map.find(power_cable_source);
                p_mover->EnginePowerSource.PowerType =
                        lookup != train_controller_node->power_type_map.end() ? lookup->second : TPowerType::NoPower;
                break;
            }
            case TrainController::POWER_SOURCE_TRANSDUCER: {
                p_mover->EnginePowerSource.Transducer.InputVoltage = power_transducer_input_voltage;
                break;
            }
            case TrainController::POWER_SOURCE_GENERATOR: {
                // engine_revolutions is an uninitialized raw pointer on a fresh TMoverParameters
                // (MOVER.h:551) - nothing currently dereferences EnginePowerSource's copy of it,
                // but HeatingPowerSource's copy does (see TrainHeating.cpp), so it's pointed at
                // enrot (the vehicle's own engine revolutions counter) here too, defensively.
                engine_generator &generator_params{p_mover->EnginePowerSource.EngineGenerator};
                generator_params.engine_revolutions = &p_mover->enrot;
                break;
            }
            case TrainController::POWER_SOURCE_ACCUMULATOR: {
                p_mover->EnginePowerSource.RAccumulator.RechargeSource =
                        train_controller_node->power_source_map.at(power_accumulator_recharge_source);
                break;
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
                p_mover->EnginePowerSource.CollectorParameters.PhysicalLayout = power_current_collector_physical_layout;
                break;
            }
            case TrainController::POWER_SOURCE_POWERCABLE: {
                p_mover->EnginePowerSource.RPowerCable.PowerTrans =
                        train_controller_node->power_type_map.at(power_cable_source);
                if (p_mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    p_mover->EnginePowerSource.RPowerCable.SteamPressure = power_cable_steam_pressure;
                }
                break;
            }
            case TrainController::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case TrainController::POWER_SOURCE_NOT_DEFINED:;
            default:;
        }

        /* Circuit: (elektryczny obwod napedowy), tylko pojazdy elektryczne i spalinowo-elektryczne */
        p_mover->CircuitRes = circuit_resistance;
        p_mover->ImaxLo = circuit_imax_low;
        p_mover->ImaxHi = circuit_imax_high;
        p_mover->IminLo = circuit_imin_low;
        p_mover->IminHi = circuit_imin_high;
        p_mover->TUHEX_Sum = circuit_tuhex_sum;
        p_mover->TUHEX_Diff = circuit_tuhex_diff;
        p_mover->TUHEX_MinIw = circuit_tuhex_min_current;
        p_mover->TUHEX_MaxIw = circuit_tuhex_max_current;
        p_mover->TUHEX_Stages = circuit_tuhex_stages;
        p_mover->TUHEX_Sum1 = circuit_tuhex_sum_1;
        p_mover->TUHEX_Sum2 = circuit_tuhex_sum_2;
        p_mover->TUHEX_Sum3 = circuit_tuhex_sum_3;

        p_mover->ConverterStart = start_mode_map.at(cntrl_converter_start_mode);
        p_mover->ConverterStartDelay = static_cast<float>(cntrl_converter_start_delay);
        p_mover->ConverterOverloadRelayStart = start_mode_map.at(cntrl_converter_overload_relay_start_mode);
        p_mover->ConverterOverloadRelayOffWhenMainIsOff = cntrl_converter_overload_relay_off_when_main_is_off;
        p_mover->PantographCompressorStart = start_mode_map.at(cntrl_pantograph_compressor_start_mode);
        p_mover->PantAutoValve = cntrl_pantograph_auto_valve;
        p_mover->MainsStart = start_mode_map.at(cntrl_main_switch_start_mode);
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

    void TrainElectricEngine::converter_fuse_reset() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_converteroverloadrelayreset (Train.cpp:3567-3585) ->
        // RelayReset(relay_t::primaryconverteroverload), "converterfuse_bt:"/ggConverterFuseButton
        // (Train.cpp:10053) - the converter-specific counterpart to fuse_reset()/FuseOn() above.
        mover->RelayReset(Maszyna::primaryconverteroverload);
    }

    void TrainElectricEngine::pantographs_valve(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->OperatePantographsValve(p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
    }

    void TrainElectricEngine::pantograph(const PantographSelector p_selector, const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        const Maszyna::end end = (p_selector == PANTOGRAPH_FIRST) ? Maszyna::end::front : Maszyna::end::rear;
        mover->OperatePantographValve(end, p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
        // The mover also gates every pantograph on a separate master air valve (PantsValve -
        // PantographsCheck(), Mover.cpp) that the original engine opens from its own
        // "raise selected pantograph" key command (Train.cpp's
        // OnCommand_pantographraiseselected calls OperatePantographsValve itself) for vehicles -
        // like every one wired through this wrapper's cabin today - that have no separate
        // master-valve switch of their own ("pantvalves_sw:"/ggPantValvesButton in Train.cpp is
        // genuinely absent from this vehicle's cabin, and nothing here has a keybind path
        // either). Without this, no pantograph could ever be raised through the cabin, on any
        // vehicle. Only opened here, never closed: PantographsCheck() ANDs it with each
        // pantograph's own individual valve, so leaving it open doesn't keep a lowered
        // pantograph powered - closing it here on every lower would also drop any OTHER,
        // still-raised pantograph sharing the same master valve on a two-pantograph vehicle.
        if (p_enabled) {
            mover->OperatePantographsValve(Maszyna::operation_t::enable);
        }
    }

    void TrainElectricEngine::set_pantograph_wire_voltage(const PantographSelector p_selector, const float p_voltage) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Written straight to the mover, like the other per-frame-relevant setters above
        // (pantograph(), pantographs_valve()) - _do_update_internal_mover() only runs when the
        // controller is dirty (effectively once, at startup), so stashing this in a member for
        // that path to pick up later would mean every subsequent frame's wire voltage is ignored.
        if (p_selector == PANTOGRAPH_FIRST) {
            pantograph_first_wire_voltage = p_voltage;
            mover->Pantographs[0].voltage = p_voltage;
            mover->PantFrontVolt = mover->Pantographs[0].is_active ? p_voltage : 0.0;
        } else {
            pantograph_second_wire_voltage = p_voltage;
            mover->Pantographs[1].voltage = p_voltage;
            mover->PantRearVolt = mover->Pantographs[1].is_active ? p_voltage : 0.0;
        }
        mover->PantographVoltage = std::max(std::fabs(mover->PantFrontVolt), std::fabs(mover->PantRearVolt));
    }

    void TrainElectricEngine::_register_commands() {
        TrainEngine::_register_commands();
        register_command("converter", Callable(this, "converter"));
        register_command("converter_fuse_reset", Callable(this, "converter_fuse_reset"));
        register_command("compressor", Callable(this, "compressor"));
        register_command("pantographs_valve", Callable(this, "pantographs_valve"));
        register_command("pantograph", Callable(this, "pantograph"));
    }

    void TrainElectricEngine::_unregister_commands() {
        TrainEngine::_unregister_commands();
        unregister_command("converter", Callable(this, "converter"));
        unregister_command("converter_fuse_reset", Callable(this, "converter_fuse_reset"));
        unregister_command("compressor", Callable(this, "compressor"));
        unregister_command("pantographs_valve", Callable(this, "pantographs_valve"));
        unregister_command("pantograph", Callable(this, "pantograph"));
    }


    void TrainElectricEngine::set_power_source(const TrainController::TrainPowerSource p_source) {
        power_source = p_source;
        dirty = true;
    }

    TrainController::TrainPowerSource TrainElectricEngine::get_power_source() const {
        return power_source;
    }
} // namespace godot
