#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>

#include "TrainElectricSeriesEngine.hpp"

namespace godot {
    TrainElectricSeriesEngine::TrainElectricSeriesEngine() = default;

    void TrainElectricSeriesEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_nominal_voltage"), &TrainElectricSeriesEngine::get_nominal_voltage);
        ClassDB::bind_method(D_METHOD("set_nominal_voltage"), &TrainElectricSeriesEngine::set_nominal_voltage);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "nominal_voltage"), "set_nominal_voltage", "get_nominal_voltage");

        ClassDB::bind_method(D_METHOD("get_winding_resistance"), &TrainElectricSeriesEngine::get_winding_resistance);
        ClassDB::bind_method(D_METHOD("set_winding_resistance"), &TrainElectricSeriesEngine::set_winding_resistance);
        ADD_PROPERTY(
                PropertyInfo(Variant::FLOAT, "winding_resistance"), "set_winding_resistance", "get_winding_resistance");
    }

    TEngineType TrainElectricSeriesEngine::get_engine_type() {
        return TEngineType::ElectricSeriesMotor;
    }

    void TrainElectricSeriesEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainElectricEngine::_do_update_internal_mover(mover);

        mover->NominalVoltage = nominal_voltage;
        mover->WindingRes = winding_resistance;
    }

    double TrainElectricSeriesEngine::get_winding_resistance() const {
        return winding_resistance;
    }

    void TrainElectricSeriesEngine::set_winding_resistance(const double value) {
        winding_resistance = value;
        _dirty = true;
    }

    double TrainElectricSeriesEngine::get_nominal_voltage() const {
        return nominal_voltage;
    }

    void TrainElectricSeriesEngine::set_nominal_voltage(const double value) {
        nominal_voltage = value;
        _dirty = true;
    }

} // namespace godot
