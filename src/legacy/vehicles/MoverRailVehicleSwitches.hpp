#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleSwitches.hpp"

namespace godot {
    /* RailVehicleSwitches on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleSwitches : public RailVehicleSwitches, public MoverComponent {
            GDCLASS(MoverRailVehicleSwitches, RailVehicleSwitches);

        protected:
            /* The vehicle's Mover, taken when its simulation starts and dropped before it is freed */
            void _implementation_changed() override {
                take_mover(
                        get_implementation(),
                        train_controller_node != nullptr ? train_controller_node->get_rid() : RID());
            }

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_sand_active() const override;
            void sand(bool p_active) override;
    };
} // namespace godot
