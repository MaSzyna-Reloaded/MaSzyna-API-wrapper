#include "TrainDieselEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainDieselEngine::_bind_methods() {
        BIND_PROPERTY(Variant::FLOAT, "oil_min_pressure", "oil_pump/pressure_minimum", &TrainDieselEngine::set_oil_min_pressure, &TrainDieselEngine::get_oil_min_pressure, "oil_min_pressure");
        BIND_PROPERTY(Variant::FLOAT, "oil_max_pressure", "oil_pump/pressure_maximum", &TrainDieselEngine::set_oil_max_pressure, &TrainDieselEngine::get_oil_max_pressure, "oil_max_pressure");
        BIND_PROPERTY(Variant::FLOAT, "maximum_traction_force", "maximum_traction_force", &TrainDieselEngine::set_traction_force_max, &TrainDieselEngine::get_traction_force_max, "maximum_traction_force");
        BIND_PROPERTY_W_HINT_RES_ARRAY(Variant::ARRAY, "wwlist", "wwlist", &TrainDieselEngine::set_wwlist, &TrainDieselEngine::get_wwlist, "wwlist", PROPERTY_HINT_TYPE_STRING, "WWListItem");
        ClassDB::bind_method(D_METHOD("fuel_pump", "enabled"), &TrainDieselEngine::fuel_pump);
        ClassDB::bind_method(D_METHOD("oil_pump", "enabled"), &TrainDieselEngine::oil_pump);
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

            mover->DElist[i].RPM = row->get_rpm();
            mover->DElist[i].GenPower = row->get_max_power();
            mover->DElist[i].Umax = row->get_max_voltage();
            mover->DElist[i].Imax = row->get_max_current();
            if (row->get_has_shunting()) {
                mover->SST[i].Umin = row->get_min_wakeup_voltage();
                mover->SST[i].Umax = row->get_max_wakeup_voltage();
                mover->SST[i].Pmax = row->get_max_wakeup_power();
                mover->SST[i].Pmin = std::sqrt(std::pow(mover->SST[i].Umin, 2) / 47.6);
                mover->SST[i].Pmax = std::min(mover->SST[i].Pmax, std::pow(mover->SST[i].Umax, 2) / 47.6);
            }
        }
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
