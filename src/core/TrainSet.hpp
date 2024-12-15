#ifndef TRAINSET_HPP
#define TRAINSET_HPP

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "RailVehicle.hpp"

namespace godot {
    class TrainSet : public RefCounted {
            GDCLASS(TrainSet, RefCounted);

        private:
            RailVehicle *start_vehicle;

        protected:
            static void _bind_methods();

        public:
            TrainSet();
            void _init(RailVehicle *_start_vehicle);

            RailVehicle *get_by_index(int index);
            Array to_array() const;

            RailVehicle *get_head() const;
            RailVehicle *get_tail() const;

            void attach_to_head(RailVehicle *vehicle, RailVehicle::Side side);
            void attach_to_tail(RailVehicle *vehicle, RailVehicle::Side side);
    };
} // namespace godot

#endif // TRAINSET_HPP
