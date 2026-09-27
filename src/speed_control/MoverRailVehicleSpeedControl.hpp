#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "RailVehicleSpeedControl.hpp"

namespace godot {
    /* RailVehicleSpeedControl on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleSpeedControl : public RailVehicleSpeedControl, public MoverComponent {
            GDCLASS(MoverRailVehicleSpeedControl, RailVehicleSpeedControl);

        private:
            static void _bind_methods();

        protected:
            void _apply_configuration() override;

        public:
            void _fill_state_dictionary(Dictionary &p_state) const override;
            bool get_active() const override;
            double get_desired_velocity() const override;
            double get_desired_power() const override;
            double get_selected_velocity() const override;
            double get_set_velocity() const override;
            void speed_control_increase() override;
            void speed_control_decrease() override;
            void speed_control_power_increase() override;
            void speed_control_power_decrease() override;
            void speed_control_button(int p_button) override;
            void speed_control_set(double p_velocity) override;
    };
} // namespace godot
