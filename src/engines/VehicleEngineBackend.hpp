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

            virtual bool get_main_switch_enabled(const VehicleEngine *p_engine) const = 0;
            virtual bool get_main_switch_closable(const VehicleEngine *p_engine) const = 0;
            virtual double get_motor_torque(const VehicleEngine *p_engine) const = 0;
            virtual double get_wheel_torque(const VehicleEngine *p_engine) const = 0;
            virtual double get_wheel_force(const VehicleEngine *p_engine) const = 0;
            virtual double get_tractive_force(const VehicleEngine *p_engine) const = 0;
            virtual bool get_compressor_enabled(const VehicleEngine *p_engine) const = 0;
            virtual bool get_compressor_allowed(const VehicleEngine *p_engine) const = 0;
            virtual double get_power(const VehicleEngine *p_engine) const = 0;
            virtual double get_rpm_count(const VehicleEngine *p_engine) const = 0;
            virtual double get_rpm_ratio(const VehicleEngine *p_engine) const = 0;
            virtual double get_circuit_nmax_rpm(const VehicleEngine *p_engine) const = 0;
            virtual int get_damage(const VehicleEngine *p_engine) const = 0;
            virtual double get_main_switch_time(const VehicleEngine *p_engine) const = 0;
            virtual bool get_main_no_power_pos(const VehicleEngine *p_engine) const = 0;
            virtual void apply_configuration(const VehicleEngine *p_engine) const = 0;
            virtual bool main_switch(const VehicleEngine *p_engine, bool p_enabled) const = 0;
            /* One simulation step of the engine, for the work the simulation leaves to its owner. */
            virtual void process(const VehicleEngine *p_engine, double p_delta) const = 0;
            virtual void fill_config(const VehicleEngine *p_engine, Dictionary &p_config) const = 0;
    };
} // namespace godot
