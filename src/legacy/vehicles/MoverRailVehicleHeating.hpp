#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleHeating.hpp"

namespace godot {
    /* RailVehicleHeating on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleHeating : public RailVehicleHeating, public MoverComponent {
            GDCLASS(MoverRailVehicleHeating, RailVehicleHeating);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;

        public:
            bool get_active() const override;
            bool get_allowed() const override;
            double get_power() const override;
            void heating(bool p_enabled) override;
            void _fill_state_dictionary(Dictionary &p_state) const override;
    };
} // namespace godot
