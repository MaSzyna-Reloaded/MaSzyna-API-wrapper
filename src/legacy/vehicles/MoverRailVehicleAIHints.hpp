#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleAIHints.hpp"

namespace godot {
    /* RailVehicleAIHints on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleAIHints : public RailVehicleAIHints, public MoverComponent {
            GDCLASS(MoverRailVehicleAIHints, RailVehicleAIHints);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;
    };
} // namespace godot
