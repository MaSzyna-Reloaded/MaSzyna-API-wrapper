#include "LegacyWagon.hpp"

namespace godot {
    void LegacyWagon::_bind_methods() {}

    void LegacyWagon::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        LegacyRailVehicle::_do_fetch_state_from_mover(mover, state);
        state["cabin"] = mover->CabActive;
        state["cabin_occupied"] = mover->CabOccupied;
    }
} // namespace godot
