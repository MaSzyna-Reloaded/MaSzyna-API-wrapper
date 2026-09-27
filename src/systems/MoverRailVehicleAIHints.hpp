#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "RailVehicleAIHints.hpp"

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
