#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "RailVehicleMasterController.hpp"

namespace godot {
    /* RailVehicleMasterController on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleMasterController : public RailVehicleMasterController, public MoverComponent {
            GDCLASS(MoverRailVehicleMasterController, RailVehicleMasterController);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
    };
} // namespace godot
