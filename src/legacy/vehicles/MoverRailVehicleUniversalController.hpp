#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleUniversalController.hpp"

namespace godot {
    /* RailVehicleUniversalController on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleUniversalController : public RailVehicleUniversalController, public MoverComponent {
            GDCLASS(MoverRailVehicleUniversalController, RailVehicleUniversalController);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
    };
} // namespace godot
