#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleEngineBackend.hpp"

namespace godot {
    class VehicleEngine;
    /* The engine on the vendored Mover. */
    class MoverEngineBackend : public VehicleEngineBackend {
        public:
            bool get_main_switch_enabled(const VehicleEngine *p_engine) const override;
            bool get_main_switch_closable(const VehicleEngine *p_engine) const override;
            double get_motor_torque(const VehicleEngine *p_engine) const override;
            double get_wheel_torque(const VehicleEngine *p_engine) const override;
            double get_wheel_force(const VehicleEngine *p_engine) const override;
            double get_tractive_force(const VehicleEngine *p_engine) const override;
            bool get_compressor_enabled(const VehicleEngine *p_engine) const override;
            bool get_compressor_allowed(const VehicleEngine *p_engine) const override;
            double get_power(const VehicleEngine *p_engine) const override;
            double get_rpm_count(const VehicleEngine *p_engine) const override;
            double get_rpm_ratio(const VehicleEngine *p_engine) const override;
            double get_circuit_nmax_rpm(const VehicleEngine *p_engine) const override;
            int get_damage(const VehicleEngine *p_engine) const override;
            double get_main_switch_time(const VehicleEngine *p_engine) const override;
            bool get_main_no_power_pos(const VehicleEngine *p_engine) const override;
            void apply_configuration(const VehicleEngine *p_engine) const override;
            bool main_switch(const VehicleEngine *p_engine, bool p_enabled) const override;
            void process(const VehicleEngine *p_engine, double p_delta) const override;
            void fill_config(const VehicleEngine *p_engine, Dictionary &p_config) const override;
    };
} // namespace godot
