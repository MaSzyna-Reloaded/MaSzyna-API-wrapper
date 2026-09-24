#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleHeating.hpp"

namespace godot {
    /* VehicleHeating on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleHeating : public VehicleHeating, public MoverComponent {
            GDCLASS(MoverVehicleHeating, VehicleHeating);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;

        public:
            bool get_active() const override;
            double get_power() const override;
            void heating(bool p_enabled) override;
            void _fill_state_dictionary(Dictionary &p_state) const override;
    };
} // namespace godot
