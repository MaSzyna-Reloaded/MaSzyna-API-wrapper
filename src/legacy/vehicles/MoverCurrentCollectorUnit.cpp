#include "MoverCurrentCollectorUnit.hpp"
#include "legacy/vehicles/MoverBackend.hpp"
#include "legacy/vehicles/MoverTypes.hpp"
#include "vehicles/rail/RailVehicleController.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"
#include <unordered_map>

namespace godot {
    namespace {
        // DynObj.cpp:3897 - the meter counts kWh
        constexpr double JOULES_PER_KWH = 3600000.0;
    } // namespace

    static const std::unordered_map<RailVehicleElectricEngine::ValveOperation, Maszyna::operation_t> &
    valve_operations() {
        static const std::unordered_map<RailVehicleElectricEngine::ValveOperation, Maszyna::operation_t> operations = {
                {RailVehicleElectricEngine::VALVE_OPERATION_NONE, Maszyna::operation_t::none},
                {RailVehicleElectricEngine::VALVE_OPERATION_ENABLE, Maszyna::operation_t::enable},
                {RailVehicleElectricEngine::VALVE_OPERATION_DISABLE, Maszyna::operation_t::disable},
                {RailVehicleElectricEngine::VALVE_OPERATION_ENABLE_ON, Maszyna::operation_t::enable_on},
                {RailVehicleElectricEngine::VALVE_OPERATION_ENABLE_OFF, Maszyna::operation_t::enable_off},
                {RailVehicleElectricEngine::VALVE_OPERATION_DISABLE_ON, Maszyna::operation_t::disable_on},
                {RailVehicleElectricEngine::VALVE_OPERATION_DISABLE_OFF, Maszyna::operation_t::disable_off},
        };
        return operations;
    }

