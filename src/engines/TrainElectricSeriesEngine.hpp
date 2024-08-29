#pragma once
#include "TrainElectricEngine.hpp"
#include "maszyna/McZapkie/MOVER.h"


namespace godot {
    class TrainController;

    class TrainElectricSeriesEngine : public TrainElectricEngine {
            GDCLASS(TrainElectricSeriesEngine, TrainElectricEngine)

            static void _bind_methods();
            double nominal_voltage = 0.0;    // Volt -> NominalVoltage
            double winding_resistance = 0.0; // WindingRes -> WindingRes

        protected:
            TEngineType get_engine_type() override;
            void _do_update_internal_mover(TMoverParameters *mover) override;

        public:
            double get_nominal_voltage() const;
            void set_nominal_voltage(double value);

            double get_winding_resistance() const;
            void set_winding_resistance(double value);

            TrainElectricSeriesEngine();
            ~TrainElectricSeriesEngine() override = default;
    };
} // namespace godot
