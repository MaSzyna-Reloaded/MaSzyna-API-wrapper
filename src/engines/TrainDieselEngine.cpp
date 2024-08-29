#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "TrainDieselEngine.hpp"
#include "maszyna/McZapkie/MOVER.h"

namespace godot {
    TrainDieselEngine::TrainDieselEngine()= default;

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
    }

    TEngineType TrainDieselEngine::get_engine_type() {
        return TEngineType::DieselEngine;
    }

    void TrainDieselEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        TrainEngine::_do_fetch_state_from_mover(mover, state);

        state["oil_pump_active"] = mover->OilPump.is_active;
        state["oil_pump_disabled"] = mover->OilPump.is_disabled;
        state["oil_pump_pressure"] = mover->OilPump.pressure;

        state["fuel_pump_active"] = mover->FuelPump.is_active;
        state["fuel_pump_disabled"] = mover->FuelPump.is_disabled;
    }

    void TrainDieselEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainEngine::_do_update_internal_mover(mover);

        mover->OilPump.pressure_minimum = oil_min_pressure;
        mover->OilPump.pressure_maximum = oil_max_pressure;

        mover->Ftmax = traction_force_max;
    }

    double TrainDieselEngine::get_oil_min_pressure() const {
        return oil_min_pressure;
    }

    void TrainDieselEngine::set_oil_min_pressure(double value) {
        oil_min_pressure = value;
        _dirty = true;
    }

    double TrainDieselEngine::get_oil_max_pressure() const {
        return oil_max_pressure;
    }

    void TrainDieselEngine::set_oil_max_pressure(double value) {
        oil_max_pressure = value;
        _dirty = true;
    }

    double TrainDieselEngine::get_traction_force_max() const {
        return traction_force_max;
    }

    void TrainDieselEngine::set_traction_force_max(double value) {
        traction_force_max = value;
        _dirty = true;
    }
} // namespace godot
