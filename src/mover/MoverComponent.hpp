#pragma once
#include "../core/MoverVehicleController.hpp"
#include "../maszyna/McZapkie/MOVER.h"

namespace godot {
    /* What every Mover* component shares: the Mover implementation of the vehicle it belongs to,
     * the way TrainPart held its TrainController. Not a Godot class - a Mover* component inherits
     * it next to its Vehicle* interface, and MoverVehicleController fills it in when the component
     * joins the vehicle and clears it when the component leaves.
     *
     * Two things follow from it not being an Object, and both are easy to undo by accident:
     *
     * `Object::cast_to<MoverComponent>` cannot reach it - that helper only walks Godot's own class
     * hierarchy - so the one place that asks whether a component has a Mover behind it uses
     * `dynamic_cast` (MoverVehicleController::_component_attached). That is the correct tool here,
     * not an oversight of `CODE_STYLE.md`'s "call a method, do not name it", and replacing it with
     * `cast_to` does not compile.
     *
     * And a Mover* component now has two bases - its Vehicle* interface, which is the Object, and
     * this one, which is not - so the two do not share an address. A `dynamic_cast` (or an
     * ordinary implicit conversion) adjusts the pointer; a `reinterpret_cast`, a C-style cast, or
     * storing one of these as `Object *` and taking it back as a `MoverComponent *` by any other
     * route hands out an address that is off by the size of the Object half. It will mostly seem
     * to work, which is what makes it worth writing down. */
    class MoverComponent {
        private:
            MoverVehicleController *mover_controller = nullptr;

        public:
            virtual ~MoverComponent() = default;

            /* The component joined (a controller) or left (null) a vehicle simulated on the Mover.
             * Called by MoverVehicleController only - the vehicle hands itself over, the component
             * never takes it. */
            void set_mover_controller(MoverVehicleController *p_controller) {
                mover_controller = p_controller;
            }

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
