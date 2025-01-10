#include "TrainDieselElectricEngine.hpp"

namespace godot {
    void TrainDieselElectricEngine::_bind_methods() {}

    TrainEngine::TrainEngineType TrainDieselElectricEngine::get_engine_type() {
        return TrainEngineType::ENGINE_TYPE_DIESEL_ELECTRIC;
    }
} // namespace godot
