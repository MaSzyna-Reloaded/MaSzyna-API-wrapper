#pragma once
#include "TrainController.hpp"
#include "VehicleComponent.hpp"

namespace godot {
    /* A component only a railway vehicle has - a brake system, couplers, an engine of the
     * original's kinds, a master controller, the alerter. It belongs to a TrainController. */
    class TrainComponent : public VehicleComponent {
            GDCLASS(TrainComponent, VehicleComponent);

        protected:
            static void _bind_methods() {}

        public:
            /* The railway vehicle this component belongs to */
            TrainController *get_train_controller() const {
                return Object::cast_to<TrainController>(get_controller());
            }
    };
} // namespace godot
