#include "TrainDieselEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    TrainDieselEngine::~TrainDieselEngine() {
        wwlist.clear();
    }

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

    void TrainDieselEngine::_do_fetch_state_from_mover(TMoverParameters *p_mover, Dictionary &p_state) {
        TrainEngine::_do_fetch_state_from_mover(p_mover, p_state);

        p_state["engine_rpm"] = p_mover->EngineRPMRatio() * p_mover->EngineMaxRPM();
        p_state["oil_pump_active"] = p_mover->OilPump.is_active;
        p_state["oil_pump_disabled"] = p_mover->OilPump.is_disabled;
        p_state["oil_pump_pressure"] = p_mover->OilPump.pressure;

        p_state["fuel_pump_active"] = p_mover->FuelPump.is_active;
        p_state["fuel_pump_disabled"] = p_mover->FuelPump.is_disabled;

        p_state["diesel_startup"] = p_mover->dizel_startup;
        p_state["diesel_ignition"] = p_mover->dizel_ignition;
        p_state["diesel_spinup"] = p_mover->dizel_spinup;
        p_state["diesel_power"] = p_mover->dizel_Power;
        p_state["diesel_torque"] = p_mover->dizel_Torque;
        p_state["diesel_fill"] = p_mover->dizel_fill;
    }

    void TrainDieselEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        TrainEngine::_do_update_internal_mover(p_mover);

        // FIXME: test data
        p_mover->EnginePowerSource.SourceType = TPowerSource::Accumulator;
        p_mover->dizel_nmin = 100; // nie wiem skad to sie ustawia, w FIZ stonki nie ma
        // end test data

        p_mover->OilPump.pressure_minimum = oil_min_pressure;
        p_mover->OilPump.pressure_maximum = oil_max_pressure;

        p_mover->Ftmax = traction_force_max;

        /* FIXME: move to TrainDieselElectricEngine */
        /* tablica rezystorow rozr. (eng. Starting resistor array) WWList aka DEList aka TDESchemeTable */
        constexpr int MAX = sizeof(p_mover->DElist) / sizeof(Maszyna::TDEScheme);
        const int wwlist_size = static_cast<int>(wwlist.size());
        p_mover->MainCtrlPosNo = wwlist_size - 1;
        for (int i = 0; i < std::min(MAX, wwlist_size); i++) {
            const Ref<WWListItem> &row = wwlist[i];
            if (row == nullptr || !row.is_valid() || row.is_null()) {
                UtilityFunctions::push_warning("[TrainDieselEngine]: wwlist property is null at index " + String::num(i));
                return;
            }

            p_mover->DElist[i].RPM = row->get_rpm();
            p_mover->DElist[i].GenPower = row->get_max_power();
            p_mover->DElist[i].Umax = row->get_max_voltage();
            p_mover->DElist[i].Imax = row->get_max_current();
            if (row->get_has_shunting()) {
                p_mover->SST[i].Umin = row->get_min_wakeup_voltage();
                p_mover->SST[i].Umax = row->get_max_wakeup_voltage();
                p_mover->SST[i].Pmax = row->get_max_wakeup_power();
                p_mover->SST[i].Pmin = std::sqrt(std::pow(p_mover->SST[i].Umin, 2) / 47.6);
                p_mover->SST[i].Pmax = std::min(p_mover->SST[i].Pmax, std::pow(p_mover->SST[i].Umax, 2) / 47.6);
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

    TypedArray<TrainCommand> TrainDieselEngine::get_supported_commands() {
        TypedArray<TrainCommand> commands = TrainEngine::get_supported_commands();
        commands.append(make_train_command("oil_pump", Callable(this, "oil_pump")));
        commands.append(make_train_command("fuel_pump", Callable(this, "fuel_pump")));
        return commands;
    }
} // namespace godot
