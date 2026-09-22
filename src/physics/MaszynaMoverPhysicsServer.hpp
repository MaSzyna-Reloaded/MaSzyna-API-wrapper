#pragma once

#include "BaseVehiclePhysicsServer.hpp"

#include "../maszyna/McZapkie/MOVER.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/templates/hash_map.hpp>

#include <unordered_map>

namespace godot {
    /* The vehicle physics contract, implemented on the vendored TMoverParameters.
     *
     * This is the only class in the project that owns a Mover. Everything above it holds a RID,
     * so swapping or adding a simulation backend touches nothing but this file and the contract
     * it implements. */
    class MaszynaMoverPhysicsServer : public BaseVehiclePhysicsServer {
            GDCLASS(MaszynaMoverPhysicsServer, BaseVehiclePhysicsServer)

        private:
            struct VehicleRecord {
                    TMoverParameters *mover = nullptr;
            };

            HashMap<RID, VehicleRecord> vehicles;
            int64_t next_vehicle_id = 0;

        protected:
            static void _bind_methods();

        public:
            ~MaszynaMoverPhysicsServer() override;

            static MaszynaMoverPhysicsServer *get_instance() {
                return Object::cast_to<MaszynaMoverPhysicsServer>(
                        Engine::get_singleton()->get_singleton("MaszynaMoverPhysicsServer"));
            }

            RID vehicle_create(
                    const String &p_type_name, const String &p_name, double p_initial_velocity,
                    int p_cabin_number) override;
            void vehicle_free(const RID &p_vehicle) override;
            bool vehicle_exists(const RID &p_vehicle) const override;
            bool vehicle_is_active(const RID &p_vehicle) const override;
            double vehicle_get_velocity(const RID &p_vehicle) const override;
            double vehicle_get_speed(const RID &p_vehicle) const override;
            void vehicle_set_location(const RID &p_vehicle, const Vector3 &p_position) override;
            void vehicle_compute_forces(const RID &p_vehicle, double p_delta) override;
            double vehicle_compute_movement(const RID &p_vehicle, double p_delta, MovementKind p_kind) override;

            /* C++ only, and deliberately unbound: the Mover is this backend's private business and
             * has no place in a public API. It is reachable from the components while they are
             * still written against it - see TODO.md, the #184 stages that end that. */
            TMoverParameters *vehicle_get_mover(const RID &p_vehicle) const;
    };
} // namespace godot
