#include "MoverVehicleSwitches.hpp"
#include "../mover/MoverBackend.hpp"
#include "VehicleSwitches.hpp"

namespace godot {
    void MoverVehicleSwitches::_bind_methods() {}


    void MoverVehicleSwitches::_apply_configuration() {
        TMoverParameters *p_mover = mover_of(this);
        ASSERT_MOVER(p_mover);
        ASSERT_MOVER(p_mover);
        VehicleComponent::_apply_configuration();

        p_mover->PantSwitchType = get_pantograph_impulse() ? "impulse" : "";
        p_mover->ConvSwitchType = get_converter_impulse() ? "impulse" : "";
        p_mover->StLinSwitchType = get_motor_connectors_impulse() ? "impulse" : "toggle";
    }


    bool MoverVehicleSwitches::get_sand_active() const {
        const TMoverParameters *mover = mover_of(this);
        return mover != nullptr ? mover->SandDose : false;
    }

    void MoverVehicleSwitches::_fill_state_dictionary(Dictionary &p_state) const {
        // a component without a backend publishes nothing at all, rather than zeroes
        if (mover_of(this) == nullptr) {
            return;
        }
        p_state["sand_active"] = get_sand_active();
    }

    void MoverVehicleSwitches::sand(const bool p_active) {
        TMoverParameters *mover = mover_of(this);
        ASSERT_MOVER(mover);
        // Train.cpp:1917-1939 (OnCommand_sandboxactivate) -> SandboxManual(State),
        // "sand_bt:"/ggSandButton (Train.cpp:10044) - momentary, active only while held.
        mover->SandboxManual(p_active);
    }


} // namespace godot
