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

            virtual double get_rpm(const TMoverParameters *p_mover) const = 0;
            virtual bool get_oil_pump_active(const TMoverParameters *p_mover) const = 0;
            virtual bool get_oil_pump_disabled(const TMoverParameters *p_mover) const = 0;
            virtual double get_oil_pump_pressure(const TMoverParameters *p_mover) const = 0;
            virtual bool get_fuel_pump_active(const TMoverParameters *p_mover) const = 0;
            virtual bool get_fuel_pump_disabled(const TMoverParameters *p_mover) const = 0;
            virtual bool get_startup(const TMoverParameters *p_mover) const = 0;
            virtual bool get_ignition(const TMoverParameters *p_mover) const = 0;
            virtual bool get_spinup(const TMoverParameters *p_mover) const = 0;
            virtual double get_output_power(const TMoverParameters *p_mover) const = 0;
            virtual double get_torque(const TMoverParameters *p_mover) const = 0;
            virtual double get_fill(const TMoverParameters *p_mover) const = 0;
            virtual double get_max_rpm(const TMoverParameters *p_mover) const = 0;
            virtual void update_mover(const VehicleDieselEngine *p_engine, TMoverParameters *p_mover) const = 0;
            virtual void fill_config(const VehicleDieselEngine *p_engine, const TMoverParameters *p_mover, Dictionary &p_config) const = 0;
    };
} // namespace godot
