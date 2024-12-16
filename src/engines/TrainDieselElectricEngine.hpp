#pragma once
#include "TrainDieselEngine.hpp"

namespace godot {
    class TrainDieselElectricEngine final : public TrainDieselEngine {
            GDCLASS(TrainDieselElectricEngine, TrainDieselEngine)

        private:
            static void _bind_methods();

        protected:
            TEngineType get_engine_type() override;
    };
} // namespace godot
