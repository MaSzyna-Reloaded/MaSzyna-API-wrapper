#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleUniversalController.hpp"

namespace godot {
    /* VehicleUniversalController on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleUniversalController : public VehicleUniversalController {
            GDCLASS(MoverVehicleUniversalController, VehicleUniversalController);

        private:
            static void _bind_methods();
        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
    };
} // namespace godot
