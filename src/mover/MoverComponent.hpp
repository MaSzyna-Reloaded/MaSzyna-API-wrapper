#pragma once
#include "../core/MoverVehicleController.hpp"
#include "../maszyna/McZapkie/MOVER.h"

namespace godot {
    /* What every Mover* component shares: the Mover implementation of the vehicle it belongs to,
     * the way TrainPart held its TrainController. Not a Godot class - a Mover* component inherits
     * it next to its Vehicle* interface, and MoverVehicleController fills it in when the component
     * joins the vehicle and clears it when the component leaves. */
    class MoverComponent {
            friend class MoverVehicleController;

        private:
            MoverVehicleController *mover_controller = nullptr;

        public:
            virtual ~MoverComponent() = default;

            /* The vehicle's Mover implementation, or null while the component belongs to none. */
            MoverVehicleController *get_mover_controller() const {
                return mover_controller;
            }

            /* The vehicle's Mover, or null while there is none yet. */
            TMoverParameters *get_mover() const {
                return mover_controller != nullptr ? mover_controller->get_mover() : nullptr;
            }
    };
} // namespace godot
