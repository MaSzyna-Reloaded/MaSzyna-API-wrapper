#pragma once
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class RailVehicleEngine;

    /* The drive of an engine ("typ napedu", MOVER.h:1569): main switch, torque and force, power,
     * revolutions, the overload relay and the EIM controller - a unit every engine kind has.
     *
     * RailVehicleEngine is the component and carries the vehicle's authored configuration; this
     * interface is the unit it is composed of, with an implementation per simulation. Not a Godot
     * class - Godot sees only the engine. */
    class RailVehicleDriveUnit {
        public:
            virtual ~RailVehicleDriveUnit() = default;

            virtual bool get_main_switch_enabled() const = 0;
            virtual bool get_main_switch_closable() const = 0;
            virtual double get_motor_torque() const = 0;
            virtual double get_wheel_torque() const = 0;
            virtual double get_wheel_force() const = 0;
            virtual double get_tractive_force() const = 0;
            virtual double get_power() const = 0;
            virtual double get_rpm_count() const = 0;
            virtual double get_rpm_ratio() const = 0;
            virtual double get_circuit_nmax_rpm() const = 0;
            virtual int get_damage() const = 0;
            virtual double get_main_switch_time() const = 0;
            virtual bool get_main_no_power_pos() const = 0;
            virtual bool get_motor_overload_relay_high_threshold() const = 0;
            /* The power the EIM controller actually asks for, 0..1, braking below (eimic_real) */
            virtual double get_eimic_real() const = 0;
            virtual void apply_configuration(const RailVehicleEngine *p_engine) const = 0;
            virtual bool main_switch(bool p_enabled) const = 0;
            /* The motor overload relay's high threshold, or the shunting mode of an engine that has
             * one (CurrentSwitch(), Mover.cpp:805) */
            virtual bool motor_overload_relay_threshold(bool p_high) const = 0;
            /* One simulation step of the engine, for the work the simulation leaves to its owner. */
            virtual void process(const RailVehicleEngine *p_engine, double p_delta) const = 0;
            virtual void fill_config(Dictionary &p_config) const = 0;
    };
} // namespace godot
