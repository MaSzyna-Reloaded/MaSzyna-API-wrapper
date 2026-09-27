#include "SimulationClock.hpp"
#include "MaszynaRuntime.hpp"

namespace godot {
    void SimulationClock::_bind_methods() {}

    SimulationClock::SimulationClock() {
        set_process_priority(CLOCK_PRIORITY);
        set_process(true);
    }

    void SimulationClock::_process(const double p_delta) {
        if (MaszynaRuntime *runtime = MaszynaRuntime::get_instance(); runtime != nullptr) {
            runtime->advance(p_delta);
        }
    }
} // namespace godot
