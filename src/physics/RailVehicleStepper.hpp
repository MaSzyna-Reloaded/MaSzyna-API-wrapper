#pragma once
#include <godot_cpp/classes/node.hpp>

namespace godot {
    /* Drives RailVehicleServer's step, and does it from `_process` with the lowest priority
     * rather than from SceneTree's `process_frame`.
     *
     * The distinction is the whole reason this node exists: `process_frame` is emitted *after*
     * every node has been processed, so anything that reads a vehicle's transform in its own
     * `_process` - the external camera is one - reads the position from before the step and is
     * drawn a frame behind it. A node processed first puts the step ahead of every reader again,
     * which is what `process_priority = -100` guaranteed before the server moved to C++
     * (see `4ec5490`, and `FINDINGS.md`, 2026-09-24). */
    class RailVehicleStepper : public Node {
            GDCLASS(RailVehicleStepper, Node)

            /* Low enough that nothing reasonable sorts before it; Godot processes ascending. */
            static constexpr int STEP_PRIORITY = -100;

        protected:
            static void _bind_methods();

        public:
            RailVehicleStepper();
            void _process(double p_delta) override;
    };
} // namespace godot
