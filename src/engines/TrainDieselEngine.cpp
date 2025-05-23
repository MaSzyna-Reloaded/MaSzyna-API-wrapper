#include "TrainDieselEngine.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainDieselEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_oil_min_pressure"), &TrainDieselEngine::get_oil_min_pressure);
        ClassDB::bind_method(D_METHOD("set_oil_min_pressure", "value"), &TrainDieselEngine::set_oil_min_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "oil_pump/pressure_minimum"), "set_oil_min_pressure",
                "get_oil_min_pressure");

        ClassDB::bind_method(D_METHOD("get_oil_max_pressure"), &TrainDieselEngine::get_oil_max_pressure);
        ClassDB::bind_method(D_METHOD("set_oil_max_pressure", "value"), &TrainDieselEngine::set_oil_max_pressure);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "oil_pump/pressure_maximum"), "set_oil_max_pressure",
                "get_oil_max_pressure");

        ClassDB::bind_method(D_METHOD("get_traction_force_max"), &TrainDieselEngine::get_traction_force_max);
        ClassDB::bind_method(D_METHOD("set_traction_force_max", "value"), &TrainDieselEngine::set_traction_force_max);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "Ftmax"), "set_traction_force_max", "get_traction_force_max");

        ClassDB::bind_method(D_METHOD("set_wwlist"), &TrainDieselEngine::set_wwlist);
        ClassDB::bind_method(D_METHOD("get_wwlist"), &TrainDieselEngine::get_wwlist);
        ClassDB::bind_method(D_METHOD("fuel_pump", "enabled"), &TrainDieselEngine::fuel_pump);
        ClassDB::bind_method(D_METHOD("oil_pump", "enabled"), &TrainDieselEngine::oil_pump);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::ARRAY, "wwlist", PROPERTY_HINT_TYPE_STRING, String::num(Variant::OBJECT) + "/" + String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":WWListItem", PROPERTY_USAGE_DEFAULT, "TypedArray<WWListItem>"),
                "set_wwlist", "get_wwlist");
    }

    TrainEngine::EngineType TrainDieselEngine::get_engine_type() {
        return TrainEngine::EngineType::DIESEL;
    }

    void TrainDieselEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        TrainEngine::_do_fetch_state_from_mover(mover, state);

        state["engine_rpm"] = mover->EngineRPMRatio() * mover->EngineMaxRPM();
        state["oil_pump_active"] = mover->OilPump.is_active;
        state["oil_pump_disabled"] = mover->OilPump.is_disabled;
        state["oil_pump_pressure"] = mover->OilPump.pressure;

        state["fuel_pump_active"] = mover->FuelPump.is_active;
        state["fuel_pump_disabled"] = mover->FuelPump.is_disabled;

        state["diesel_startup"] = mover->dizel_startup;
        state["diesel_ignition"] = mover->dizel_ignition;
        state["diesel_spinup"] = mover->dizel_spinup;
        state["diesel_power"] = mover->dizel_Power;
        state["diesel_torque"] = mover->dizel_Torque;
        state["diesel_fill"] = mover->dizel_fill;
    }

    void TrainDieselEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainEngine::_do_update_internal_mover(mover);

        // FIXME: test data
        mover->EnginePowerSource.SourceType = TPowerSource::Accumulator;
        mover->dizel_nmin = 100; // nie wiem skad to sie ustawia, w FIZ stonki nie ma
        // end test data

        mover->OilPump.pressure_minimum = oil_min_pressure;
        mover->OilPump.pressure_maximum = oil_max_pressure;

        mover->Ftmax = traction_force_max;

        /* FIXME: move to TrainDieselElectricEngine */
        /* tablica rezystorow rozr. (eng. Starting resistor array) WWList aka DEList aka TDESchemeTable */
        constexpr int _max = sizeof(mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(wwlist.size());
        mover->MainCtrlPosNo = wwlist_size - 1;
        for (int i = 0; i < std::min(_max, wwlist_size); i++) {
            const Ref<WWListItem> &row = wwlist[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning("[TrainDieselEngine]: wwlist property is null at index " + String::num(i));
                return;
            }

            mover->DElist[i].RPM = row->rpm;
            mover->DElist[i].GenPower = row->max_power;
            mover->DElist[i].Umax = row->max_voltage;
            mover->DElist[i].Imax = row->max_current;
            if (row->has_shunting) {
                mover->SST[i].Umin = row->min_wakeup_voltage;
                mover->SST[i].Umax = row->max_wakeup_voltage;
                mover->SST[i].Pmax = row->max_wakeup_power;
                mover->SST[i].Pmin = std::sqrt(std::pow(mover->SST[i].Umin, 2) / 47.6);
                mover->SST[i].Pmax = std::min(mover->SST[i].Pmax, std::pow(mover->SST[i].Umax, 2) / 47.6);
            }
        }
    }

    double TrainDieselEngine::get_oil_min_pressure() const {
        return oil_min_pressure;
    }

    void TrainDieselEngine::set_oil_min_pressure(const float value) {
        oil_min_pressure = value;
        _dirty = true;
    }

    double TrainDieselEngine::get_oil_max_pressure() const {
        return oil_max_pressure;
    }

    void TrainDieselEngine::set_oil_max_pressure(const float value) {
        oil_max_pressure = value;
        _dirty = true;
    }

    double TrainDieselEngine::get_traction_force_max() const {
        return traction_force_max;
    }

    void TrainDieselEngine::set_traction_force_max(const double value) {
        traction_force_max = value;
        _dirty = true;
    }

    TypedArray<WWListItem> TrainDieselEngine::get_wwlist() {
        return wwlist;
    }

    void TrainDieselEngine::set_wwlist(const TypedArray<WWListItem>& p_wwlist) {
        wwlist.clear();
        wwlist.append_array(p_wwlist);
    }

    void TrainDieselEngine::oil_pump(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->OilPumpSwitch(p_enabled);
    }

    void TrainDieselEngine::fuel_pump(const bool p_enabled) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->FuelPumpSwitch(p_enabled);
    }

    void TrainDieselEngine::_register_commands() {
        TrainEngine::_register_commands();
        register_command("oil_pump", Callable(this, "oil_pump"));
        register_command("fuel_pump", Callable(this, "fuel_pump"));
    }

    void TrainDieselEngine::_unregister_commands() {
        TrainEngine::_unregister_commands();
        unregister_command("oil_pump", Callable(this, "oil_pump"));
        unregister_command("fuel_pump", Callable(this, "fuel_pump"));
    }
} // namespace godot
