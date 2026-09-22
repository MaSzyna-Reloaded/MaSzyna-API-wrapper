#pragma once
#include "VehicleElectroPneumaticDynamicBrake.hpp"

namespace godot {
    /* VehicleElectroPneumaticDynamicBrake on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleElectroPneumaticDynamicBrake : public VehicleElectroPneumaticDynamicBrake {
            GDCLASS(MoverVehicleElectroPneumaticDynamicBrake, VehicleElectroPneumaticDynamicBrake);

        private:
            static void _bind_methods();
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            double get_ed_braking_ep_delay() const override;
            double get_ep_max_brake_engagement_speed() const override;
            double get_ep_min_regenerative_braking() const override;
            double get_ep_force() const override;
            bool get_ep_fuse() const override;
            void set_ep_brake_force(int p_value) override;
            void switch_ep_fuse(bool p_value) override;
        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _fill_config_dictionary(Dictionary &p_config) const override {};
    };
} // namespace godot
