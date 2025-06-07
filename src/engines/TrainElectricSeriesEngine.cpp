#include "TrainElectricSeriesEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainElectricSeriesEngine::_bind_methods() {
        BIND_PROPERTY(Variant::FLOAT, "nominal_voltage", "nominal_voltage", &TrainElectricSeriesEngine::set_nominal_voltage, &TrainElectricSeriesEngine::get_nominal_voltage, "nominal_voltage");
        BIND_PROPERTY(Variant::FLOAT, "winding_resistance", "winding_resistance", &TrainElectricSeriesEngine::set_winding_resistance, &TrainElectricSeriesEngine::get_winding_resistance, "winding_resistance");
    }

    TrainEngine::EngineType TrainElectricSeriesEngine::get_engine_type() {
        return TrainEngine::EngineType::ELECTRIC_SERIES_MOTOR;
    }

    void TrainElectricSeriesEngine::_do_update_internal_mover(TMoverParameters *mover) {
        TrainElectricEngine::_do_update_internal_mover(mover);
        mover->NominalVoltage = nominal_voltage;
        mover->WindingRes = winding_resistance;
    }

} // namespace godot
