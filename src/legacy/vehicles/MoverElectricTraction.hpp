#pragma once
#include "legacy/maszyna-mover/McZapkie/MOVER.h"
#include "legacy/vehicles/MoverComponent.hpp"
#include "vehicles/base/VehicleElectricTraction.hpp"

namespace godot {
    class RailVehicleEngine;
    /* The traction motors on the vendored Mover - shared by every engine that has them. */
    class MoverElectricTraction : public VehicleElectricTraction {
        private:
            /* The Mover* component that installs this delegate - it reaches the Mover through it. */
            const MoverComponent &owner;

        public:
            explicit MoverElectricTraction(const MoverComponent &p_owner) : owner(p_owner) {}

            double get_motor_current(const RailVehicleEngine *p_engine) const override;
            double get_circuit_imax(const RailVehicleEngine *p_engine) const override;
            bool get_dynamic_brake_active(const RailVehicleEngine *p_engine) const override;
            bool get_fuse_active(const RailVehicleEngine *p_engine) const override;
            bool get_motor_connectors_open(const RailVehicleEngine *p_engine) const override;
            bool is_line_contactor_closed(const RailVehicleEngine *p_engine) const override;
            bool is_pressure_switch_tripped(const RailVehicleEngine *p_engine) const override;
            void reset_fuse(const RailVehicleEngine *p_engine) const override;
            void open_motor_connectors(const RailVehicleEngine *p_engine, bool p_open) const override;
    };
} // namespace godot
