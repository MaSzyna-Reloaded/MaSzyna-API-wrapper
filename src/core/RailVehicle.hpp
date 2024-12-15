#ifndef RAILVEHICLE_HPP
#define RAILVEHICLE_HPP

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {
    class TrainSet;

    class RailVehicle : public Node {
            GDCLASS(RailVehicle, Node);

        protected:
            static void _bind_methods();
            Ref<TrainSet> trainset;

        public:
            String id;
            RailVehicle *front = {};
            RailVehicle *back = {};

            enum Side {
                FRONT = 0,
                BACK,
            };

            RailVehicle();

            String _to_string() const;

            void couple(RailVehicle *other_vehicle, Side self_side, Side other_side);
            void couple_front(RailVehicle *other_vehicle, Side other_side);
            void couple_back(RailVehicle *other_vehicle, Side other_side);
            Ref<TrainSet> get_trainset() const;

            RailVehicle *uncouple_front();
            RailVehicle *uncouple_back();
    };
} // namespace godot

VARIANT_ENUM_CAST(RailVehicle::Side)

#endif // RAILVEHICLE_HPP
