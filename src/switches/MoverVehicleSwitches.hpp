#pragma once
#include "VehicleSwitches.hpp"

namespace godot {
    /* VehicleSwitches on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleSwitches : public VehicleSwitches {
            GDCLASS(MoverVehicleSwitches, VehicleSwitches);

        private:
            static void _bind_methods();
        protected:
            void _apply_configuration() override;
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_sand_active() const override;
            void sand(bool p_active) override;
    };
} // namespace godot
