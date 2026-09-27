#pragma once
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class RailVehicleEngine;

    /* What an engine needs a simulation to answer, with no Godot class in it.
     *
     * RailVehicleEngine is the public contract and carries the vehicle's authored configuration; this
     * is the other half - the live values and the two operations that reach the simulation. They
     * are kept apart so the engine interfaces name no backend at all, and so that engine kinds
     * whose interfaces already form a chain can still share one implementation. */
    class RailVehicleEngineBackend {
        public:
            virtual ~RailVehicleEngineBackend() = default;

            virtual bool get_main_switch_enabled(const RailVehicleEngine *p_engine) const = 0;
            virtual bool get_main_switch_closable(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_motor_torque(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_wheel_torque(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_wheel_force(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_tractive_force(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_power(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_rpm_count(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_rpm_ratio(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_circuit_nmax_rpm(const RailVehicleEngine *p_engine) const = 0;
            virtual int get_damage(const RailVehicleEngine *p_engine) const = 0;
            virtual double get_main_switch_time(const RailVehicleEngine *p_engine) const = 0;
            virtual bool get_main_no_power_pos(const RailVehicleEngine *p_engine) const = 0;
            virtual bool get_motor_overload_relay_high_threshold(const RailVehicleEngine *p_engine) const = 0;
            /* The power the EIM controller actually asks for, 0..1, braking below (eimic_real) */
            virtual double get_eimic_real(const RailVehicleEngine *p_engine) const = 0;
            virtual void apply_configuration(const RailVehicleEngine *p_engine) const = 0;
            virtual bool main_switch(const RailVehicleEngine *p_engine, bool p_enabled) const = 0;
            /* The motor overload relay's high threshold, or the shunting mode of an engine that has
             * one (CurrentSwitch(), Mover.cpp:805) */
            virtual bool motor_overload_relay_threshold(const RailVehicleEngine *p_engine, bool p_high) const = 0;
            /* One simulation step of the engine, for the work the simulation leaves to its owner. */
            virtual void process(const RailVehicleEngine *p_engine, double p_delta) const = 0;
            virtual void fill_config(const RailVehicleEngine *p_engine, Dictionary &p_config) const = 0;
    };
} // namespace godot
