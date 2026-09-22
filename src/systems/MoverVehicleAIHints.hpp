#pragma once
#include "VehicleAIHints.hpp"

namespace godot {
    /* VehicleAIHints on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleAIHints : public VehicleAIHints {
            GDCLASS(MoverVehicleAIHints, VehicleAIHints);

        private:
            static void _bind_methods();
        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
    };
} // namespace godot
