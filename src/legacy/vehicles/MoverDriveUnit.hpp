#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleDriveUnit.hpp"

namespace godot {
    /* The drive on the vendored Mover. */
    class MoverDriveUnit : public RailVehicleDriveUnit {
        private:
            /* The Mover* component that owns this unit - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverDriveUnit(const MoverComponent &p_owner) : owner(p_owner) {}

            bool get_main_switch_enabled() const override;
            bool get_main_switch_closable() const override;
            double get_motor_torque() const override;
            double get_wheel_torque() const override;
            double get_wheel_force() const override;
            double get_tractive_force() const override;
            double get_power() const override;
            double get_rpm_count() const override;
            double get_rpm_ratio() const override;
            double get_circuit_nmax_rpm() const override;
            int get_damage() const override;
            double get_main_switch_time() const override;
            bool get_main_no_power_pos() const override;
            bool get_motor_overload_relay_high_threshold() const override;
            double get_eimic_real() const override;
            void apply_configuration(const RailVehicleEngine *p_engine) const override;
            bool main_switch(bool p_enabled) const override;
            bool motor_overload_relay_threshold(bool p_high) const override;
            void process(const RailVehicleEngine *p_engine, double p_delta) const override;
            void fill_config(Dictionary &p_config) const override;
    };
} // namespace godot
