#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "TrainDieselEngine.hpp"

namespace godot {
    TrainDieselEngine::TrainDieselEngine() = default;

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

        ClassDB::bind_method(D_METHOD("get_fuel_pump_enabled"), &TrainDieselEngine::get_fuel_pump_enabled);
        ClassDB::bind_method(D_METHOD("set_fuel_pump_enabled"), &TrainDieselEngine::set_fuel_pump_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/fuel_pump_enabled"), "set_fuel_pump_enabled",
                "get_fuel_pump_enabled");
        ClassDB::bind_method(D_METHOD("get_oil_pump_enabled"), &TrainDieselEngine::get_oil_pump_enabled);
        ClassDB::bind_method(D_METHOD("set_oil_pump_enabled"), &TrainDieselEngine::set_oil_pump_enabled);
        ADD_PROPERTY(
                PropertyInfo(Variant::BOOL, "switches/oil_pump_enabled"), "set_oil_pump_enabled",
                "get_oil_pump_enabled");
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

        // FIXME: test data
        mover->EnginePowerSource.SourceType = TPowerSource::Accumulator;

        mover->OilPump.pressure_minimum = oil_min_pressure;
        mover->OilPump.pressure_maximum = oil_max_pressure;

        mover->FuelPumpSwitch(sw_fuel_pump_enabled);
        mover->OilPumpSwitch(sw_oil_pump_enabled);

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

    void TrainDieselEngine::set_fuel_pump_enabled(const bool p_state) {
        sw_fuel_pump_enabled = p_state;
        _dirty = true;
    }

    bool TrainDieselEngine::get_fuel_pump_enabled() const {
        return sw_fuel_pump_enabled;
    }

    void TrainDieselEngine::set_oil_pump_enabled(const bool p_state) {
        sw_oil_pump_enabled = p_state;
        _dirty = true;
    }

    bool TrainDieselEngine::get_oil_pump_enabled() const {
        return sw_oil_pump_enabled;
    }

} // namespace godot
