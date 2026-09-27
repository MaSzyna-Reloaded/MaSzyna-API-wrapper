#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleHorns.hpp"

namespace godot {
    /* RailVehicleHorns on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleHorns : public RailVehicleHorns, public MoverComponent {
            GDCLASS(MoverRailVehicleHorns, RailVehicleHorns);

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
