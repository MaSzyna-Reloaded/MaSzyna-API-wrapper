#include "MoverElectricEngineBackend.hpp"
#include "VehicleElectricEngine.hpp"
#include "../mover/MoverBackend.hpp"
#include "../mover/MoverTypes.hpp"
#include "../core/VehicleController.hpp"

namespace godot {
    bool MoverElectricEngineBackend::get_converter_enabled(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->ConverterFlag : false;
    }

    bool MoverElectricEngineBackend::get_converted_allowed(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->ConverterAllow : false;
    }

    double MoverElectricEngineBackend::get_converter_time_to_start(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->ConverterStartDelayTimer : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_max_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.MaxVoltage : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_max_current(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.MaxCurrent : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_max_lifting(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MaxH : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_min_lifting(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MinH : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_sliding_width(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.CSW : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_min_main_switch_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MinV : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_min_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MinPress : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_max_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MaxPress : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_pantograph_tank_pressure(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->PantPress : 0.0;
    }

    bool MoverElectricEngineBackend::get_collector_pantograph_pressure_switch_armed(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->PantPressSwitchActive : false;
    }

    bool MoverElectricEngineBackend::get_collector_pantograph_compressor_valve(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? !p_mover->bPantKurek3 : false;
    }

    bool MoverElectricEngineBackend::get_collector_overvoltage_relay(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.OVP : false;
    }

    double MoverElectricEngineBackend::get_collector_required_main_switch_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.InsetV : 0.0;
    }

    bool MoverElectricEngineBackend::get_collector_valve_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->PantsValve.is_active : false;
    }

    bool MoverElectricEngineBackend::get_collector_pantographs_dropped(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->PantAllDown : false;
    }

    bool MoverElectricEngineBackend::get_collector_pantograph_first_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->Pantographs[0].is_active : false;
    }

    double MoverElectricEngineBackend::get_collector_pantograph_first_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->Pantographs[0].voltage : 0.0;
    }

    bool MoverElectricEngineBackend::get_collector_pantograph_second_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->Pantographs[1].is_active : false;
    }

