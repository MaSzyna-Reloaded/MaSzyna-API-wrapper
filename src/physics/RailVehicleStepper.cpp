#include "RailVehicleStepper.hpp"
#include "RailVehicleServer.hpp"

namespace godot {
    void RailVehicleStepper::_bind_methods() {}

    RailVehicleStepper::RailVehicleStepper() {
        set_process_priority(STEP_PRIORITY);
        set_process(true);
    }

    void RailVehicleStepper::_process(const double p_delta) {
        if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
            server->step_frame(p_delta);
        }
    }
} // namespace godot
