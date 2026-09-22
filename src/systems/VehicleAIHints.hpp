#pragma once
#include "../core/VehicleComponent.hpp"
#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>

namespace godot {
    class VehicleController;
    class VehicleAIHints : public VehicleComponent {
            GDCLASS(VehicleAIHints, VehicleComponent);

        public:
            /* Pantstate= : suggested pantograph setup for the AI driver */
            enum PantographState {
                PANTOGRAPH_STATE_FRONT = 1,
                PANTOGRAPH_STATE_REAR = 2,
                PANTOGRAPH_STATE_BOTH = 3,
            };

        private:
            static void _bind_methods();

        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;

        public:
            MAKE_MEMBER_GS_NR(PantographState, pantograph_state, PANTOGRAPH_STATE_FRONT);
            MAKE_MEMBER_GS(bool, raise_pantographs_when_idle, true);
            MAKE_MEMBER_GS(double, local_brake_acceleration_factor, 1.05);
    };
} // namespace godot
VARIANT_ENUM_CAST(VehicleAIHints::PantographState)
