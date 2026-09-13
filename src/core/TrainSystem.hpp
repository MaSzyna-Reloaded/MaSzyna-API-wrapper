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

    class TrainController;

    class TrainSystem : public Object {
            GDCLASS(TrainSystem, Object);

        private:
            std::map<String, TrainController *> trains;
            Dictionary commands;

        public:
            static TrainSystem *get_instance() {
                return dynamic_cast<TrainSystem *>(godot::Engine::get_singleton()->get_singleton("TrainSystem"));
            }

            void register_train(const String &p_train_id, TrainController *p_train);
            void unregister_train(const String &p_train_id);
            bool is_train_registered(const String &p_train_id) const;
            TrainController *get_train(const String &p_train_id);
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
            void send_command(
                    const String &p_train_id, const String &p_command, const Variant &p_p1 = Variant(),
                    const Variant &p_p2 = Variant());
            void broadcast_command(
                    const String &p_command, const Variant &p_p1 = Variant(), const Variant &p_p2 = Variant());
            bool is_command_supported(const String &p_command);

            void log(const String &p_train_id, GameLog::LogLevel p_level, const String &p_line);

            static const char *train_position_changed_signal;

        protected:
            static void _bind_methods();
            void _on_train_position_changed(const Vector3 &p_position, const String &p_train_id);
    };
} // namespace godot
