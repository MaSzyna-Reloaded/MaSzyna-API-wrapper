#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <map>
#include "./LogSystem.hpp"

namespace godot {

    class TrainController;

    class TrainSystem final : public RefCounted {
            //NOLINTNEXTLINE(modernize-use-auto)
            GDCLASS(TrainSystem, RefCounted);

        private:
            std::map<String, TrainController *> trains;
            Dictionary commands;

        public:
            static const char *TRAIN_LOG_UPDATED_SIGNAL;

            static TrainSystem *get_instance() {
                return dynamic_cast<TrainSystem *>(Engine::get_singleton()->get_singleton("TrainSystem"));
            }

            TrainSystem();
            ~TrainSystem() override = default;

            void register_train(const String &train_id, TrainController *train);
            void unregister_train(const String &train_id);
            [[nodiscard]] bool is_train_registered(const String &train_id) const;
            TrainController *get_train(const String &train_id);
            [[nodiscard]] int get_train_count() const;

            Variant get_config_property(const String &train_id, const String &property);
            Dictionary get_all_config_properties(const String &train_id);
            Array get_supported_config_properties(const String &train_id);

            void register_command(const String &train_id, const String &command, const Callable &callback);
            void unregister_command(const String &train_id, const String &command, const Callable &callback);
            Array get_supported_commands() const;
            Array get_registered_trains() const;
            void send_command(
                    const String &train_id, const String &command, const Variant &p1 = Variant(),
                    const Variant &p2 = Variant());
            void broadcast_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());
            bool is_command_supported(const String &command) const;

            void log(const String &train_id, LogSystem::LogLevel level, const String &line);

            Dictionary get_train_state(const String &train_id);

        protected:
            static void _bind_methods();
    };
} // namespace godot
