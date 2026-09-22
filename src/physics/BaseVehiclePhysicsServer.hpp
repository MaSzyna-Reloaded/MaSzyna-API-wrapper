#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
    /* What it means to simulate a vehicle, with no statement about how.
     *
     * The implementation this project ships is MaszynaMoverPhysicsServer, which is the only place
     * that knows the vendored TMoverParameters. Everything above this contract - the rail vehicle
     * server, the components, the game - speaks RIDs, and never learns which backend answers.
     *
     * Every handle is a RID and every callback a Callable: a public API carries no raw pointers. */
    class BaseVehiclePhysicsServer : public Object {
            GDCLASS(BaseVehiclePhysicsServer, Object)

        public:
            /* Which movement integration one sub-iteration performs. The original runs the cheap
             * one for every sub-iteration but the last (DynObj.cpp:4086). */
            enum MovementKind {
                MOVEMENT_FAST,
                MOVEMENT_FULL,
            };

            /* The two ends of a vehicle, as the original indexes its couplers. */
            enum VehicleEnd {
                VEHICLE_END_FRONT = 0,
                VEHICLE_END_REAR = 1,
            };

        protected:
            static void _bind_methods();

        public:
            virtual RID vehicle_create(
                    const String &p_type_name, const String &p_name, double p_initial_velocity, int p_cabin_number);
            virtual void vehicle_free(const RID &p_vehicle);
            virtual bool vehicle_exists(const RID &p_vehicle) const;

            /* Whether the vehicle still has anything to integrate. A braked standing vehicle stays
             * active in the original too (Mover.cpp:4603). */
            virtual bool vehicle_is_active(const RID &p_vehicle) const;

            virtual double vehicle_get_velocity(const RID &p_vehicle) const;
            virtual double vehicle_get_speed(const RID &p_vehicle) const;

            /* Where the vehicle believes it is, pushed in by whoever owns its placement. */
            virtual void vehicle_set_location(const RID &p_vehicle, const Vector3 &p_position);

            virtual void vehicle_compute_forces(const RID &p_vehicle, double p_delta);
            /* Integrates one sub-iteration and returns how far the vehicle wants to move, in the
             * rear-relative sign the placement layer uses. */
            virtual double vehicle_compute_movement(const RID &p_vehicle, double p_delta, MovementKind p_kind);

    };
} // namespace godot

VARIANT_ENUM_CAST(BaseVehiclePhysicsServer::MovementKind);
VARIANT_ENUM_CAST(BaseVehiclePhysicsServer::VehicleEnd);
