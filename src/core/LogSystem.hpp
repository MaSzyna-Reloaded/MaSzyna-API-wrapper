#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

#ifdef DEBUG_ENABLED
// #define DEBUG(msg, ...) LogSystem::get_instance()->debug("[debug] " + vformat(String(msg), ##__VA_ARGS__))
#define DEBUG(msg, ...)                                                                                                \
    UtilityFunctions::print_rich("[color=orange](debug) " + vformat(String(msg), ##__VA_ARGS__) + "[/color]")
#else
#define DEBUG(msg, ...)
#endif

namespace godot {

    class LogSystem : public RefCounted {
            GDCLASS(LogSystem, RefCounted);

        private:
        public:
            static const char *LOG_UPDATED_SIGNAL;

            inline static LogSystem *get_instance() {
                return dynamic_cast<LogSystem *>(godot::Engine::get_singleton()->get_singleton("LogSystem"));
            }

            enum LogLevel {
                LOGLEVEL_DEBUG = 0,
                LOGLEVEL_INFO,
                LOGLEVEL_WARNING,
                LOGLEVEL_ERROR,
            };

            LogSystem();
            ~LogSystem() override = default;

            void log(const LogLevel level, const String &line);
            void debug(const String &line);
            void info(const String &line);
            void warning(const String &line);
            void error(const String &line);

        protected:
            static void _bind_methods();
    };
} // namespace godot
VARIANT_ENUM_CAST(LogSystem::LogLevel)
