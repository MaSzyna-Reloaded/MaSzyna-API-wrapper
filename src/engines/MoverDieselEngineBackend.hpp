#pragma once
#include "VehicleDieselEngineBackend.hpp"
#include "VehicleDieselEngine.hpp"

namespace godot {
    /* VehicleDieselEngine on the vendored Mover. */
    class MoverDieselEngineBackend : public VehicleDieselEngineBackend {
        public:
            double get_rpm(const TMoverParameters *p_mover) const override;
            bool get_oil_pump_active(const TMoverParameters *p_mover) const override;
            bool get_oil_pump_disabled(const TMoverParameters *p_mover) const override;
            double get_oil_pump_pressure(const TMoverParameters *p_mover) const override;
            bool get_fuel_pump_active(const TMoverParameters *p_mover) const override;
            bool get_fuel_pump_disabled(const TMoverParameters *p_mover) const override;
            bool get_startup(const TMoverParameters *p_mover) const override;
            bool get_ignition(const TMoverParameters *p_mover) const override;
            bool get_spinup(const TMoverParameters *p_mover) const override;
            double get_output_power(const TMoverParameters *p_mover) const override;
            double get_torque(const TMoverParameters *p_mover) const override;
            double get_fill(const TMoverParameters *p_mover) const override;
            double get_max_rpm(const TMoverParameters *p_mover) const override;
            void update_mover(const VehicleDieselEngine *p_engine, TMoverParameters *p_mover) const override;
            void fill_config(const VehicleDieselEngine *p_engine, const TMoverParameters *p_mover, Dictionary &p_config) const override;
    };
} // namespace godot
