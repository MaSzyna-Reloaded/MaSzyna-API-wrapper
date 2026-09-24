#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleHorns.hpp"

namespace godot {
    /* VehicleHorns on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleHorns : public VehicleHorns, public MoverComponent {
            GDCLASS(MoverVehicleHorns, VehicleHorns);

        private:
            static void _bind_methods();
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_low_pressed() const override;
            bool get_high_pressed() const override;
            bool get_whistle_pressed() const override;
            int get_combined_signal() const override;
            bool get_low_active() const override;
            bool get_high_active() const override;
            bool get_whistle_active() const override;
            int get_horn() const override;
            void set_horn_low(bool p_state) override;
            void set_horn_high(bool p_state) override;
            void set_whistle(bool p_state) override;
            void set_horn(double p_position) override;
    };
} // namespace godot
