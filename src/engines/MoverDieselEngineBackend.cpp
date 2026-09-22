#include "MoverDieselEngineBackend.hpp"
#include "../core/VehicleController.hpp"

namespace godot {
    double MoverDieselEngineBackend::get_rpm(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->EngineRPMRatio() * p_mover->EngineMaxRPM() : 0.0;
    }

    bool MoverDieselEngineBackend::get_oil_pump_active(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->OilPump.is_active : false;
    }

    bool MoverDieselEngineBackend::get_oil_pump_disabled(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->OilPump.is_disabled : false;
    }

    double MoverDieselEngineBackend::get_oil_pump_pressure(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->OilPump.pressure : 0.0;
    }

    bool MoverDieselEngineBackend::get_fuel_pump_active(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->FuelPump.is_active : false;
    }

    bool MoverDieselEngineBackend::get_fuel_pump_disabled(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->FuelPump.is_disabled : false;
    }

    bool MoverDieselEngineBackend::get_startup(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->dizel_startup : false;
    }

    bool MoverDieselEngineBackend::get_ignition(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->dizel_ignition : false;
    }

    bool MoverDieselEngineBackend::get_spinup(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->dizel_spinup : false;
    }

    double MoverDieselEngineBackend::get_output_power(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->dizel_Power : 0.0;
    }

    double MoverDieselEngineBackend::get_torque(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->dizel_Torque : 0.0;
    }

    double MoverDieselEngineBackend::get_fill(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->dizel_fill : 0.0;
    }

    double MoverDieselEngineBackend::get_max_rpm(const TMoverParameters *p_mover) const {
                return p_mover != nullptr ? p_mover->EngineMaxRPM() : 0.0;
    }

    void MoverDieselEngineBackend::update_mover(const VehicleDieselEngine *p_engine, TMoverParameters *p_mover) const {

        // FIXME: test data
        p_mover->EnginePowerSource.SourceType = TPowerSource::Accumulator;
        // end test data

        p_mover->OilPump.pressure_minimum = p_engine->get_oil_pump_pressure_minimum();
        p_mover->OilPump.pressure_maximum = p_engine->get_oil_pump_pressure_maximum();
        p_mover->FuelPump.start_type = p_engine->start_mode_map.at(p_engine->get_fuel_pump_start_mode());
        p_mover->OilPump.start_type = p_engine->start_mode_map.at(p_engine->get_oil_pump_start_mode());
        p_mover->WaterPump.start_type = p_engine->start_mode_map.at(p_engine->get_water_pump_start_mode());

        p_mover->dizel_nmin = p_engine->get_mechanical_min_rpm();
        p_mover->dizel_nmax = p_engine->get_mechanical_max_rpm();
        p_mover->dizel_nmax_cutoff = p_engine->get_mechanical_fuel_cutoff_rpm();
        p_mover->dizel_AIM = p_engine->get_mechanical_inertia();
        p_mover->engageupspeed = p_engine->get_mechanical_clutch_engage_speed();
        p_mover->engagedownspeed = p_engine->get_mechanical_clutch_disengage_speed();

        p_mover->hydro_TC = p_engine->get_torque_converter_present();
        p_mover->hydro_TC_TMMax = p_engine->get_torque_converter_max_torque_ratio();
        p_mover->hydro_TC_CouplingPoint = p_engine->get_torque_converter_coupling_point();
        p_mover->hydro_TC_LockupTorque = p_engine->get_torque_converter_lockup_torque();
        p_mover->hydro_TC_LockupRate = p_engine->get_torque_converter_lockup_rate();
        p_mover->hydro_TC_UnlockRate = p_engine->get_torque_converter_unlock_rate();
        p_mover->hydro_TC_FillRateInc = p_engine->get_torque_converter_fill_rate_increase();
        p_mover->hydro_TC_FillRateDec = p_engine->get_torque_converter_fill_rate_decrease();
        p_mover->hydro_TC_TorqueInIn = p_engine->get_torque_converter_torque_in_in();
        p_mover->hydro_TC_TorqueInOut = p_engine->get_torque_converter_torque_in_out();
        p_mover->hydro_TC_TorqueOutOut = p_engine->get_torque_converter_torque_out_out();
        p_mover->hydro_TC_LockupSpeed = p_engine->get_torque_converter_lockup_speed();
        p_mover->hydro_TC_UnlockSpeed = p_engine->get_torque_converter_unlock_speed();

        p_mover->hydro_TC_Table.clear();
        for (int i = 0; i < p_engine->get_torque_converter_table().size(); i++) {
            const Ref<CurvePointItem> &row = p_engine->get_torque_converter_table()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: p_engine->get_torque_converter_table() property is null at index " + String::num(i));
                continue;
            }
            p_mover->hydro_TC_Table.emplace(row->get_x(), row->get_y());
        }

