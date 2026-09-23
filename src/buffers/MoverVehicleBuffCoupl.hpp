#pragma once
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
            void couple() override;
            void decouple() override;
    };
} // namespace godot
