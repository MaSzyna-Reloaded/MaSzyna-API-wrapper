#include "TrainDieselElectricEngine.hpp"

namespace godot {
    TrainDieselElectricEngine::TrainDieselElectricEngine() = default;

    void TrainDieselElectricEngine::_bind_methods() {}

    TEngineType TrainDieselElectricEngine::get_engine_type() {
        return TEngineType::DieselElectric;
    }
} // namespace godot
