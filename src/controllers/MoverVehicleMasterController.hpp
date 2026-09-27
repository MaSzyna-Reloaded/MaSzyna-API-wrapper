#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleMasterController.hpp"

namespace godot {
    /* VehicleMasterController on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleMasterController : public VehicleMasterController, public MoverComponent {
            GDCLASS(MoverVehicleMasterController, VehicleMasterController);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
    };
} // namespace godot
