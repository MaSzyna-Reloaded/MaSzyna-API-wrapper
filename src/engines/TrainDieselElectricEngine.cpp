#include "TrainDieselElectricEngine.hpp"

namespace godot {
    TrainDieselElectricEngine::TrainDieselElectricEngine() {
        set_capability("engine");
        set_type_id("dieselelectric");
    }

    void TrainDieselElectricEngine::_bind_methods() {}

    TrainEngine::EngineType TrainDieselElectricEngine::get_engine_type() {
        return TrainEngine::EngineType::DIESEL_ELECTRIC;
    }
} // namespace godot