    double MoverCurrentCollectorUnit::get_max_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.MaxVoltage : 0.0;
    }

    double MoverCurrentCollectorUnit::get_max_current() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.MaxCurrent : 0.0;
    }

    double MoverCurrentCollectorUnit::get_max_lifting() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MaxH : 0.0;
    }

    double MoverCurrentCollectorUnit::get_min_lifting() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MinH : 0.0;
    }

    double MoverCurrentCollectorUnit::get_sliding_width() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.CSW : 0.0;
    }

    double MoverCurrentCollectorUnit::get_min_main_switch_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MinV : 0.0;
    }

    double MoverCurrentCollectorUnit::get_min_pantograph_tank_pressure() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MinPress : 0.0;
    }

    double MoverCurrentCollectorUnit::get_max_pantograph_tank_pressure() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.MaxPress : 0.0;
    }

    double MoverCurrentCollectorUnit::get_pantograph_tank_pressure() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantPress : 0.0;
    }

    bool MoverCurrentCollectorUnit::get_pantograph_pressure_switch_armed() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantPressSwitchActive : false;
    }

    bool MoverCurrentCollectorUnit::get_pantograph_compressor_valve() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? !p_mover->bPantKurek3 : false;
    }

    bool MoverCurrentCollectorUnit::get_pantograph_compressor_enabled() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantCompFlag : false;
    }

    bool MoverCurrentCollectorUnit::get_overvoltage_relay() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.OVP : false;
    }

    double MoverCurrentCollectorUnit::get_required_main_switch_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.CollectorParameters.InsetV : 0.0;
    }

    bool MoverCurrentCollectorUnit::get_valve_active() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantsValve.is_active : false;
    }

    bool MoverCurrentCollectorUnit::get_valve_enabled() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantsValve.is_enabled : false;
    }

    bool MoverCurrentCollectorUnit::get_pantographs_dropped() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantAllDown : false;
    }

    bool MoverCurrentCollectorUnit::get_pantograph_first_active() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->Pantographs[0].is_active : false;
    }

    double MoverCurrentCollectorUnit::get_pantograph_first_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->Pantographs[0].voltage : 0.0;
    }

    bool MoverCurrentCollectorUnit::get_pantograph_second_active() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->Pantographs[1].is_active : false;
    }

    double MoverCurrentCollectorUnit::get_pantograph_second_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->Pantographs[1].voltage : 0.0;
    }

    double MoverCurrentCollectorUnit::get_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->PantographVoltage : 0.0;
    }

    double MoverCurrentCollectorUnit::get_trainset_high_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->GetTrainsetHighVoltage() : 0.0;
    }

    double MoverCurrentCollectorUnit::get_energy_drawn() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnergyMeter.first : 0.0;
    }

    double MoverCurrentCollectorUnit::get_energy_returned() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnergyMeter.second : 0.0;
    }

    // Original engine: DynObj.cpp:3798-3832 (the current through each pantograph on the wire) and
    // 3897, 3942 (EnergyMeter). Each pantograph is counted with its own voltage and its own
    // is_active; the original swaps the front and rear voltages and asks the front pantograph's
    // is_active for both, which for two raised pantographs comes to the same sum.
    void MoverCurrentCollectorUnit::meter_energy(const double p_delta) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        if (mover->EnginePowerSource.SourceType != Maszyna::TPowerSource::CurrentCollector) {
            return;
        }
        const double current = ((mover->DynamicBrakeFlag && mover->ResistorsFlag)
                                        ? 0.0
                                        : std::abs(mover->Itot) * mover->IsVehicleEIMBrakingFactor()) +
                               mover->TotalCurrent;
        // PantFrontVolt/PantRearVolt are zero for a pantograph that is not active or not on a wire
        // (set_pantograph_wire_voltage())
        const int active_pantographs = (mover->PantFrontVolt > 0.0 ? 1 : 0) + (mover->PantRearVolt > 0.0 ? 1 : 0);
        const double pantograph_current = current / std::max(1, active_pantographs);
        const double energy =
                (mover->PantFrontVolt + mover->PantRearVolt) * pantograph_current * p_delta / JOULES_PER_KWH;
        (pantograph_current > 0.0 ? mover->EnergyMeter.first : mover->EnergyMeter.second) += energy;
    }

    double MoverCurrentCollectorUnit::get_transducer_input_voltage() const {
        TMoverParameters *p_mover = owner.get_mover();
        return p_mover != nullptr ? p_mover->EnginePowerSource.Transducer.InputVoltage : 0.0;
    }

    void MoverCurrentCollectorUnit::pantographs_valve(const bool p_enabled) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        mover->OperatePantographsValve(p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
    }

    // Train.cpp:3456-3510 OnCommand_pantographraiseselected/lowerselected - the selected
    // pantographs' master valve
    void MoverCurrentCollectorUnit::pantographs_valve_operate(
            const RailVehicleElectricEngine::ValveOperation p_operation) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        mover->OperatePantographsValve(valve_operations().at(p_operation));
    }

    // Train.cpp:3336 OnCommand_pantographlowerall
    void MoverCurrentCollectorUnit::pantographs_drop_all(const bool p_enabled) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        mover->DropAllPantographs(p_enabled);
    }

    void MoverCurrentCollectorUnit::pantograph_compressor(const bool p_enabled) const {
        TMoverParameters *mover = owner.get_mover();
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

    void MoverCurrentCollectorUnit::pantograph_compressor_valve(const bool p_to_compressor) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        // Original engine: OnCommand_pantographcompressorvalveenable/disable (Train.cpp:2869-2909)
        mover->bPantKurek3 = !p_to_compressor;
    }

    bool MoverCurrentCollectorUnit::get_pantograph_valve_enabled(
            const RailVehicleElectricEngine::PantographSelector p_selector) const {
        const TMoverParameters *mover = owner.get_mover();
        const Maszyna::end end =
                p_selector == RailVehicleElectricEngine::PANTOGRAPH_FIRST ? Maszyna::end::front : Maszyna::end::rear;
        return mover != nullptr ? mover->Pantographs[end].valve.is_enabled : false;
    }

    // Train.cpp:3218-3300 OnCommand_pantographraisefront/lowerfront and their rear twins
    void MoverCurrentCollectorUnit::pantograph_valve_operate(
            const RailVehicleElectricEngine::PantographSelector p_selector,
            const RailVehicleElectricEngine::ValveOperation p_operation) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        const Maszyna::end end =
                p_selector == RailVehicleElectricEngine::PANTOGRAPH_FIRST ? Maszyna::end::front : Maszyna::end::rear;
        mover->OperatePantographValve(end, valve_operations().at(p_operation));
    }

    void MoverCurrentCollectorUnit::pantograph(
            const RailVehicleElectricEngine::PantographSelector p_selector, const bool p_enabled) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        const Maszyna::end end =
                (p_selector == RailVehicleElectricEngine::PANTOGRAPH_FIRST) ? Maszyna::end::front : Maszyna::end::rear;
        mover->OperatePantographValve(end, p_enabled ? Maszyna::operation_t::enable : Maszyna::operation_t::disable);
    }

    void MoverCurrentCollectorUnit::set_pantograph_wire_voltage(
            const RailVehicleElectricEngine::PantographSelector p_selector, const float p_voltage) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        // Written straight to the mover, like the other per-frame-relevant setters above
        // (pantograph(), pantographs_valve()) - _apply_configuration() only runs when the
        // controller is dirty (effectively once, at startup), so stashing this in a member for
        // that path to pick up later would mean every subsequent frame's wire voltage is ignored.
        if (p_selector == RailVehicleElectricEngine::PANTOGRAPH_FIRST) {
            mover->Pantographs[0].voltage = p_voltage;
            mover->PantFrontVolt = mover->Pantographs[0].is_active ? p_voltage : 0.0;
        } else {
            mover->Pantographs[1].voltage = p_voltage;
            mover->PantRearVolt = mover->Pantographs[1].is_active ? p_voltage : 0.0;
        }
    }

    void MoverCurrentCollectorUnit::set_voltage(const float p_voltage) const {
        TMoverParameters *mover = owner.get_mover();
        ASSERT_MOVER(mover);
        mover->PantographVoltage = p_voltage;
    }

    void MoverCurrentCollectorUnit::apply_configuration(const RailVehicleElectricEngine *p_engine) const {
        TMoverParameters *p_mover = owner.get_mover();
        // Pantographs[*].voltage/PantFrontVolt/PantRearVolt/PantographVoltage are NOT set here:
        // this only runs when the controller is dirty (effectively once, at startup), but wire
        // voltage changes every frame as the vehicle moves - see set_pantograph_wire_voltage(),
        // which writes them straight to the p_mover instead.
        p_mover->EnginePowerSource.SourceType = mover_power_source(p_engine->get_power_source());

        switch (p_engine->get_power_source()) {
            case RailVehicleController::POWER_SOURCE_INTERNAL: {
                p_mover->EnginePowerSource.PowerType = mover_power_type(p_engine->get_power_cable_source());
                break;
            }
            case RailVehicleController::POWER_SOURCE_TRANSDUCER: {
                p_mover->EnginePowerSource.Transducer.InputVoltage = p_engine->get_power_transducer_input_voltage();
                break;
            }
            case RailVehicleController::POWER_SOURCE_GENERATOR: {
                // engine_revolutions is an uninitialized raw pointer on a fresh TMoverParameters
                // (MOVER.h:551) - nothing currently dereferences EnginePowerSource's copy of it,
                // but HeatingPowerSource's copy does (see RailVehicleHeating.cpp), so it's pointed at
                // enrot (the vehicle's own engine revolutions counter) here too, defensively.
                engine_generator &generator_params{p_mover->EnginePowerSource.EngineGenerator};
                generator_params.engine_revolutions = &p_mover->enrot;
                break;
            }
            case RailVehicleController::POWER_SOURCE_ACCUMULATOR: {
                p_mover->EnginePowerSource.RAccumulator.RechargeSource =
                        mover_power_source(p_engine->get_power_accumulator_recharge_source());
                break;
            }
            case RailVehicleController::POWER_SOURCE_CURRENTCOLLECTOR: {
                p_mover->EnginePowerSource.CollectorParameters.MinH =
                        p_engine->get_power_current_collector_min_collector_lifting();
                p_mover->EnginePowerSource.CollectorParameters.MaxH =
                        p_engine->get_power_current_collector_max_collector_lifting();
                p_mover->EnginePowerSource.CollectorParameters.CSW =
                        p_engine->get_power_current_collector_sliding_width();
                // Mover.cpp:11622 - MaxVoltage is also the collector's own limit; left at 0, an induction
                // motor opens the line breaker above MaxV + 200 V (Mover.cpp:5706) the moment it closes
                p_mover->EnginePowerSource.CollectorParameters.MaxV =
                        p_engine->get_power_current_collector_max_voltage();
                p_mover->EnginePowerSource.CollectorParameters.MinV =
                        p_engine->get_power_current_collector_min_main_switch_voltage();
                p_mover->EnginePowerSource.CollectorParameters.MinPress =
                        p_engine->get_power_current_collector_min_pantograph_tank_pressure();
                p_mover->EnginePowerSource.CollectorParameters.MaxPress =
                        p_engine->get_power_current_collector_max_pantograph_tank_pressure();
                p_mover->EnginePowerSource.CollectorParameters.OVP =
                        p_engine->get_power_current_collector_overvoltage_relay();
                p_mover->EnginePowerSource.CollectorParameters.CollectorsNo =
                        p_engine->get_power_current_collector_number_of_collectors();
                p_mover->EnginePowerSource.MaxVoltage = p_engine->get_power_current_collector_max_voltage();
                p_mover->EnginePowerSource.MaxCurrent = p_engine->get_power_current_collector_max_current();
                p_mover->EnginePowerSource.CollectorParameters.InsetV =
                        p_engine->get_power_current_collector_required_main_switch_voltage();
                p_mover->EnginePowerSource.CollectorParameters.PhysicalLayout =
                        p_engine->get_power_current_collector_physical_layout();
                break;
            }
            case RailVehicleController::POWER_SOURCE_POWERCABLE: {
                p_mover->EnginePowerSource.RPowerCable.PowerTrans =
                        mover_power_type(p_engine->get_power_cable_source());
                if (p_mover->EnginePowerSource.RPowerCable.PowerTrans == TPowerType::SteamPower) {
                    p_mover->EnginePowerSource.RPowerCable.SteamPressure = p_engine->get_power_cable_steam_pressure();
                }
                break;
            }
            case RailVehicleController::POWER_SOURCE_HEATER:; // Not finished on MaSzyna's side
            case RailVehicleController::POWER_SOURCE_NOT_DEFINED:;
            default:;
        }

        p_mover->PantographCompressorStart = mover_start_mode(p_engine->get_cntrl_pantograph_compressor_start_mode());
        p_mover->PantAutoValve = p_engine->get_cntrl_pantograph_auto_valve();
        // LoadFIZ_Cntrl (Mover.cpp:10927-10946) - the master valve and each pantograph's own
        p_mover->PantsValve.start_type = mover_start_mode(p_engine->get_cntrl_pantographs_valve_start_mode());
        p_mover->PantsValve.spring = p_engine->get_cntrl_pantographs_valve_spring();
        for (auto &pantograph: p_mover->Pantographs) {
            pantograph.valve.start_type = mover_start_mode(p_engine->get_cntrl_pantograph_valve_start_mode());
            pantograph.valve.spring = p_engine->get_cntrl_pantograph_valve_spring();
            pantograph.valve.solenoid = p_engine->get_cntrl_pantograph_valve_solenoid();
        }
    }
} // namespace godot
