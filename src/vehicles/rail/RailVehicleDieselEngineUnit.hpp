#pragma once
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class RailVehicleDieselEngine;

    /* The diesel engine proper ("dizel_*"): its revolutions, pumps, start-up, ignition and fill -
     * a unit the diesel and the diesel-electric engines are composed of.
     *
     * RailVehicleDieselEngine is the component and carries the authored configuration; this
     * interface is the unit, with an implementation per simulation. Not a Godot class - Godot sees
     * only the engine. */
    class RailVehicleDieselEngineUnit {
        public:
            virtual ~RailVehicleDieselEngineUnit() = default;

            virtual double get_rpm() const = 0;
            virtual bool get_oil_pump_active() const = 0;
            virtual bool get_oil_pump_disabled() const = 0;
            virtual double get_oil_pump_pressure() const = 0;
            virtual bool get_fuel_pump_active() const = 0;
            virtual bool get_fuel_pump_disabled() const = 0;
            virtual bool get_heat_malfunction() const = 0;
            virtual bool get_fuel_pump_enabled() const = 0;
            virtual bool get_oil_pump_enabled() const = 0;
            virtual bool get_startup() const = 0;
            virtual bool get_ignition() const = 0;
            virtual bool get_spinup() const = 0;
            virtual double get_output_power() const = 0;
            virtual double get_torque() const = 0;
            virtual double get_fill() const = 0;
            virtual double get_max_rpm() const = 0;
            virtual void apply_configuration(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual void oil_pump(bool p_enabled) const = 0;
            virtual void fuel_pump(bool p_enabled) const = 0;
            virtual void oil_pump_switch_off(bool p_enabled) const = 0;
            virtual void fuel_pump_switch_off(bool p_enabled) const = 0;
            virtual void fill_config(Dictionary &p_config) const = 0;
    };
} // namespace godot
