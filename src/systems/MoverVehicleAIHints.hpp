#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleAIHints.hpp"

namespace godot {
    /* VehicleAIHints on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleAIHints : public VehicleAIHints, public MoverComponent {
            GDCLASS(MoverVehicleAIHints, VehicleAIHints);

        private:
            static void _bind_methods();
        protected:
            void _apply_configuration() override;
    };
} // namespace godot
