#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "TrainEngine.hpp"


namespace godot {
    class TrainController;

    class TrainDieselEngine : public TrainEngine {
            GDCLASS(TrainDieselEngine, TrainEngine)
        private:
            static void _bind_methods();
            float oil_min_pressure = 0.0;   // OilMinPressure -> OilPump.pressure_minimum
            float oil_max_pressure = 0.65;  // OilMaxPressure -> OilPump.pressure_maximum
            double traction_force_max = 0.0; // Ftmax -> Ftmax

            TypedArray<Array> wwlist;

        protected:
            TEngineType get_engine_type() override;
            void _do_update_internal_mover(TMoverParameters *mover) override;
            void _do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) override;
            void _register_commands() override;
            void _unregister_commands() override;


        public:
            double get_oil_min_pressure() const;
            void set_oil_min_pressure(float value);

            double get_oil_max_pressure() const;
            void set_oil_max_pressure(float value);

            double get_traction_force_max() const;
            void set_traction_force_max(double value);

            TypedArray<Array> get_wwlist();
            void set_wwlist(const TypedArray<Array>& p_wwlist);

            void oil_pump(bool p_enabled);
            void fuel_pump(bool p_enabled);
    };
} // namespace godot
