#include "TrainElectricSeriesEngine.hpp"
#include "macros.hpp"

#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainElectricSeriesEngine::_bind_methods() {
        BIND_PROPERTY(TrainElectricSeriesEngine, Variant::FLOAT, nominal_voltage);
        BIND_PROPERTY(TrainElectricSeriesEngine, Variant::FLOAT, winding_resistance);
    }

    TrainEngine::EngineType TrainElectricSeriesEngine::get_engine_type() {
        return TrainEngine::EngineType::ELECTRIC_SERIES_MOTOR;
    }

    void TrainElectricSeriesEngine::_do_update_internal_mover(TMoverParameters *p_mover) {
        TrainElectricEngine::_do_update_internal_mover(p_mover);
        p_mover->NominalVoltage = nominal_voltage;
        p_mover->WindingRes = winding_resistance;
    }

} // namespace godot
