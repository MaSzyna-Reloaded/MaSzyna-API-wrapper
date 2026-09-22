#pragma once
#include "VehicleElectricTraction.hpp"

namespace godot {
    /* The traction motors on the vendored Mover - shared by every engine that has them. */
    class MoverElectricTraction : public VehicleElectricTraction {
        public:
            double get_motor_current(const TMoverParameters *p_mover) const override;
            double get_circuit_imax(const TMoverParameters *p_mover) const override;
            bool get_dynamic_brake_active(const TMoverParameters *p_mover) const override;
            bool get_fuse_active(const TMoverParameters *p_mover) const override;
            bool get_motor_connectors_open(const TMoverParameters *p_mover) const override;
            void reset_fuse(TMoverParameters *p_mover) const override;
            void open_motor_connectors(TMoverParameters *p_mover, bool p_open) const override;
    };
} // namespace godot
