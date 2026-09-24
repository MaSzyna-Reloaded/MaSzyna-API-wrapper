#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleController.hpp"
#include <unordered_map>

namespace godot {
    /* VehicleController on the vendored Mover: the one class that owns a vehicle's
     * TMoverParameters. It builds it, steps it, answers from it and carries the original engine's
     * vehicle mechanics that act on it (the TDynamicObject parts of DynObj.cpp: cab, couplers,
     * neighbours). Movement along the track, transforms, tracks and wires are not here - they are
     * RailVehicleServer's. */
    class MoverVehicleController : public VehicleController {
            GDCLASS(MoverVehicleController, VehicleController)

        private:
            /* Created by _initialize_simulation(), deleted by release(). */
            TMoverParameters *mover = nullptr;

            /* Which movement integration one sub-iteration performs. The original runs the cheap
             * one for every sub-iteration but the last (DynObj.cpp:4086). */
            enum class Integration {
                FAST,
                FULL,
            };

            // Hasler speed recorder (Train.cpp:6917-6940 fTachoVelocity/fTachoVelocityJump/fTachoCount)
            double tachometer_velocity = 0.0;
            double tachometer_velocity_jump = 0.0;
            double tachometer_count = 0.0;
            double tachometer_time = 0.0;
            bool tachometer_clock_active = false;

            // coupled movers only know each other (TCoupling::Connected) - maps them back to controllers
            static std::unordered_map<const TMoverParameters *, MoverVehicleController *> controllers_by_mover;

            void initialize_mover_state();
            void _integrate(double p_delta, Integration p_integration);
            void _update_tachometer(double p_delta);
            int _resolve_coupler_end(const Variant &p_where) const;
            void _consume_coupler_sounds();

        protected:
            static void _bind_methods();
            void _initialize_simulation() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;

            double get_live_battery_voltage() const override;
            double get_tachometer_speed() const override;
            double get_tachometer_speed_jump() const override;
            double get_tachometer_clock_speed() const override;
            int get_direction_absolute() const override;
            int get_cabin() const override;
            bool get_cabin_controleable() const override;
            int get_cabin_occupied() const override;
            bool get_battery_enabled() const override;
            bool get_radio_enabled() const override;
            bool get_radio_powered() const override;
            double get_power24_voltage() const override;
            bool get_power24_available() const override;
            bool get_power110_available() const override;
            double get_current0() const override;
            double get_current1() const override;
            double get_current2() const override;
            bool get_relay_novolt() const override;
            bool get_relay_overvoltage() const override;
            bool get_relay_ground() const override;
            int get_train_damage() const override;
            int get_controller_second_position() const override;
            int get_controller_main_position() const override;
            int get_controller_joint_position() const override;
            int get_controller_main_actual_position() const override;
            int get_circuit_rlist_size() const override;

        public:
            ~MoverVehicleController() override;

            /* C++ only and unbound: the Mover is this implementation's own business. The Mover*
             * components of this vehicle reach it through here. */
            TMoverParameters *get_mover() const;

            void battery(bool p_enabled) const override;
            void cab_activation(bool p_enabled) const override;
            void cab_activation_auto() const override;
            void cab_change(int p_direction) const override;
            void main_controller_increase(int p_step = 1) const override;
            void main_controller_decrease(int p_step = 1) const override;
            void second_controller_increase(int p_step = 1) const override;
            void second_controller_decrease(int p_step = 1) const override;
            void direction_increase() const override;
            void direction_decrease() const override;
            void radio(bool p_enabled) override;

            bool is_simulation_ready() const override;
            void release() override;
            void update_state() override;
            double get_velocity() const override;
            double get_speed() const override;
            double get_mass_total() const override;
            double get_total_distance() const override;
            int get_direction() const override;
            void apply_config() override;
            double process_movement(double p_delta) override;
            void update_location() override;
            void update_neighbour(int p_end, VehicleController *p_other, int p_other_end, double p_track_distance) override;
            void compute_forces(double p_delta) override;
            void compute_movement(double p_delta) override;
            void compute_fast_movement(double p_delta) override;
            bool is_physics_active() const override;
            void couple(VehicleController *p_other, int p_end, int p_other_end, int p_coupling_type) override;
            void uncouple(int p_end) override;
            bool is_coupled(int p_end) const override;
            void coupler_connect(const Variant &p_where) override;
            void coupler_disconnect(const Variant &p_where) override;
            VehicleController *get_coupled_controller(int p_end) const override;
            int get_coupled_end(int p_end) const override;
    };
} // namespace godot
