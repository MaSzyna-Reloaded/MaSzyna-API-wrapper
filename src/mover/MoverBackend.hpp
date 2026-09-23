#pragma once
#include "../core/VehicleComponent.hpp"
#include "../core/VehicleController.hpp"
#include "../maszyna/McZapkie/MOVER.h"

/* The vendored Mover is the backend a component may happen to be implemented on. It is named
 * here and in the `Mover*` implementations that include this header - never in `VehicleComponent`
 * or in any `Vehicle<Domain>` interface, which describe a vehicle without saying what simulates
 * it. */

#define ASSERT_MOVER(mover_ptr)                                                                                        \
    if ((mover_ptr) == nullptr) {                                                                                      \
        return;                                                                                                        \
    }

namespace godot {
    /// The Mover of the vehicle this component belongs to, or nullptr while it belongs to none.
    inline TMoverParameters *mover_of(const VehicleComponent *p_component) {
        if (p_component == nullptr) {
            return nullptr;
        }
        const VehicleController *controller = p_component->get_controller();
        return controller != nullptr ? controller->get_mover() : nullptr;
    }
} // namespace godot
