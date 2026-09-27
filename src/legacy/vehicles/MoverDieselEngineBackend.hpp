#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/rail/RailVehicleDieselEngine.hpp"
#include "vehicles/rail/RailVehicleDieselEngineBackend.hpp"

namespace godot {
    class RailVehicleDieselEngine;
    /* RailVehicleDieselEngine on the vendored Mover. */
    class MoverDieselEngineBackend : public RailVehicleDieselEngineBackend {
        private:
            /* The Mover* component that installs this delegate - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverDieselEngineBackend(const MoverComponent &p_owner) : owner(p_owner) {}

            double get_rpm(const RailVehicleDieselEngine *p_engine) const override;
            bool get_oil_pump_active(const RailVehicleDieselEngine *p_engine) const override;
            bool get_oil_pump_disabled(const RailVehicleDieselEngine *p_engine) const override;
            double get_oil_pump_pressure(const RailVehicleDieselEngine *p_engine) const override;
            bool get_fuel_pump_active(const RailVehicleDieselEngine *p_engine) const override;
            bool get_fuel_pump_disabled(const RailVehicleDieselEngine *p_engine) const override;
            bool get_heat_malfunction(const RailVehicleDieselEngine *p_engine) const override;
            bool get_fuel_pump_enabled(const RailVehicleDieselEngine *p_engine) const override;
            bool get_oil_pump_enabled(const RailVehicleDieselEngine *p_engine) const override;
            bool get_startup(const RailVehicleDieselEngine *p_engine) const override;
            bool get_ignition(const RailVehicleDieselEngine *p_engine) const override;
            bool get_spinup(const RailVehicleDieselEngine *p_engine) const override;
            double get_output_power(const RailVehicleDieselEngine *p_engine) const override;
            double get_torque(const RailVehicleDieselEngine *p_engine) const override;
            double get_fill(const RailVehicleDieselEngine *p_engine) const override;
            double get_max_rpm(const RailVehicleDieselEngine *p_engine) const override;
            void apply_configuration(const RailVehicleDieselEngine *p_engine) const override;
            void oil_pump(const RailVehicleDieselEngine *p_engine, bool p_enabled) const override;
            void fuel_pump(const RailVehicleDieselEngine *p_engine, bool p_enabled) const override;
            void oil_pump_switch_off(const RailVehicleDieselEngine *p_engine, bool p_enabled) const override;
            void fuel_pump_switch_off(const RailVehicleDieselEngine *p_engine, bool p_enabled) const override;
            void fill_config(const RailVehicleDieselEngine *p_engine, Dictionary &p_config) const override;
    };
} // namespace godot
