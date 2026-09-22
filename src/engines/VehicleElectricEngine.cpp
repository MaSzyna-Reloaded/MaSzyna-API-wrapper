#include "VehicleElectricEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

#include <algorithm>
#include <cmath>

namespace godot {
    void VehicleElectricEngine::_bind_methods() {
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_source, "power", PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::INT, power_current_collector_number_of_collectors,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_voltage, "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_current, "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_collector_lifting,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_min_collector_lifting,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_sliding_width, "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_min_main_switch_voltage,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_min_pantograph_tank_pressure,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_max_pantograph_tank_pressure,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::BOOL, power_current_collector_overvoltage_relay,
                "power/current_collector");
        BIND_PROPERTY(
                VehicleElectricEngine, Variant::FLOAT, power_current_collector_required_main_switch_voltage,
                "power/current_collector");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, power_transducer_input_voltage, "power/transducer");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_accumulator_recharge_source, "power/accumulator",
                PROPERTY_HINT_ENUM,
                "NotDefined,InternalSource,Transducer,Generator,Accumulator,CurrentCollector,PowerCable,Heater,Main");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_cable_source, "power/power_cable", PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"NoPower", VehicleController::POWER_TYPE_NONE},
                         {"BioPower", VehicleController::POWER_TYPE_BIO},
                         {"MechPower", VehicleController::POWER_TYPE_MECH},
                         {"ElectricPower", VehicleController::POWER_TYPE_ELECTRIC},
                         {"SteamPower", VehicleController::POWER_TYPE_STEAM}}));
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, power_cable_steam_pressure, "power/power_cable");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, power_current_collector_physical_layout, "power/current_collector",
                PROPERTY_HINT_FLAGS, "Front,Rear");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_resistance, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imax_low, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imax_high, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imin_low, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_imin_high, "circuit");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_diff, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_min_current, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_max_current, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::INT, circuit_tuhex_stages, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum_1, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum_2, "circuit/tuhex");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, circuit_tuhex_sum_3, "circuit/tuhex");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_converter_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleElectricEngine, Variant::FLOAT, cntrl_converter_start_delay, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_converter_overload_relay_start_mode, "cntrl",
                PROPERTY_HINT_ENUM, "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleElectricEngine, Variant::BOOL, cntrl_converter_overload_relay_off_when_main_is_off, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_pantograph_compressor_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleElectricEngine, Variant::BOOL, cntrl_pantograph_auto_valve, "cntrl");
        BIND_PROPERTY_W_HINT(
                VehicleElectricEngine, Variant::INT, cntrl_main_switch_start_mode, "cntrl", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        ClassDB::bind_method(D_METHOD("compressor", "enabled"), &VehicleElectricEngine::compressor);
        ClassDB::bind_method(D_METHOD("converter", "enabled"), &VehicleElectricEngine::converter);
        ClassDB::bind_method(D_METHOD("converter_fuse_reset"), &VehicleElectricEngine::converter_fuse_reset);
        ClassDB::bind_method(D_METHOD("pantographs_valve", "enabled"), &VehicleElectricEngine::pantographs_valve);
        ClassDB::bind_method(
                D_METHOD("pantographs_drop_all", "enabled"), &VehicleElectricEngine::pantographs_drop_all);
        ClassDB::bind_method(
                D_METHOD("pantograph_compressor", "enabled"), &VehicleElectricEngine::pantograph_compressor);
        ClassDB::bind_method(
                D_METHOD("pantograph_compressor_valve", "to_compressor"),
                &VehicleElectricEngine::pantograph_compressor_valve);
        ClassDB::bind_method(D_METHOD("pantograph", "selector", "enabled"), &VehicleElectricEngine::pantograph);
        ClassDB::bind_method(
                D_METHOD("set_pantograph_wire_voltage", "selector", "voltage"),
                &VehicleElectricEngine::set_pantograph_wire_voltage);

        BIND_ENUM_CONSTANT(PANTOGRAPH_FIRST);
        BIND_ENUM_CONSTANT(PANTOGRAPH_SECOND);
    }

    void VehicleElectricEngine::_declare_state_properties() {
        VehicleEngine::_declare_state_properties();
        state_base_index = get_state_property_count();
        declare_state_property("converter_enabled", Variant::BOOL);
        declare_state_property("converted_allowed", Variant::BOOL);
        declare_state_property("converter_time_to_start", Variant::FLOAT);
        declare_state_property("power_source", Variant::INT);
        declare_state_property("accumulator/recharge_source", Variant::INT);
        declare_state_property("current_collector/max_voltage", Variant::FLOAT);
        declare_state_property("current_collector/max_current", Variant::FLOAT);
        declare_state_property("current_collector/max_collector_lifting", Variant::FLOAT);
        declare_state_property("current_collector/min_collector_lifting", Variant::FLOAT);
        declare_state_property("current_collector/collector_sliding_width", Variant::FLOAT);
        declare_state_property("current_collector/min_main_switch_voltage", Variant::FLOAT);
        declare_state_property("current_collector/min_pantograph_tank_pressure", Variant::FLOAT);
        declare_state_property("current_collector/max_pantograph_tank_pressure", Variant::FLOAT);
        declare_state_property("current_collector/pantograph_tank_pressure", Variant::FLOAT);
        declare_state_property("current_collector/pantograph_pressure_switch_armed", Variant::BOOL);
        declare_state_property("current_collector/pantograph_compressor_valve", Variant::BOOL);
        declare_state_property("current_collector/overvoltage_relay", Variant::BOOL);
        declare_state_property("current_collector/required_main_switch_voltage", Variant::FLOAT);
        declare_state_property("current_collector/valve_active", Variant::BOOL);
        declare_state_property("current_collector/pantographs_dropped", Variant::BOOL);
        declare_state_property("current_collector/pantograph_first_active", Variant::BOOL);
        declare_state_property("current_collector/pantograph_first_voltage", Variant::FLOAT);
        declare_state_property("current_collector/pantograph_second_active", Variant::BOOL);
        declare_state_property("current_collector/pantograph_second_voltage", Variant::FLOAT);
        declare_state_property("current_collector/voltage", Variant::FLOAT);
        declare_state_property("indicators/contactors_active", Variant::BOOL);
        declare_state_property("indicators/diff_relay_active", Variant::BOOL);
        declare_state_property("indicators/resistors_active", Variant::BOOL);
        declare_state_property("indicators/vent_overload_active", Variant::BOOL);
        declare_state_property("indicators/highcurrent_active", Variant::BOOL);
        declare_state_property("indicators/mainbreaker_active", Variant::BOOL);
        declare_state_property("transducer/input_voltage", Variant::FLOAT);
        declare_state_property("power_cable/source", Variant::INT);
        declare_state_property("power_cable/steam_pressure", Variant::FLOAT);
    }

    Variant VehicleElectricEngine::_get_state_property(const int p_local_index) const {
        if (p_local_index < state_base_index) {
            return VehicleEngine::_get_state_property(p_local_index);
        }
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return Variant();
        }
        switch (p_local_index - state_base_index) {
            case STATE_CONVERTER_ENABLED:
                return mover->ConverterFlag;
            case STATE_CONVERTER_ALLOWED:
                return mover->ConverterAllow;
            case STATE_CONVERTER_TIME_TO_START:
                return mover->ConverterStartDelayTimer;
            case STATE_POWER_SOURCE:
                return train_controller_node->tpower_source_map.at(mover->EnginePowerSource.SourceType);
            case STATE_ACCUMULATOR_RECHARGE_SOURCE:
                return mover->EnginePowerSource.SourceType == TPowerSource::Accumulator
                        ? Variant(train_controller_node->tpower_source_map.at(mover->EnginePowerSource.RAccumulator.RechargeSource))
                        : Variant();
            case STATE_CC_MAX_VOLTAGE:
                return mover->EnginePowerSource.MaxVoltage;
            case STATE_CC_MAX_CURRENT:
                return mover->EnginePowerSource.MaxCurrent;
            case STATE_CC_MAX_LIFTING:
                return mover->EnginePowerSource.CollectorParameters.MaxH;
            case STATE_CC_MIN_LIFTING:
                return mover->EnginePowerSource.CollectorParameters.MinH;
            case STATE_CC_SLIDING_WIDTH:
                return mover->EnginePowerSource.CollectorParameters.CSW;
            case STATE_CC_MIN_MAIN_SWITCH_VOLTAGE:
                return mover->EnginePowerSource.CollectorParameters.MinV;
            case STATE_CC_MIN_TANK_PRESSURE:
                return mover->EnginePowerSource.CollectorParameters.MinPress;
            case STATE_CC_MAX_TANK_PRESSURE:
                return mover->EnginePowerSource.CollectorParameters.MaxPress;
            case STATE_CC_TANK_PRESSURE:
                return mover->PantPress;
            case STATE_CC_PRESSURE_SWITCH_ARMED:
                return mover->PantPressSwitchActive;
            case STATE_CC_COMPRESSOR_VALVE:
                return !mover->bPantKurek3;
            case STATE_CC_OVERVOLTAGE_RELAY:
                return mover->EnginePowerSource.CollectorParameters.OVP;
            case STATE_CC_REQUIRED_MAIN_SWITCH_VOLTAGE:
                return mover->EnginePowerSource.CollectorParameters.InsetV;
            case STATE_CC_VALVE_ACTIVE:
                return mover->PantsValve.is_active;
            case STATE_CC_PANTOGRAPHS_DROPPED:
                return mover->PantAllDown;
            case STATE_CC_FIRST_ACTIVE:
                return mover->Pantographs[0].is_active;
            case STATE_CC_FIRST_VOLTAGE:
                return mover->Pantographs[0].voltage;
            case STATE_CC_SECOND_ACTIVE:
                return mover->Pantographs[1].is_active;
            case STATE_CC_SECOND_VOLTAGE:
                return mover->Pantographs[1].voltage;
            case STATE_CC_VOLTAGE:
                return mover->PantographVoltage;
            case STATE_IND_CONTACTORS:
                return (mover->StLinFlag || mover->ControlPressureSwitch) ? false : (mover->BrakePress < 1.0);
            case STATE_IND_DIFF_RELAY:
                return (mover->GroundRelay || mover->ControlPressureSwitch) ? false : (mover->BrakePress < 1.0);
            case STATE_IND_RESISTORS:
                return mover->StLinFlag ? mover->ResistorsFlagCheck() : false;
            case STATE_IND_VENT_OVERLOAD:
                return (mover->RventRot < 5.0) && mover->ResistorsFlagCheck();
            case STATE_IND_HIGHCURRENT:
                return !(mover->Imax < mover->ImaxHi);
            case STATE_IND_MAINBREAKER:
                return mover->Mains;
            case STATE_TRANSDUCER_INPUT_VOLTAGE:
                return mover->EnginePowerSource.Transducer.InputVoltage;
            case STATE_POWER_CABLE_SOURCE:
                return mover->EnginePowerSource.SourceType == TPowerSource::PowerCable
                        ? Variant(train_controller_node->tpower_type_map.at(mover->EnginePowerSource.RPowerCable.PowerTrans))
                        : Variant();
            case STATE_POWER_CABLE_STEAM_PRESSURE:
                return mover->EnginePowerSource.SourceType == TPowerSource::PowerCable ? Variant(mover->EnginePowerSource.RPowerCable.SteamPressure) : Variant();
            default:
                return Variant();
        }
    }

    void VehicleElectricEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        VehicleEngine::_do_update_internal_mover(p_mover);
        // Pantographs[*].voltage/PantFrontVolt/PantRearVolt/PantographVoltage are NOT set here:
        // this only runs when the controller is dirty (effectively once, at startup), but wire
        // voltage changes every frame as the vehicle moves - see set_pantograph_wire_voltage(),
        // which writes them straight to the mover instead.
        p_mover->EnginePowerSource.SourceType = train_controller_node->power_source_map.at(power_source);

        switch (power_source) {
            case VehicleController::POWER_SOURCE_INTERNAL: {
                const std::map<VehicleController::TrainPowerType, TPowerType>::const_iterator lookup =
                        train_controller_node->power_type_map.find(power_cable_source);
                p_mover->EnginePowerSource.PowerType =
                        lookup != train_controller_node->power_type_map.end() ? lookup->second : TPowerType::NoPower;
                break;
            }
            case VehicleController::POWER_SOURCE_TRANSDUCER: {
                p_mover->EnginePowerSource.Transducer.InputVoltage = power_transducer_input_voltage;
                break;
            }
            case VehicleController::POWER_SOURCE_GENERATOR: {
                // engine_revolutions is an uninitialized raw pointer on a fresh TMoverParameters
                // (MOVER.h:551) - nothing currently dereferences EnginePowerSource's copy of it,
                // but HeatingPowerSource's copy does (see VehicleHeating.cpp), so it's pointed at
                // enrot (the vehicle's own engine revolutions counter) here too, defensively.
                engine_generator &generator_params{p_mover->EnginePowerSource.EngineGenerator};
                generator_params.engine_revolutions = &p_mover->enrot;
                break;
            }
            case VehicleController::POWER_SOURCE_ACCUMULATOR: {
                p_mover->EnginePowerSource.RAccumulator.RechargeSource =
                        train_controller_node->power_source_map.at(power_accumulator_recharge_source);
                break;
            }
            case VehicleController::POWER_SOURCE_CURRENTCOLLECTOR: {
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
            case VehicleController::POWER_SOURCE_POWERCABLE: {
                p_mover->EnginePowerSource.RPowerCable.PowerTrans =
                        train_controller_node->power_type_map.at(power_cable_source);
                if (p_mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    p_mover->EnginePowerSource.RPowerCable.SteamPressure = power_cable_steam_pressure;
                }
                break;
            }
            case VehicleController::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case VehicleController::POWER_SOURCE_NOT_DEFINED:;
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

    void VehicleElectricEngine::converter(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->ConverterSwitch(p_enabled);
    }

    void VehicleElectricEngine::compressor(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->CompressorSwitch(p_enabled);
    }

    void VehicleElectricEngine::converter_fuse_reset() {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_converteroverloadrelayreset (Train.cpp:3567-3585) ->
        // RelayReset(relay_t::primaryconverteroverload), "converterfuse_bt:"/ggConverterFuseButton
        // (Train.cpp:10053) - the converter-specific counterpart to fuse_reset()/FuseOn() above.
        mover->RelayReset(Maszyna::primaryconverteroverload);
    }

    void VehicleElectricEngine::pantographs_valve(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->OperatePantographsValve(p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
    }

    // Train.cpp:3336 OnCommand_pantographlowerall
    void VehicleElectricEngine::pantographs_drop_all(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->DropAllPantographs(p_enabled);
    }

    void VehicleElectricEngine::pantograph_compressor(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressoractivate (Train.cpp:2912) - runs while held,
        // starting only with low enough pressure and live 24V power
        if (!p_enabled) {
            mover->PantCompFlag = false;
            return;
        }
        if (mover->PantPress < 4.8 && mover->Power24vIsAvailable) {
            mover->PantCompFlag = true;
        }
    }

    void VehicleElectricEngine::pantograph_compressor_valve(const bool p_to_compressor) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressorvalveenable/disable (Train.cpp:2869-2909)
        mover->bPantKurek3 = !p_to_compressor;
    }

    void VehicleElectricEngine::pantograph(const PantographSelector p_selector, const bool p_enabled) {
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

    void VehicleElectricEngine::set_pantograph_wire_voltage(const PantographSelector p_selector, const float p_voltage) {
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

    void VehicleElectricEngine::_register_commands() {
        VehicleEngine::_register_commands();
        register_command("converter", Callable(this, "converter"));
        register_command("converter_fuse_reset", Callable(this, "converter_fuse_reset"));
        register_command("compressor", Callable(this, "compressor"));
        register_command("pantographs_valve", Callable(this, "pantographs_valve"));
        register_command("pantographs_drop_all", Callable(this, "pantographs_drop_all"));
        register_command("pantograph_compressor", Callable(this, "pantograph_compressor"));
        register_command("pantograph_compressor_valve", Callable(this, "pantograph_compressor_valve"));
        register_command("pantograph", Callable(this, "pantograph"));
    }

    void VehicleElectricEngine::_unregister_commands() {
        VehicleEngine::_unregister_commands();
        unregister_command("converter", Callable(this, "converter"));
        unregister_command("converter_fuse_reset", Callable(this, "converter_fuse_reset"));
        unregister_command("compressor", Callable(this, "compressor"));
        unregister_command("pantographs_valve", Callable(this, "pantographs_valve"));
        unregister_command("pantographs_drop_all", Callable(this, "pantographs_drop_all"));
        unregister_command("pantograph_compressor", Callable(this, "pantograph_compressor"));
        unregister_command("pantograph_compressor_valve", Callable(this, "pantograph_compressor_valve"));
        unregister_command("pantograph", Callable(this, "pantograph"));
    }


    void VehicleElectricEngine::set_power_source(const VehicleController::TrainPowerSource p_source) {
        power_source = p_source;
        dirty = true;
    }

    VehicleController::TrainPowerSource VehicleElectricEngine::get_power_source() const {
        return power_source;
    }
} // namespace godot
