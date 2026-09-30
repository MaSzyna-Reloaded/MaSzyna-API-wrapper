#pragma once
#include "vehicles/base/VehicleImplementationServer.hpp"

#include <godot_cpp/classes/engine.hpp>

namespace godot {
    /* The vehicles simulated by the vendored Mover (TMoverParameters), registered with
     * VehicleServer as IMPLEMENTATION_NAME. A Mover vehicle is a rail vehicle: its step is
     * RailVehicleServer's, in the original's phases. */
    class MaszynaMoverVehicleServer : public VehicleImplementationServer {
            GDCLASS(MaszynaMoverVehicleServer, VehicleImplementationServer)

        protected:
            static void _bind_methods() {}

        public:
            static constexpr const char *IMPLEMENTATION_NAME = "maszyna_mover";

            static MaszynaMoverVehicleServer *get_instance() {
                return Object::cast_to<MaszynaMoverVehicleServer>(
                        Engine::get_singleton()->get_singleton("MaszynaMoverVehicleServer"));
            }

            void stepping_advance(const Vector<RID> &p_vehicles, double p_delta) override;
    };
} // namespace godot
