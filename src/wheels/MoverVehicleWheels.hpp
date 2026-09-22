#pragma once
#include "VehicleWheels.hpp"

namespace godot {
    /* VehicleWheels on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverVehicleWheels : public VehicleWheels {
            GDCLASS(MoverVehicleWheels, VehicleWheels);

        private:
            static void _bind_methods();
        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            double get_angle_front_deg() const override;
            double get_angle_powered_deg() const override;
            double get_angle_rear_deg() const override;
            double get_rotation_speed_rps() const override;
            double get_rotation_acceleration_rps2() const override;
            bool get_slipping() const override;
            double get_flat() const override;
        private:
            double wheel_angle_front_deg = 0.0;
            double wheel_angle_powered_deg = 0.0;
            double wheel_angle_rear_deg = 0.0;
        protected:
            void _do_update_internal_mover(TMoverParameters *p_mover) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _do_process_mover(TMoverParameters *p_mover, double p_delta) override;
    };
} // namespace godot
