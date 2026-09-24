#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
    class VehicleDieselEngine;

    /* What VehicleDieselEngine needs a simulation to answer. A diesel and a Polish diesel-electric both run one, and their interfaces
     * already form a chain, so neither can inherit this from the other. */
    class VehicleDieselEngineBackend {
        public:
            virtual ~VehicleDieselEngineBackend() = default;

            virtual double get_rpm(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_oil_pump_active(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_oil_pump_disabled(const VehicleDieselEngine *p_engine) const = 0;
            virtual double get_oil_pump_pressure(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_fuel_pump_active(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_fuel_pump_disabled(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_startup(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_ignition(const VehicleDieselEngine *p_engine) const = 0;
            virtual bool get_spinup(const VehicleDieselEngine *p_engine) const = 0;
            virtual double get_output_power(const VehicleDieselEngine *p_engine) const = 0;
            virtual double get_torque(const VehicleDieselEngine *p_engine) const = 0;
            virtual double get_fill(const VehicleDieselEngine *p_engine) const = 0;
            virtual double get_max_rpm(const VehicleDieselEngine *p_engine) const = 0;
            virtual void apply_configuration(const VehicleDieselEngine *p_engine) const = 0;
            virtual void oil_pump(const VehicleDieselEngine *p_engine, bool p_enabled) const = 0;
            virtual void fuel_pump(const VehicleDieselEngine *p_engine, bool p_enabled) const = 0;
            virtual void fill_config(const VehicleDieselEngine *p_engine, Dictionary &p_config) const = 0;
    };
} // namespace godot