    double MoverElectricEngineBackend::get_collector_pantograph_second_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->Pantographs[1].voltage : 0.0;
    }

    double MoverElectricEngineBackend::get_collector_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->PantographVoltage : 0.0;
    }

    bool MoverElectricEngineBackend::get_contactors_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? (p_mover->StLinFlag || p_mover->ControlPressureSwitch) ? false : (p_mover->BrakePress < 1.0) : false;
    }

    bool MoverElectricEngineBackend::get_diff_relay_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? (p_mover->GroundRelay || p_mover->ControlPressureSwitch) ? false : (p_mover->BrakePress < 1.0) : false;
    }

    bool MoverElectricEngineBackend::get_resistors_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->StLinFlag ? p_mover->ResistorsFlagCheck() : false : false;
    }

    bool MoverElectricEngineBackend::get_vent_overload_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? (p_mover->RventRot < 5.0) && p_mover->ResistorsFlagCheck() : false;
    }

    bool MoverElectricEngineBackend::get_highcurrent_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? !(p_mover->Imax < p_mover->ImaxHi) : false;
    }

    bool MoverElectricEngineBackend::get_mainbreaker_active(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->Mains : false;
    }

    double MoverElectricEngineBackend::get_transducer_input_voltage(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->EnginePowerSource.Transducer.InputVoltage : 0.0;
    }

    bool MoverElectricEngineBackend::get_camshaft_available(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->HasCamshaft : false;
    }

    bool MoverElectricEngineBackend::get_converter_overload(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->ConvOvldFlag : false;
    }

    double MoverElectricEngineBackend::get_line_breaker_delay(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->CtrlDelay : 0.0;
    }

    double MoverElectricEngineBackend::get_line_breaker_initial_delay(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->InitialCtrlDelay : 0.0;
    }

    bool MoverElectricEngineBackend::get_line_breaker_closes_at_no_power(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
                return p_mover != nullptr ? p_mover->LineBreakerClosesOnlyAtNoPowerPos : false;
    }

    void MoverElectricEngineBackend::apply_configuration(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = mover_of(p_engine);
        // Pantographs[*].voltage/PantFrontVolt/PantRearVolt/PantographVoltage are NOT set here:
        // this only runs when the controller is dirty (effectively once, at startup), but wire
        // voltage changes every frame as the vehicle moves - see set_pantograph_wire_voltage(),
        // which writes them straight to the p_mover instead.
        p_mover->EnginePowerSource.SourceType = mover_power_source(p_engine->get_power_source());

        switch (p_engine->get_power_source()) {
            case VehicleController::POWER_SOURCE_INTERNAL: {
                p_mover->EnginePowerSource.PowerType = mover_power_type(p_engine->get_power_cable_source());
                break;
            }
            case VehicleController::POWER_SOURCE_TRANSDUCER: {
                p_mover->EnginePowerSource.Transducer.InputVoltage = p_engine->get_power_transducer_input_voltage();
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
                        mover_power_source(p_engine->get_power_accumulator_recharge_source());
                break;
            }
            case VehicleController::POWER_SOURCE_CURRENTCOLLECTOR: {
                p_mover->EnginePowerSource.CollectorParameters.MinH = p_engine->get_power_current_collector_min_collector_lifting();
                p_mover->EnginePowerSource.CollectorParameters.MaxH = p_engine->get_power_current_collector_max_collector_lifting();
                p_mover->EnginePowerSource.CollectorParameters.CSW = p_engine->get_power_current_collector_sliding_width();
                // Mover.cpp:11622 - MaxVoltage is also the collector's own limit; left at 0, an induction
                // motor opens the line breaker above MaxV + 200 V (Mover.cpp:5706) the moment it closes
                p_mover->EnginePowerSource.CollectorParameters.MaxV = p_engine->get_power_current_collector_max_voltage();
                p_mover->EnginePowerSource.CollectorParameters.MinV = p_engine->get_power_current_collector_min_main_switch_voltage();
                p_mover->EnginePowerSource.CollectorParameters.MinPress =
                        p_engine->get_power_current_collector_min_pantograph_tank_pressure();
                p_mover->EnginePowerSource.CollectorParameters.MaxPress =
                        p_engine->get_power_current_collector_max_pantograph_tank_pressure();
                p_mover->EnginePowerSource.CollectorParameters.OVP = p_engine->get_power_current_collector_overvoltage_relay();
                p_mover->EnginePowerSource.CollectorParameters.CollectorsNo =
                        p_engine->get_power_current_collector_number_of_collectors();
                p_mover->EnginePowerSource.MaxVoltage = p_engine->get_power_current_collector_max_voltage();
                p_mover->EnginePowerSource.MaxCurrent = p_engine->get_power_current_collector_max_current();
                p_mover->EnginePowerSource.CollectorParameters.InsetV =
                        p_engine->get_power_current_collector_required_main_switch_voltage();
                p_mover->EnginePowerSource.CollectorParameters.PhysicalLayout = p_engine->get_power_current_collector_physical_layout();
                break;
            }
            case VehicleController::POWER_SOURCE_POWERCABLE: {
                p_mover->EnginePowerSource.RPowerCable.PowerTrans =
                        mover_power_type(p_engine->get_power_cable_source());
                if (p_mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    p_mover->EnginePowerSource.RPowerCable.SteamPressure = p_engine->get_power_cable_steam_pressure();
                }
                break;
            }
            case VehicleController::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case VehicleController::POWER_SOURCE_NOT_DEFINED:;
            default:;
        }

        /* Circuit: (elektryczny obwod napedowy), tylko pojazdy elektryczne i spalinowo-elektryczne */
        p_mover->CircuitRes = p_engine->get_circuit_resistance();
        p_mover->ImaxLo = p_engine->get_circuit_imax_low();
        p_mover->ImaxHi = p_engine->get_circuit_imax_high();
        p_mover->IminLo = p_engine->get_circuit_imin_low();
        p_mover->IminHi = p_engine->get_circuit_imin_high();
        p_mover->TUHEX_Sum = p_engine->get_circuit_tuhex_sum();
        p_mover->TUHEX_Diff = p_engine->get_circuit_tuhex_diff();
        p_mover->TUHEX_MinIw = p_engine->get_circuit_tuhex_min_current();
        p_mover->TUHEX_MaxIw = p_engine->get_circuit_tuhex_max_current();
        p_mover->TUHEX_Stages = p_engine->get_circuit_tuhex_stages();
        p_mover->TUHEX_Sum1 = p_engine->get_circuit_tuhex_sum_1();
        p_mover->TUHEX_Sum2 = p_engine->get_circuit_tuhex_sum_2();
        p_mover->TUHEX_Sum3 = p_engine->get_circuit_tuhex_sum_3();

        p_mover->ConverterStart = mover_start_mode(p_engine->get_cntrl_converter_start_mode());
        p_mover->ConverterStartDelay = static_cast<float>(p_engine->get_cntrl_converter_start_delay());
        p_mover->ConverterOverloadRelayStart = mover_start_mode(p_engine->get_cntrl_converter_overload_relay_start_mode());
        p_mover->ConverterOverloadRelayOffWhenMainIsOff = p_engine->get_cntrl_converter_overload_relay_off_when_main_is_off();
        p_mover->PantographCompressorStart = mover_start_mode(p_engine->get_cntrl_pantograph_compressor_start_mode());
        p_mover->PantAutoValve = p_engine->get_cntrl_pantograph_auto_valve();
        p_mover->MainsStart = mover_start_mode(p_engine->get_cntrl_main_switch_start_mode());
    }

    void MoverElectricEngineBackend::converter(const VehicleElectricEngine *p_engine, const bool p_enabled) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        mover->ConverterSwitch(p_enabled);
    }

    void MoverElectricEngineBackend::compressor(const VehicleElectricEngine *p_engine, const bool p_enabled) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        mover->CompressorSwitch(p_enabled);
    }

    void MoverElectricEngineBackend::converter_fuse_reset(const VehicleElectricEngine *p_engine) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_converteroverloadrelayreset (Train.cpp:3567-3585) ->
        // RelayReset(relay_t::primaryconverteroverload), "converterfuse_bt:"/ggConverterFuseButton
        // (Train.cpp:10053) - the converter-specific counterpart to fuse_reset()/FuseOn() above.
        mover->RelayReset(Maszyna::primaryconverteroverload);
    }

    void MoverElectricEngineBackend::pantographs_valve(const VehicleElectricEngine *p_engine, const bool p_enabled) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        mover->OperatePantographsValve(p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
    }

    // Train.cpp:3336 OnCommand_pantographlowerall
    void MoverElectricEngineBackend::pantographs_drop_all(const VehicleElectricEngine *p_engine, const bool p_enabled) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        mover->DropAllPantographs(p_enabled);
    }

    void MoverElectricEngineBackend::pantograph_compressor(const VehicleElectricEngine *p_engine, const bool p_enabled) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressoractivate (Train.cpp:2912) - runs while held,
        // starting only with low enough pressure and live 24V power
        if (!p_enabled) {
            mover->PantCompFlag = false;
            return;
        }
        if (mover->PantPress < PANTOGRAPH_COMPRESSOR_START_PRESSURE && mover->Power24vIsAvailable) {
            mover->PantCompFlag = true;
        }
    }

    void MoverElectricEngineBackend::pantograph_compressor_valve(const VehicleElectricEngine *p_engine, const bool p_to_compressor) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressorvalveenable/disable (Train.cpp:2869-2909)
        mover->bPantKurek3 = !p_to_compressor;
    }

    void MoverElectricEngineBackend::pantograph(
            const VehicleElectricEngine *p_engine, const VehicleElectricEngine::PantographSelector p_selector,
            const bool p_enabled) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        const Maszyna::end end = (p_selector == VehicleElectricEngine::PANTOGRAPH_FIRST) ? Maszyna::end::front : Maszyna::end::rear;
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

    void MoverElectricEngineBackend::set_pantograph_wire_voltage(
            const VehicleElectricEngine *p_engine, const VehicleElectricEngine::PantographSelector p_selector,
            const float p_voltage) const {
        TMoverParameters *mover = mover_of(p_engine);
        ASSERT_MOVER(mover);
        // Written straight to the mover, like the other per-frame-relevant setters above
        // (pantograph(), pantographs_valve()) - _apply_configuration() only runs when the
        // controller is dirty (effectively once, at startup), so stashing this in a member for
        // that path to pick up later would mean every subsequent frame's wire voltage is ignored.
        if (p_selector == VehicleElectricEngine::PANTOGRAPH_FIRST) {
            mover->Pantographs[0].voltage = p_voltage;
            mover->PantFrontVolt = mover->Pantographs[0].is_active ? p_voltage : 0.0;
        } else {
            mover->Pantographs[1].voltage = p_voltage;
            mover->PantRearVolt = mover->Pantographs[1].is_active ? p_voltage : 0.0;
        }
        mover->PantographVoltage = std::max(std::fabs(mover->PantFrontVolt), std::fabs(mover->PantRearVolt));
    }
} // namespace godot
