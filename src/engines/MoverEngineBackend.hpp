#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "../mover/MoverComponent.hpp"
#include "RailVehicleEngineBackend.hpp"

namespace godot {
    class RailVehicleEngine;
    /* The engine on the vendored Mover. */
    class MoverEngineBackend : public RailVehicleEngineBackend {
        private:
            /* The Mover* component that installs this delegate - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverEngineBackend(const MoverComponent &p_owner) : owner(p_owner) {}

            bool get_main_switch_enabled(const RailVehicleEngine *p_engine) const override;
            bool get_main_switch_closable(const RailVehicleEngine *p_engine) const override;
            double get_motor_torque(const RailVehicleEngine *p_engine) const override;
            double get_wheel_torque(const RailVehicleEngine *p_engine) const override;
            double get_wheel_force(const RailVehicleEngine *p_engine) const override;
            double get_tractive_force(const RailVehicleEngine *p_engine) const override;
            double get_power(const RailVehicleEngine *p_engine) const override;
            double get_rpm_count(const RailVehicleEngine *p_engine) const override;
            double get_rpm_ratio(const RailVehicleEngine *p_engine) const override;
            double get_circuit_nmax_rpm(const RailVehicleEngine *p_engine) const override;
            int get_damage(const RailVehicleEngine *p_engine) const override;
            double get_main_switch_time(const RailVehicleEngine *p_engine) const override;
            bool get_main_no_power_pos(const RailVehicleEngine *p_engine) const override;
            bool get_motor_overload_relay_high_threshold(const RailVehicleEngine *p_engine) const override;
            double get_eimic_real(const RailVehicleEngine *p_engine) const override;
            void apply_configuration(const RailVehicleEngine *p_engine) const override;
            bool main_switch(const RailVehicleEngine *p_engine, bool p_enabled) const override;
            bool motor_overload_relay_threshold(const RailVehicleEngine *p_engine, bool p_high) const override;
            void process(const RailVehicleEngine *p_engine, double p_delta) const override;
            void fill_config(const RailVehicleEngine *p_engine, Dictionary &p_config) const override;
    };
} // namespace godot
