#pragma once

namespace godot {
    class VehicleEngine;
    /* Electric traction motors, as a contract rather than a class in the hierarchy.
     *
     * A diesel-electric has them without being fed from the catenary, and a catenary-fed
     * locomotive has them too - and GDCLASS gives one base class, so neither interface can
     * inherit them from the other. Both declare these five values and forward to one shared
     * implementation instead, which is the only place the backend is read.
     *
     * Not a registered Godot class: it never stands on its own, it is what the engine interfaces
     * delegate to. */
    class VehicleElectricTraction {
        public:
            virtual ~VehicleElectricTraction() = default;

            /* Motor current (Im) */
            virtual double get_motor_current(const VehicleEngine *p_engine) const = 0;
            /* Current limit of the traction circuit (Imax) */
            virtual double get_circuit_imax(const VehicleEngine *p_engine) const = 0;
            /* Rheostatic / regenerative braking is engaged */
            virtual bool get_dynamic_brake_active(const VehicleEngine *p_engine) const = 0;
            /* The motor overload fuse has tripped */
            virtual bool get_fuse_active(const VehicleEngine *p_engine) const = 0;
            /* The line contactors are open */
            virtual bool get_motor_connectors_open(const VehicleEngine *p_engine) const = 0;

            virtual void reset_fuse(const VehicleEngine *p_engine) const = 0;
            virtual void open_motor_connectors(const VehicleEngine *p_engine, bool p_open) const = 0;
    };
} // namespace godot
