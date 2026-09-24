#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleBuffCoupl.hpp"

namespace godot {
    /* VehicleBuffCoupl on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleBuffCoupl : public VehicleBuffCoupl {
            GDCLASS(MoverVehicleBuffCoupl, VehicleBuffCoupl);

        private:
            static void _bind_methods();
        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
        public:
            bool is_coupled(End p_end) const override;
            bool is_brake_hose_connected(End p_end) const override;
            bool is_main_hose_connected(End p_end) const override;
            bool is_coupling_owner(End p_end) const override;
            End get_connected_end(End p_end) const override;

            void couple() override;
            void decouple() override;
    };
} // namespace godot
