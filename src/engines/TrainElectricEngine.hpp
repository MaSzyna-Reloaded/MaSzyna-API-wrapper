#pragma once
#include "TrainEngine.hpp"


namespace godot {
    class TrainController;

    class TrainElectricEngine : public TrainEngine {
            GDCLASS(TrainElectricEngine, TrainEngine)

            static void _bind_methods();

        public:
            TrainElectricEngine();
            ~TrainElectricEngine() override = default;
    };
} // namespace godot
