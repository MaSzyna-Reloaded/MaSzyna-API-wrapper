#pragma once
#include "TrainDieselEngine.hpp"

namespace godot {
    class TrainDieselElectricEngine: public TrainDieselEngine {
            GDCLASS(TrainDieselElectricEngine, TrainDieselEngine)

        private:
            static void _bind_methods();

        protected:
            TrainEngine::EngineType get_engine_type() override;

        public:
            TrainDieselElectricEngine();
    };
} // namespace godot
