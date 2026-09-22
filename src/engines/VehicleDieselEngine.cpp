#include "VehicleDieselEngine.hpp"
#include "macros.hpp"

#include <algorithm>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleDieselEngine::_bind_methods() {
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, oil_pump_pressure_minimum, "oil_pump");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, oil_pump_pressure_maximum, "oil_pump");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, fuel_pump_start_mode, "fuel_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, oil_pump_start_mode, "oil_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, water_pump_start_mode, "water_pump", PROPERTY_HINT_ENUM,
                "Disabled,Manual,Automatic,ManualWithAutoFallback,Converter,Battery,Direction");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_min_rpm, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_max_rpm, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_fuel_cutoff_rpm, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_inertia, "mechanical");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_clutch_engage_speed, "mechanical/clutch");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, mechanical_clutch_disengage_speed, "mechanical/clutch");
        BIND_PROPERTY(VehicleDieselEngine, Variant::BOOL, torque_converter_present, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_max_torque_ratio, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_coupling_point, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_torque, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_rate, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_rate, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_fill_rate_increase, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_fill_rate_decrease, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_torque_in_in, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_torque_in_out, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_torque_out_out, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_lockup_speed, "torque_converter");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, torque_converter_unlock_speed, "torque_converter");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, torque_converter_table, "torque_converter",
                PROPERTY_HINT_TYPE_STRING, "CurvePointItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, vel2nmax_table, PROPERTY_HINT_TYPE_STRING, "CurvePointItem");
        BIND_PROPERTY(VehicleDieselEngine, Variant::BOOL, retarder_present, "retarder");
        BIND_PROPERTY_W_HINT(
                VehicleDieselEngine, Variant::INT, retarder_placement, "retarder", PROPERTY_HINT_ENUM,
                "AfterGearbox,BetweenGearboxAndTC,BetweenTCAndEngine");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_torque_in_in, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_max_torque, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_max_power, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_fill_rate_increase, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_fill_rate_decrease, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, retarder_min_velocity, "retarder");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_max_torque, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_max_torque_rpm, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_max_rpm_torque, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_nominal_fuel_dose, "throttle_table_positions");
        BIND_PROPERTY(VehicleDieselEngine, Variant::FLOAT, throttle_table_resistance_torque, "throttle_table_positions");
        BIND_PROPERTY(
                VehicleDieselEngine, Variant::FLOAT, throttle_table_nominal_fuel_consumption_rate,
                "throttle_table_positions");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, throttle_table_positions, "throttle_table_positions",
                PROPERTY_HINT_TYPE_STRING, "ThrottlePositionItem");
        BIND_PROPERTY_W_HINT_RES_ARRAY(
                VehicleDieselEngine, Variant::ARRAY, torque_table, PROPERTY_HINT_TYPE_STRING, "CurvePointItem");
        ClassDB::bind_method(D_METHOD("fuel_pump", "enabled"), &VehicleDieselEngine::fuel_pump);
        ClassDB::bind_method(D_METHOD("oil_pump", "enabled"), &VehicleDieselEngine::oil_pump);

        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_AFTER_GEARBOX);
        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_BETWEEN_GEARBOX_AND_TC);
        BIND_ENUM_CONSTANT(RETARDER_PLACEMENT_BETWEEN_TC_AND_ENGINE);
    }

    VehicleEngine::EngineType VehicleDieselEngine::get_engine_type() const {
        return VehicleEngine::EngineType::DIESEL;
    }


    void VehicleDieselEngine::_fill_state_dictionary(Dictionary &p_state) const {
        VehicleEngine::_fill_state_dictionary(p_state);
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_state["engine_rpm"] = mover->EngineRPMRatio() * mover->EngineMaxRPM();
        p_state["oil_pump_active"] = mover->OilPump.is_active;
        p_state["oil_pump_disabled"] = mover->OilPump.is_disabled;
        p_state["oil_pump_pressure"] = mover->OilPump.pressure;
        p_state["fuel_pump_active"] = mover->FuelPump.is_active;
        p_state["fuel_pump_disabled"] = mover->FuelPump.is_disabled;
        p_state["diesel_startup"] = mover->dizel_startup;
        p_state["diesel_ignition"] = mover->dizel_ignition;
        p_state["diesel_spinup"] = mover->dizel_spinup;
        p_state["diesel_power"] = mover->dizel_Power;
        p_state["diesel_torque"] = mover->dizel_Torque;
        p_state["diesel_fill"] = mover->dizel_fill;
        p_state["diesel_max_rpm"] = mover->EngineMaxRPM();
    }

    void VehicleDieselEngine::_fill_config_dictionary(Dictionary &p_config) const {
        VehicleEngine::_fill_config_dictionary(p_config);
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        p_config["engine_shake_enabled"] = true;
    }

    void VehicleDieselEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        VehicleEngine::_do_update_internal_mover(p_mover);

        // FIXME: test data
        p_mover->EnginePowerSource.SourceType = TPowerSource::Accumulator;
        // end test data

        p_mover->OilPump.pressure_minimum = oil_pump_pressure_minimum;
        p_mover->OilPump.pressure_maximum = oil_pump_pressure_maximum;
        p_mover->FuelPump.start_type = start_mode_map.at(fuel_pump_start_mode);
        p_mover->OilPump.start_type = start_mode_map.at(oil_pump_start_mode);
        p_mover->WaterPump.start_type = start_mode_map.at(water_pump_start_mode);

        p_mover->dizel_nmin = mechanical_min_rpm;
        p_mover->dizel_nmax = mechanical_max_rpm;
        p_mover->dizel_nmax_cutoff = mechanical_fuel_cutoff_rpm;
        p_mover->dizel_AIM = mechanical_inertia;
        p_mover->engageupspeed = mechanical_clutch_engage_speed;
        p_mover->engagedownspeed = mechanical_clutch_disengage_speed;

        p_mover->hydro_TC = torque_converter_present;
        p_mover->hydro_TC_TMMax = torque_converter_max_torque_ratio;
        p_mover->hydro_TC_CouplingPoint = torque_converter_coupling_point;
        p_mover->hydro_TC_LockupTorque = torque_converter_lockup_torque;
        p_mover->hydro_TC_LockupRate = torque_converter_lockup_rate;
        p_mover->hydro_TC_UnlockRate = torque_converter_unlock_rate;
        p_mover->hydro_TC_FillRateInc = torque_converter_fill_rate_increase;
        p_mover->hydro_TC_FillRateDec = torque_converter_fill_rate_decrease;
        p_mover->hydro_TC_TorqueInIn = torque_converter_torque_in_in;
        p_mover->hydro_TC_TorqueInOut = torque_converter_torque_in_out;
        p_mover->hydro_TC_TorqueOutOut = torque_converter_torque_out_out;
        p_mover->hydro_TC_LockupSpeed = torque_converter_lockup_speed;
        p_mover->hydro_TC_UnlockSpeed = torque_converter_unlock_speed;

        p_mover->hydro_TC_Table.clear();
        for (int i = 0; i < torque_converter_table.size(); i++) {
            const Ref<CurvePointItem> &row = torque_converter_table[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: torque_converter_table property is null at index " + String::num(i));
                continue;
            }
            p_mover->hydro_TC_Table.emplace(row->get_x(), row->get_y());
        }

        p_mover->dizel_vel2nmax_Table.clear();
        for (int i = 0; i < vel2nmax_table.size(); i++) {
            const Ref<CurvePointItem> &row = vel2nmax_table[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: vel2nmax_table property is null at index " + String::num(i));
                continue;
            }
            // matches readV2NMAXList (Mover.cpp:8476-8489): x unconverted, y (rpm) -> rev/s
            p_mover->dizel_vel2nmax_Table.emplace(row->get_x(), row->get_y() / 60.0);
        }

        p_mover->hydro_R = retarder_present;
        p_mover->hydro_R_Placement = retarder_placement;
        p_mover->hydro_R_TorqueInIn = retarder_torque_in_in;
        p_mover->hydro_R_MaxTorque = retarder_max_torque;
        p_mover->hydro_R_MaxPower = retarder_max_power;
        p_mover->hydro_R_FillRateInc = retarder_fill_rate_increase;
        p_mover->hydro_R_FillRateDec = retarder_fill_rate_decrease;
        p_mover->hydro_R_MinVel = retarder_min_velocity;

        /* DList: tabela przepustnicy */
        p_mover->dizel_Mmax = throttle_table_max_torque;
        p_mover->dizel_nMmax = throttle_table_max_torque_rpm;
        p_mover->dizel_Mnmax = throttle_table_max_rpm_torque;
        p_mover->dizel_nominalfill = throttle_table_nominal_fuel_dose;
        p_mover->dizel_Mstand = throttle_table_resistance_torque;
        p_mover->dizel_NominalFuelConsumptionRate = throttle_table_nominal_fuel_consumption_rate;

        constexpr int MAX_THROTTLE_TABLE = Maszyna::ResArraySize + 1;
        const int throttle_table_size = static_cast<int>(throttle_table_positions.size());
        if (throttle_table_size > MAX_THROTTLE_TABLE) {
            UtilityFunctions::push_warning(
                    "[VehicleDieselEngine]: throttle_table_positions has " + String::num_int64(throttle_table_size) +
                    " entries, exceeding the mover's limit of " + String::num_int64(MAX_THROTTLE_TABLE) +
                    "; truncating.");
        }
        for (int i = 0; i < std::min(MAX_THROTTLE_TABLE, throttle_table_size); i++) {
            const Ref<ThrottlePositionItem> &row = throttle_table_positions[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: throttle_table_positions property is null at index " + String::num(i));
                continue;
            }
            p_mover->RList[i].Relay = row->get_throttle_position();
            p_mover->RList[i].R = row->get_fuel_dose();
            p_mover->RList[i].Mn = row->get_clutch_behavior();
        }

        /* DMList: charakterystyka momentu obrotowego silnika spalinowego */
        p_mover->dizel_Momentum_Table.clear();
        for (int i = 0; i < torque_table.size(); i++) {
            const Ref<CurvePointItem> &row = torque_table[i];
            if (row == nullptr || !row.is_valid()) {
                UtilityFunctions::push_warning(
                        "[VehicleDieselEngine]: torque_table property is null at index " + String::num(i));
                continue;
            }
            p_mover->dizel_Momentum_Table.emplace(row->get_x() / 60.0, row->get_y());
        }
    }

    void VehicleDieselEngine::oil_pump(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->OilPumpSwitch(p_enabled);
    }

    void VehicleDieselEngine::fuel_pump(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->FuelPumpSwitch(p_enabled);
    }

    void VehicleDieselEngine::_register_commands() {
        VehicleEngine::_register_commands();
        register_command("oil_pump", Callable(this, "oil_pump"));
        register_command("fuel_pump", Callable(this, "fuel_pump"));
    }

    void VehicleDieselEngine::_unregister_commands() {
        VehicleEngine::_unregister_commands();
        unregister_command("oil_pump", Callable(this, "oil_pump"));
        unregister_command("fuel_pump", Callable(this, "fuel_pump"));
    }
} // namespace godot
