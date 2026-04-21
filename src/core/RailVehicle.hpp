#ifndef RAILVEHICLE_HPP
#define RAILVEHICLE_HPP

#include "macros.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {
    class LegacyRailVehicleModule;
    class TrainSet;
    class RailVehicle : public Resource {
            GDCLASS(RailVehicle, Resource);

        protected:
            static void _bind_methods();
            Ref<TrainSet> trainset;
            bool _dirty = false;
            bool _dirty_prop = false;
            Array modules;
            Node *runtime_host = nullptr;
            bool runtime_initialized = false;

        public:
            String id;
            RailVehicle *front = {};
            RailVehicle *back = {};

            enum Side {
                FRONT = 0,
                BACK,
            };

        protected:
            virtual void _on_coupled(RailVehicle *other_vehicle, Side self_side, Side other_side);
            virtual void _on_uncoupled(RailVehicle *other_vehicle, Side self_side, Side other_side);
            virtual void _enter_tree();
            virtual void _ready();
            virtual void _exit_tree();
            virtual void _process(double delta);
            virtual void _physics_process(double delta);

        public:
            RailVehicle();
            ~RailVehicle() override;

            MAKE_MEMBER_GS_DIRTY(float, mass, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, max_velocity, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, length, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, width, 0.0);
            MAKE_MEMBER_GS_DIRTY(float, height, 0.0);

            String _to_string() const;

            void couple(RailVehicle *other_vehicle, Side self_side, Side other_side);
            void couple_front(RailVehicle *other_vehicle, Side other_side);
            void couple_back(RailVehicle *other_vehicle, Side other_side);
            RailVehicle *get_front_vehicle() const;
            RailVehicle *get_back_vehicle() const;
            Array get_rail_vehicle_modules() const;
            Ref<TrainSet> get_trainset() const;
            void set_modules(const Array &p_modules);
            Array get_modules() const;
            virtual Dictionary get_supported_commands();
            void enter_tree(Node *p_host);
            void ready();
            void exit_tree();
            void process(double delta);
            void physics_process(double delta);
            Node *get_runtime_host() const;

            RailVehicle *decouple(int relative_index);
            RailVehicle *uncouple_front();
            RailVehicle *uncouple_back();
    };
} // namespace godot

VARIANT_ENUM_CAST(RailVehicle::Side)

#endif // RAILVEHICLE_HPP
