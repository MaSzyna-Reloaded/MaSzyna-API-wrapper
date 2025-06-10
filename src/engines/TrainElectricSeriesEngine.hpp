#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "TrainElectricEngine.hpp"
#include "macros.hpp"

namespace godot {
    class TrainController;

    class TrainElectricSeriesEngine: public TrainElectricEngine {
            GDCLASS(TrainElectricSeriesEngine, TrainElectricEngine)
        public:
            static void _bind_methods();

        protected:
            EngineType get_engine_type() override;
            void _do_update_internal_mover(TMoverParameters *mover) override;

        public:
            MAKE_MEMBER_GS(double, nominal_voltage, 0.0);
            MAKE_MEMBER_GS(double, winding_resistance, 0.0);
    };
} // namespace godot
