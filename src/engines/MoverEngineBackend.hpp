#pragma once
#include "VehicleEngineBackend.hpp"

namespace godot {
    /* The engine on the vendored Mover. */
    class MoverEngineBackend : public VehicleEngineBackend {
        public:
            bool get_main_switch_enabled(const TMoverParameters *p_mover) const override;
            bool get_main_switch_closable(const TMoverParameters *p_mover) const override;
            double get_motor_torque(const TMoverParameters *p_mover) const override;
            double get_wheel_torque(const TMoverParameters *p_mover) const override;
            double get_wheel_force(const TMoverParameters *p_mover) const override;
            double get_tractive_force(const TMoverParameters *p_mover) const override;
            bool get_compressor_enabled(const TMoverParameters *p_mover) const override;
            bool get_compressor_allowed(const TMoverParameters *p_mover) const override;
            double get_power(const TMoverParameters *p_mover) const override;
            double get_rpm_count(const TMoverParameters *p_mover) const override;
            double get_rpm_ratio(const TMoverParameters *p_mover) const override;
            double get_circuit_nmax_rpm(const TMoverParameters *p_mover) const override;
            int get_damage(const TMoverParameters *p_mover) const override;
            double get_main_switch_time(const TMoverParameters *p_mover) const override;
            bool get_main_no_power_pos(const TMoverParameters *p_mover) const override;
            void update_mover(const VehicleEngine *p_engine, TMoverParameters *p_mover) const override;
            void fill_config(const VehicleEngine *p_engine, const TMoverParameters *p_mover, Dictionary &p_config) const override;
    };
} // namespace godot