        p_mover->dizel_vel2nmax_Table.clear();
        for (int i = 0; i < p_engine->get_vel2nmax_table().size(); i++) {
            const Ref<CurvePointItem> &row = p_engine->get_vel2nmax_table()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: p_engine->get_vel2nmax_table() property is null at index " + String::num(i));
                continue;
            }
            // matches readV2NMAXList (Mover.cpp:8476-8489): x unconverted, y (rpm) -> rev/s
            p_mover->dizel_vel2nmax_Table.emplace(row->get_x(), row->get_y() / 60.0);
        }

        p_mover->hydro_R = p_engine->get_retarder_present();
        p_mover->hydro_R_Placement = p_engine->get_retarder_placement();
        p_mover->hydro_R_TorqueInIn = p_engine->get_retarder_torque_in_in();
        p_mover->hydro_R_MaxTorque = p_engine->get_retarder_max_torque();
        p_mover->hydro_R_MaxPower = p_engine->get_retarder_max_power();
        p_mover->hydro_R_FillRateInc = p_engine->get_retarder_fill_rate_increase();
        p_mover->hydro_R_FillRateDec = p_engine->get_retarder_fill_rate_decrease();
        p_mover->hydro_R_MinVel = p_engine->get_retarder_min_velocity();

        /* DList: tabela przepustnicy */
        p_mover->dizel_Mmax = p_engine->get_throttle_table_max_torque();
        p_mover->dizel_nMmax = p_engine->get_throttle_table_max_torque_rpm();
        p_mover->dizel_Mnmax = p_engine->get_throttle_table_max_rpm_torque();
        p_mover->dizel_nominalfill = p_engine->get_throttle_table_nominal_fuel_dose();
        p_mover->dizel_Mstand = p_engine->get_throttle_table_resistance_torque();
        p_mover->dizel_NominalFuelConsumptionRate = p_engine->get_throttle_table_nominal_fuel_consumption_rate();

        constexpr int MAX_THROTTLE_TABLE = Maszyna::ResArraySize + 1;
        const int throttle_table_size = static_cast<int>(p_engine->get_throttle_table_positions().size());
        if (throttle_table_size > MAX_THROTTLE_TABLE) {
            UtilityFunctions::push_warning(
                    "[VehicleDieselEngine]: p_engine->get_throttle_table_positions() has " + String::num_int64(throttle_table_size) +
                    " entries, exceeding the p_mover's limit of " + String::num_int64(MAX_THROTTLE_TABLE) +
                    "; truncating.");
        }
        for (int i = 0; i < std::min(MAX_THROTTLE_TABLE, throttle_table_size); i++) {
            const Ref<ThrottlePositionItem> &row = p_engine->get_throttle_table_positions()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: p_engine->get_throttle_table_positions() property is null at index " + String::num(i));
                continue;
            }
            p_mover->RList[i].Relay = row->get_throttle_position();
            p_mover->RList[i].R = row->get_fuel_dose();
            p_mover->RList[i].Mn = row->get_clutch_behavior();
        }

        /* DMList: charakterystyka momentu obrotowego silnika spalinowego */
        p_mover->dizel_Momentum_Table.clear();
        for (int i = 0; i < p_engine->get_torque_table().size(); i++) {
            const Ref<CurvePointItem> &row = p_engine->get_torque_table()[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: p_engine->get_torque_table() property is null at index " + String::num(i));
                continue;
            }
            p_mover->dizel_Momentum_Table.emplace(row->get_x() / 60.0, row->get_y());
        }
    }

    void MoverDieselEngineBackend::fill_config(const VehicleDieselEngine *p_engine, const TMoverParameters *p_mover, Dictionary &p_config) const {
                if (p_mover == nullptr) {
            return;
        }
        p_config["engine_shake_enabled"] = true;
    }
} // namespace godot
