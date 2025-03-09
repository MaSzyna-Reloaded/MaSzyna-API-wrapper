#include "TrainDieselElectricEngine.hpp"

namespace godot {
    void TrainDieselElectricEngine::_bind_methods() {}

    TrainEngine::EngineType TrainDieselElectricEngine::get_engine_type() {
        return TrainEngine::EngineType::ENGINE_TYPE_DIESEL_ELECTRIC;
    }
} // namespace godot
