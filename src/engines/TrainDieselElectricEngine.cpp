#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "TrainDieselElectricEngine.hpp"

namespace godot {
    TrainDieselElectricEngine::TrainDieselElectricEngine() = default;

    void TrainDieselElectricEngine::_bind_methods() {}

    TEngineType TrainDieselElectricEngine::get_engine_type() {
        return TEngineType::DieselElectric;
    }
} // namespace godot
