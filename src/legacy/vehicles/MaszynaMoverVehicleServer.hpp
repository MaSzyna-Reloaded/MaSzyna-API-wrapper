#pragma once
#include "vehicles/base/VehicleImplementationServer.hpp"
#include "vehicles/rail/RailVehicleController.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {
    /* The vehicles simulated by the vendored Mover (TMoverParameters), registered with
     * VehicleServer as IMPLEMENTATION_NAME. It knows how its vehicles are stepped: all of them
     * together, in the original's phases (vehicle_table::update(), DynObj.cpp:8686-8724), with
     * RailVehicleServer doing what the rail does for each of them. */
    class MaszynaMoverVehicleServer : public VehicleImplementationServer {
            GDCLASS(MaszynaMoverVehicleServer, VehicleImplementationServer)

        private:
            /* Original engine: the primary physics update rate - a frame is integrated whole, in as
             * many steps as keep each at or below it (drivermode.cpp:193-206) */
            static constexpr double PHYSICS_STEP = 0.01;
            /* Velocity change of a vehicle within one frame reported as a kick (m/s^2) */
            static constexpr double DIAGNOSTICS_MAX_ACCELERATION = 3.0;

            bool diagnostics = false;
            /* The velocity each vehicle had after the last step, for the kick diagnostics */
            HashMap<RID, double> diagnostics_velocity;
            /* Rebuilt every step, kept as members so the step allocates nothing per frame */
            Vector<RID> stepped_vehicles;
            Vector<Ref<RailVehicleController>> stepped_controllers;

            /* Diagnostics: a velocity jump within one frame is a kick - with consistent track
             * movement it comes from the forces, typically a coupler reacting to an inconsistent
             * vehicle position. */
            void _check_velocity_jumps(double p_delta);
            void _on_vehicle_freed(const RID &p_vehicle);

        protected:
            static void _bind_methods() {}

        public:
            static constexpr const char *IMPLEMENTATION_NAME = "maszyna_mover";

            static MaszynaMoverVehicleServer *get_instance() {
                return Object::cast_to<MaszynaMoverVehicleServer>(
                        Engine::get_singleton()->get_singleton("MaszynaMoverVehicleServer"));
            }

            MaszynaMoverVehicleServer();

            void stepping_advance(const Vector<RID> &p_vehicles, double p_delta) override;
    };
} // namespace godot
