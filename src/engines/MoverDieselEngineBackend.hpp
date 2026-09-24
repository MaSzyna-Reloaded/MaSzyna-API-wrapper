#pragma once
#include "../maszyna/McZapkie/MOVER.h"
#include "VehicleDieselEngineBackend.hpp"
#include "VehicleDieselEngine.hpp"

namespace godot {
    class VehicleDieselEngine;
    /* VehicleDieselEngine on the vendored Mover. */
    class MoverDieselEngineBackend : public VehicleDieselEngineBackend {
        public:
            double get_rpm(const VehicleDieselEngine *p_engine) const override;
            bool get_oil_pump_active(const VehicleDieselEngine *p_engine) const override;
            bool get_oil_pump_disabled(const VehicleDieselEngine *p_engine) const override;
            double get_oil_pump_pressure(const VehicleDieselEngine *p_engine) const override;
            bool get_fuel_pump_active(const VehicleDieselEngine *p_engine) const override;
            bool get_fuel_pump_disabled(const VehicleDieselEngine *p_engine) const override;
            bool get_startup(const VehicleDieselEngine *p_engine) const override;
            bool get_ignition(const VehicleDieselEngine *p_engine) const override;
            bool get_spinup(const VehicleDieselEngine *p_engine) const override;
            double get_output_power(const VehicleDieselEngine *p_engine) const override;
            double get_torque(const VehicleDieselEngine *p_engine) const override;
            double get_fill(const VehicleDieselEngine *p_engine) const override;
            double get_max_rpm(const VehicleDieselEngine *p_engine) const override;
            void apply_configuration(const VehicleDieselEngine *p_engine) const override;
            void oil_pump(const VehicleDieselEngine *p_engine, bool p_enabled) const override;
            void fuel_pump(const VehicleDieselEngine *p_engine, bool p_enabled) const override;
            void fill_config(const VehicleDieselEngine *p_engine, Dictionary &p_config) const override;
    };
} // namespace godot
