#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class VehicleEngine;

    /* What an engine needs a simulation to answer, with no Godot class in it.
     *
     * VehicleEngine is the public contract and carries the vehicle's authored configuration; this
     * is the other half - the live values and the two operations that reach the simulation. They
     * are kept apart so the engine interfaces name no backend at all, and so that engine kinds
     * whose interfaces already form a chain can still share one implementation. */
    class VehicleEngineBackend {
        public:
            virtual ~VehicleEngineBackend() = default;

            virtual bool get_main_switch_enabled(const TMoverParameters *p_mover) const = 0;
            virtual bool get_main_switch_closable(const TMoverParameters *p_mover) const = 0;
            virtual double get_motor_torque(const TMoverParameters *p_mover) const = 0;
            virtual double get_wheel_torque(const TMoverParameters *p_mover) const = 0;
            virtual double get_wheel_force(const TMoverParameters *p_mover) const = 0;
            virtual double get_tractive_force(const TMoverParameters *p_mover) const = 0;
            virtual bool get_compressor_enabled(const TMoverParameters *p_mover) const = 0;
            virtual bool get_compressor_allowed(const TMoverParameters *p_mover) const = 0;
            virtual double get_power(const TMoverParameters *p_mover) const = 0;
            virtual double get_rpm_count(const TMoverParameters *p_mover) const = 0;
            virtual double get_rpm_ratio(const TMoverParameters *p_mover) const = 0;
            virtual double get_circuit_nmax_rpm(const TMoverParameters *p_mover) const = 0;
            virtual int get_damage(const TMoverParameters *p_mover) const = 0;
            virtual double get_main_switch_time(const TMoverParameters *p_mover) const = 0;
            virtual bool get_main_no_power_pos(const TMoverParameters *p_mover) const = 0;
            virtual void update_mover(const VehicleEngine *p_engine, TMoverParameters *p_mover) const = 0;
            virtual void fill_config(const VehicleEngine *p_engine, const TMoverParameters *p_mover, Dictionary &p_config) const = 0;
    };
} // namespace godot
