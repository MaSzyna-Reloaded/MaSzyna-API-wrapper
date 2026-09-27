#include "VehicleNeighbour.hpp"

namespace godot {
    void VehicleNeighbour::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_vehicle_rid", "vehicle_rid"), &VehicleNeighbour::set_vehicle_rid);
        ClassDB::bind_method(D_METHOD("get_vehicle_rid"), &VehicleNeighbour::get_vehicle_rid);
        ADD_PROPERTY(PropertyInfo(Variant::RID, "vehicle_rid"), "set_vehicle_rid", "get_vehicle_rid");
        ClassDB::bind_method(D_METHOD("set_end", "end"), &VehicleNeighbour::set_end);
        ClassDB::bind_method(D_METHOD("get_end"), &VehicleNeighbour::get_end);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "end"), "set_end", "get_end");
        ClassDB::bind_method(D_METHOD("set_distance", "distance"), &VehicleNeighbour::set_distance);
        ClassDB::bind_method(D_METHOD("get_distance"), &VehicleNeighbour::get_distance);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance"), "set_distance", "get_distance");
    }

    void VehicleNeighbour::set_vehicle_rid(const RID &p_vehicle_rid) {
        vehicle_rid = p_vehicle_rid;
    }

    RID VehicleNeighbour::get_vehicle_rid() const {
        return vehicle_rid;
    }

    void VehicleNeighbour::set_end(const int p_end) {
        end = p_end;
    }

    int VehicleNeighbour::get_end() const {
        return end;
    }

    void VehicleNeighbour::set_distance(const double p_distance) {
        distance = p_distance;
    }

    double VehicleNeighbour::get_distance() const {
        return distance;
    }
} // namespace godot
