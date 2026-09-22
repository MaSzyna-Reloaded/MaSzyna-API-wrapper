#pragma once
#include "VehicleElectricTraction.hpp"

namespace godot {
    class VehicleEngine;
    /* The traction motors on the vendored Mover - shared by every engine that has them. */
    class MoverElectricTraction : public VehicleElectricTraction {
        public:
            double get_motor_current(const VehicleEngine *p_engine) const override;
            double get_circuit_imax(const VehicleEngine *p_engine) const override;
            bool get_dynamic_brake_active(const VehicleEngine *p_engine) const override;
            bool get_fuse_active(const VehicleEngine *p_engine) const override;
            bool get_motor_connectors_open(const VehicleEngine *p_engine) const override;
            void reset_fuse(const VehicleEngine *p_engine) const override;
            void open_motor_connectors(const VehicleEngine *p_engine, bool p_open) const override;
    };
} // namespace godot
