#include "MaszynaMoverVehicleServer.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"

namespace godot {
    void MaszynaMoverVehicleServer::stepping_advance(const Vector<RID> &p_vehicles, const double p_delta) {
        RailVehicleServer *rail_vehicles = RailVehicleServer::get_instance();
        ERR_FAIL_NULL(rail_vehicles);
        rail_vehicles->stepping_advance(p_vehicles, p_delta);
    }
} // namespace godot
