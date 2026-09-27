#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {
    /* The nearest vehicle along the route from one end of a vehicle
     * (RailVehicleServer::vehicle_find_vehicle(); the original's neighbour_data, DynObj.h). */
    class VehicleNeighbour : public RefCounted {
            GDCLASS(VehicleNeighbour, RefCounted)

        private:
            /* the vehicle found */
            RID vehicle_rid = RID();
            /* its end facing the one searched from (0 front, 1 rear) */
            int end = 0;
            /* between the two vehicles' ends [m] */
            double distance = 0.0;

        protected:
            static void _bind_methods();

        public:
            void set_vehicle_rid(const RID &p_vehicle_rid);
            RID get_vehicle_rid() const;
            void set_end(int p_end);
            int get_end() const;
            void set_distance(double p_distance);
            double get_distance() const;
    };
} // namespace godot
