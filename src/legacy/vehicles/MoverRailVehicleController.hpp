#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "vehicles/rail/RailVehicleController.hpp"

namespace godot {
    /* VehicleController on the vendored Mover: the one class that owns a vehicle's
     * TMoverParameters. It builds it, steps it, answers from it and carries the original engine's
     * vehicle mechanics that act on it (the TDynamicObject parts of DynObj.cpp: cab, couplers,
     * neighbours). Movement along the track, transforms, tracks and wires are not here - they are
     * RailVehicleServer's. */
    class MoverRailVehicleController : public RailVehicleController {
            GDCLASS(MoverRailVehicleController, RailVehicleController)

        private:
            /* This vehicle's Mover, which MaszynaMoverVehicleServer owns: taken in
             * _initialize_simulation(), handed back in release(). mover_vehicle is the handle it
             * was created for - the controller's own may change meanwhile. */
            TMoverParameters *mover = nullptr;
            RID mover_vehicle;
            /* The MaszynaMoverVehicleServer the Mover came from, by id: at shutdown it is freed -
             * and with it every Mover - before the controllers it served */
            ObjectID mover_implementation;
            /* That server, or null once it is gone - and the Mover with it */
            class MaszynaMoverVehicleServer *_mover_implementation() const;
            /* The controller a coupled Mover belongs to - coupled Movers only know each other
             * (TCoupling::Connected) */
            Ref<RailVehicleController> _controller_of(const TMoverParameters *p_mover) const;

            /* Which movement integration one sub-iteration performs. The original runs the cheap
             * one for every sub-iteration but the last (DynObj.cpp:4086). */
            enum class Integration {
                FAST,
                FULL,
            };

            void initialize_mover_state();
            void _integrate(double p_delta, Integration p_integration);
            CouplerEnd _resolve_coupler_end(const Variant &p_where) const;
            /* The coupled vehicle as this end's neighbour; false when the end is not coupled */
            bool _neighbour_from_coupler(CouplerEnd p_end);
            void _consume_coupler_events();

        protected:
            static void _bind_methods();
            void _initialize_simulation() override;
            void _fill_config_dictionary(Dictionary &p_config) const override;

            int get_direction_absolute() const override;
            int get_cabin_occupied() const override;
            int get_train_damage() const override;
            double get_mass_reduced() const override;
            bool get_coupler_stretched() const override;

        public:
            MoverRailVehicleController();
            ~MoverRailVehicleController() override;

            /* C++ only and unbound: the Mover is this implementation's own business. */
            TMoverParameters *get_mover() const;

            void cab_activation(bool p_enabled) const override;
            void cab_activation_auto() const override;
            void cab_change(int p_direction) const override;
            void ground_relay_reset() const override;
            void antislip() const override;
            void main_controller_increase(int p_step = 1) const override;
            void main_controller_decrease(int p_step = 1) const override;
            void second_controller_increase(int p_step = 1) const override;
            void second_controller_decrease(int p_step = 1) const override;
            void direction_increase() const override;
            void direction_decrease() const override;

            bool is_simulation_ready() const override;
            void release() override;
            void update_state() override;
            double get_velocity() const override;
            double get_speed() const override;
            double get_acceleration() const override;
            double get_mass_total() const override;
            double get_total_distance() const override;
            int get_direction() const override;
            void apply_config() override;
            double process_movement(double p_delta) override;
            void update_location() override;
            void update_neighbour(
                    CouplerEnd p_end, const Ref<RailVehicleController> &p_other, CouplerEnd p_other_end,
                    double p_track_distance) override;
            void clear_neighbour(CouplerEnd p_end) override;
            void compute_forces(double p_delta) override;
            void compute_movement(double p_delta) override;
            void compute_fast_movement(double p_delta) override;
            bool is_physics_active() const override;
            void wake() override;
            void
            couple(const Ref<RailVehicleController> &p_other, CouplerEnd p_end, CouplerEnd p_other_end,
                   BitField<CouplingFlags> p_coupling) override;
            void uncouple(CouplerEnd p_end) override;
            bool is_coupled(CouplerEnd p_end) const override;
            bool is_coupled_by(CouplerEnd p_end, BitField<CouplingFlags> p_flags) const override;
            void coupler_connect(const Variant &p_where) override;
            void coupler_disconnect(const Variant &p_where) override;
            Ref<RailVehicleController> get_coupled_controller(CouplerEnd p_end) const override;
            CouplerEnd get_coupled_end(CouplerEnd p_end) const override;
    };
} // namespace godot
