#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleBuffCoupl.hpp"

namespace godot {
    /* RailVehicleBuffCoupl on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleBuffCoupl : public RailVehicleBuffCoupl, public MoverComponent {
            GDCLASS(MoverRailVehicleBuffCoupl, RailVehicleBuffCoupl);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _fill_state_dictionary(Dictionary &p_state) const override;

        public:
            bool is_coupled(End p_end) const override;
            bool is_brake_hose_connected(End p_end) const override;
            bool is_main_hose_connected(End p_end) const override;
            bool is_coupling_owner(End p_end) const override;
            End get_connected_end(End p_end) const override;
    };
} // namespace godot
