#pragma once
#include "./GameLog.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/string.hpp>
#include <map>

namespace godot {

    class VehicleController;

    class TrainSystem : public Object {
            GDCLASS(TrainSystem, Object);

        private:
            std::map<String, VehicleController *> trains;
            Dictionary commands;

        public:
            static TrainSystem *get_instance() {
                return dynamic_cast<TrainSystem *>(godot::Engine::get_singleton()->get_singleton("TrainSystem"));
            }

            void register_train(const String &p_train_id, VehicleController *p_train);
            void unregister_train(const String &p_train_id);
            bool is_train_registered(const String &p_train_id) const;
            VehicleController *get_train(const String &p_train_id);
            Vector3 get_train_world_position(const String &p_train_id) const;
            int get_train_count() const;
            Array get_registered_trains();
            Array get_train_ids_in_rect(const Rect2 &p_rect);
            Dictionary get_train_state(const String &p_train_id);

            Dictionary get_all_config_properties(const String &p_train_id);
            Array get_supported_config_properties(const String &p_train_id);
            Variant get_config_property(const String &p_train_id, const String &p_property_name);

            void register_command(const String &p_train_id, const String &p_command, const Callable &p_callback);
            void unregister_command(const String &p_train_id, const String &p_command, const Callable &p_callback);
            Array get_supported_commands();
            Variant send_command(
                    const String &p_train_id, const String &p_command, const Variant &p_p1 = Variant(),
                    const Variant &p_p2 = Variant());
            void broadcast_command(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            bool is_command_supported(const String &p_command);

            void log(const String &p_train_id, GameLog::LogLevel p_level, const String &p_line);

            static const char *train_position_changed_signal;
            static const char *train_unregistered_signal;

        public:
            /// One physics sub-iteration for a whole set of controllers, looped in C++.
            ///
            /// Driven from RailVehiclePhysicsServer, which used to make four calls across the
            /// binding per controller per iteration - with hundreds of vehicles and several
            /// iterations per frame that is thousands of crossings, each marshalling Variants,
            /// for arithmetic the original does in a plain loop (vehicle_table::update(),
            /// DynObj.cpp:8195-8210). The track logic stays in GDScript: this returns how far
            /// each controller wants to move and the caller walks it along its track.
            ///
            /// [param full_movement] picks ComputeMovement() over FastComputeMovement(), which the
            /// original uses for every sub-iteration but the last (DynObj.cpp:4086).
            PackedFloat64Array
            step_vehicles(const TypedArray<VehicleController> &p_controllers, double p_step, bool p_full_movement);

        protected:
            static void _bind_methods();
            void _on_train_position_changed(const Vector3 &p_position, const String &p_train_id);
    };
} // namespace godot
