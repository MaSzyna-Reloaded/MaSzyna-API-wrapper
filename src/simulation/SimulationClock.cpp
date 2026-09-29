#include "SimulationClock.hpp"
#include "SimulationServer.hpp"

namespace godot {
    void SimulationClock::_bind_methods() {}

    SimulationClock::SimulationClock() {
        set_process_priority(CLOCK_PRIORITY);
        set_process(true);
    }

    void SimulationClock::_process(const double p_delta) {
        if (SimulationServer *runtime = SimulationServer::get_instance(); runtime != nullptr) {
            runtime->simulation_advance(p_delta);
        }
    }
} // namespace godot
