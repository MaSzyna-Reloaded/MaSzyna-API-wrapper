#include "MoverVehicleAIHints.hpp"
#include "../mover/MoverBackend.hpp"
#include "VehicleAIHints.hpp"

namespace godot {
    void MoverVehicleAIHints::_bind_methods() {}


    void MoverVehicleAIHints::_apply_configuration() {
        TMoverParameters *p_mover = get_mover();
        ASSERT_MOVER(p_mover);
        VehicleComponent::_apply_configuration();

        p_mover->AIHintPantstate = get_pantograph_state();
        p_mover->AIHintPantUpIfIdle = get_raise_pantographs_when_idle();
        p_mover->AIHintLocalBrakeAccFactor = get_local_brake_acceleration_factor();
    }

} // namespace godot
