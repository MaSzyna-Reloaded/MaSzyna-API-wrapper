#pragma once
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class RailVehicleDieselEngine;

    /* What RailVehicleDieselEngine needs a simulation to answer. A diesel and a Polish diesel-electric both run one, and
     * their interfaces already form a chain, so neither can inherit this from the other. */
    class RailVehicleDieselEngineBackend {
        public:
            virtual ~RailVehicleDieselEngineBackend() = default;

            virtual double get_rpm(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_oil_pump_active(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_oil_pump_disabled(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual double get_oil_pump_pressure(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_fuel_pump_active(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_fuel_pump_disabled(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_heat_malfunction(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_fuel_pump_enabled(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_oil_pump_enabled(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_startup(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_ignition(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual bool get_spinup(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual double get_output_power(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual double get_torque(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual double get_fill(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual double get_max_rpm(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual void apply_configuration(const RailVehicleDieselEngine *p_engine) const = 0;
            virtual void oil_pump(const RailVehicleDieselEngine *p_engine, bool p_enabled) const = 0;
            virtual void fuel_pump(const RailVehicleDieselEngine *p_engine, bool p_enabled) const = 0;
            virtual void oil_pump_switch_off(const RailVehicleDieselEngine *p_engine, bool p_enabled) const = 0;
            virtual void fuel_pump_switch_off(const RailVehicleDieselEngine *p_engine, bool p_enabled) const = 0;
            virtual void fill_config(const RailVehicleDieselEngine *p_engine, Dictionary &p_config) const = 0;
    };
} // namespace godot
