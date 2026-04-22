#ifndef RAILVEHICLE_HPP
#define RAILVEHICLE_HPP

#include "macros.hpp"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {
    class LegacyRailVehicleModule;
    class TrainSet;
    class RailVehicle : public Resource {
            GDCLASS(RailVehicle, Resource);

        protected:
            static void _bind_methods();
            Ref<TrainSet> trainset;
            Array modules;
            bool runtime_initialized = false;
            void _assign_modules_to_vehicle();
            void _clear_module_vehicle_references();
            RailVehicle *resolve_vehicle(ObjectID vehicle_id) const;

        public:
            String id;
            ObjectID front_id;
            ObjectID back_id;

            enum Side {
                FRONT = 0,
                BACK,
            };

        protected:
            virtual void _on_coupled(RailVehicle *other_vehicle, Side self_side, Side other_side);
            virtual void _on_uncoupled(RailVehicle *other_vehicle, Side self_side, Side other_side);
            virtual void _initialize();
            virtual void _initialize_after_modules();
            virtual void _finalize();
            virtual void _update(double delta);

        public:
            RailVehicle();
            ~RailVehicle() override;

            MAKE_MEMBER_GS(float, mass, 0.0);
            MAKE_MEMBER_GS(float, max_velocity, 0.0);
            MAKE_MEMBER_GS(float, length, 0.0);
            MAKE_MEMBER_GS(float, width, 0.0);
            MAKE_MEMBER_GS(float, height, 0.0);

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
            virtual void initialize();
            virtual void finalize();
            void update(double delta);

            RailVehicle *decouple(int relative_index);
            RailVehicle *uncouple_front();
            RailVehicle *uncouple_back();
    };
} // namespace godot

VARIANT_ENUM_CAST(RailVehicle::Side)

#endif // RAILVEHICLE_HPP
