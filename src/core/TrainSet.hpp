#ifndef TRAINSET_HPP
#define TRAINSET_HPP

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
    };
} // namespace godot

#endif // TRAINSET_HPP
