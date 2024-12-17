#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#ifdef DEBUG_ENABLED
#if defined(_MSC_VER)
// MSVC doesn't require special handling.
#define DEBUG(msg, ...)                                                                                                \
    UtilityFunctions::print_rich("[color=orange](debug) " + vformat(String(msg), Array::make(__VA_ARGS__)) + "[/color]")
#elif defined(__cplusplus) && __cplusplus >= 202002L
// C++20 with __VA_OPT__
#define DEBUG(msg, ...)                                                                                                \
    UtilityFunctions::print_rich(                                                                                      \
            "[color=orange](debug) " + vformat(String(msg) __VA_OPT__(, ) Array::make(__VA_ARGS__)) + "[/color]")
#else
// Use a workaround for C++11 to C++17
#define DEBUG(msg, ...)                                                                                                \
    UtilityFunctions::print_rich("[color=orange](debug) " + vformat(String(msg), ##__VA_ARGS__) + "[/color]")
#endif
#else
#define DEBUG(msg, ...)
#endif

namespace godot {

    class LogSystem final : public RefCounted {
            GDCLASS(LogSystem, RefCounted);

        private:
        public:
            static const char *LOG_UPDATED_SIGNAL;

            static LogSystem *get_instance() {
                return dynamic_cast<LogSystem *>(Engine::get_singleton()->get_singleton("LogSystem"));
            }

            enum LogLevel {
                DEBUG = 0,
                INFO,
                WARNING,
                ERROR,
            };

            LogSystem();
            ~LogSystem() override = default;

            void log(LogLevel level, const String &line);
            void debug(const String &line);
            void info(const String &line);
            void warning(const String &line);
            void error(const String &line);

        protected:
            static void _bind_methods();
    };
} // namespace godot
VARIANT_ENUM_CAST(LogSystem::LogLevel)
