#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "VehicleRadio.hpp"

namespace godot {
    /* VehicleRadio on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleRadio : public VehicleRadio, public MoverComponent {
            GDCLASS(MoverVehicleRadio, VehicleRadio);

        private:
            /* What radio_toggled last announced - compared in the tick, since power comes and
             * goes without a command */
            bool previous_powered = false;

        protected:
            static void _bind_methods();
            void _do_process_component(double p_delta) override;

        public:
            bool get_enabled() const override;
            bool get_powered() const override;
            void radio(bool p_enabled) override;
            void radio_stop(bool p_pressed) override;
            void radio_stop_receive() override;
    };
} // namespace godot
