#pragma once

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <map>

#ifdef LIBMASZYNA_DEBUG
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
    class GameLogger;
    class GameLogHandler;

    /// The game's log, as Python's logging: named loggers (get_logger()) and the handlers
    /// registered for a logger's id (create_handler()) - a handler may wait for a logger that does
    /// not exist yet. Nothing is kept in memory: a line goes to the handlers and to message_logged
    /// at once.
    class GameLog : public Object {
            GDCLASS(GameLog, Object);

        public:
            static const char *logger_created_signal;
            static const char *message_logged_signal;
            /// The logger of the game's own messages, made with the log
            static const char *GAME_LOGGER;

            static GameLog *get_instance() {
                return dynamic_cast<GameLog *>(Engine::get_singleton()->get_singleton("GameLog"));
            }

            enum LogLevel {
                DEBUG = 0,
                INFO,
                WARNING,
                ERROR,
            };

            GameLog();

            Ref<GameLogger> create_logger(const String &p_logger_id);
            /// The one getter that makes what it returns (CODE_STYLE.md, "Logging"): a logger
            /// missing yet is created, as Python's logging.getLogger() does
            Ref<GameLogger> get_logger(const String &p_logger_id);
            PackedStringArray get_loggers() const;
            void create_handler(const String &p_logger_id, const Ref<GameLogHandler> &p_handler);
            void remove_handler(const String &p_logger_id, const Ref<GameLogHandler> &p_handler);
            void log(const String &p_logger_id, LogLevel p_level, const String &p_line);

            /// A logger's line, to the handlers of its id and to message_logged (GameLogger::log())
            void write(const String &p_logger_id, LogLevel p_level, const String &p_line);

        protected:
            static void _bind_methods();

        private:
            /// logger id -> GameLogger
            Dictionary loggers;
            /// logger id -> Array of GameLogHandler
            Dictionary handlers;
    };
} // namespace godot
VARIANT_ENUM_CAST(GameLog::LogLevel)
