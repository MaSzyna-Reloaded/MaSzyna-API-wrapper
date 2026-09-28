#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleMasterController.hpp"

namespace godot {
    /* RailVehicleMasterController on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleMasterController : public RailVehicleMasterController, public MoverComponent {
            GDCLASS(MoverRailVehicleMasterController, RailVehicleMasterController);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _fill_state_dictionary(Dictionary &p_state) const override;
    };
} // namespace godot
