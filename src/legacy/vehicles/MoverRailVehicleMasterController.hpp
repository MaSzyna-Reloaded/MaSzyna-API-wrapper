#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleMasterController.hpp"

namespace godot {
    /* RailVehicleMasterController on the vendored Mover - the only class here that knows TMoverParameters. */
    class MoverRailVehicleMasterController : public RailVehicleMasterController, public MoverComponent {
            GDCLASS(MoverRailVehicleMasterController, RailVehicleMasterController);

        protected:
            /* The vehicle's Mover, taken when its simulation starts and dropped before it is freed */
            void _implementation_changed() override {
                take_mover(
                        get_implementation(),
                        train_controller_node != nullptr ? train_controller_node->get_rid() : RID());
            }

        private:
            static void _bind_methods();

            // Hasler speed recorder (Train.cpp:6917-6940 fTachoVelocity/fTachoVelocityJump/fTachoCount)
            double tachometer_velocity = 0.0;
            double tachometer_velocity_jump = 0.0;
            double tachometer_count = 0.0;
            double tachometer_time = 0.0;
            bool tachometer_clock_active = false;
            /* TTrain::m_distancecounter (Train.h:904) - metres since activation, -1 while off */
            static constexpr double DISTANCE_COUNTER_OFF = -1.0;
            double distance_counter = DISTANCE_COUNTER_OFF;

        protected:
            void _apply_configuration() override;
            void _do_process_component(double p_delta) override;
            void _fill_config_dictionary(Dictionary &p_config) const override;
            void _fill_state_dictionary(Dictionary &p_state) const override;

        public:
            int get_main_position() const override;
            int get_second_position() const override;
            int get_joint_position() const override;
            int get_main_actual_position() const override;
            int get_second_actual_position() const override;
            bool get_main_delayed() const override;
            int get_main_no_power_position() const override;
            int get_cabin() const override;
            bool get_cabin_controleable() const override;
            double get_tachometer_speed() const override;
            double get_tachometer_speed_jump() const override;
            double get_tachometer_clock_speed() const override;
            double get_distance_counter() const override;
            void distance_counter_activate(bool p_pressed) override;
    };
} // namespace godot
