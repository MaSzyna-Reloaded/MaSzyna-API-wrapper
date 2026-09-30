#pragma once
#include "vehicles/rail/RailVehicleComponent.hpp"

namespace godot {
    /* The driver's master controller of a vehicle, whatever it drives: the positions of its main
     * and second controller and how fast they step (LoadFIZ_Cntrl, Mover.cpp:10837-10869). A control
     * car has one without an engine - EN57's ra reverses and steps its controller like its motor car. */
    class RailVehicleMasterController : public RailVehicleComponent {
            GDCLASS(RailVehicleMasterController, RailVehicleComponent);

        public:
            int get_component_type() const override {
                return RailVehicleComponentType::COMPONENT_MASTER_CONTROLLER;
            }

        private:
            static void _bind_methods();

            /* MCPN */
            int main_position_count = 0;
            /* SCPN */
            int second_position_count = 0;
            /* DirChangeMaxPos: the highest main position the reverser may be moved at */
            int direction_change_max_position = 0;
            /* CoupledCtrl: the second controller continues the main one */
            bool coupled_controllers = false;
            /* IniCDelay, SCDelay, SCDDelay [s] */
            double initial_delay = 0.0;
            double step_delay = 0.0;
            double step_down_delay = 0.0;

        public:
            void set_main_position_count(int p_value);
            int get_main_position_count() const;
            void set_second_position_count(int p_value);
            int get_second_position_count() const;
            void set_direction_change_max_position(int p_value);
            int get_direction_change_max_position() const;
            void set_coupled_controllers(bool p_value);
            bool get_coupled_controllers() const;
            void set_initial_delay(double p_value);
            double get_initial_delay() const;
            void set_step_delay(double p_value);
            double get_step_delay() const;
            void set_step_down_delay(double p_value);
            double get_step_down_delay() const;
    };
} // namespace godot
