#include "./GameLog.hpp"
#include "./GameLogHandler.hpp"
#include "./GameLogger.hpp"

namespace godot {
    const char *GameLog::logger_created_signal = "logger_created";
    const char *GameLog::message_logged_signal = "message_logged";
    const char *GameLog::GAME_LOGGER = "game";

    void GameLog::_bind_methods() {
        ClassDB::bind_method(D_METHOD("create_logger", "logger_id"), &GameLog::create_logger);
        ClassDB::bind_method(D_METHOD("get_logger", "logger_id"), &GameLog::get_logger);
        ClassDB::bind_method(D_METHOD("get_loggers"), &GameLog::get_loggers);
        ClassDB::bind_method(D_METHOD("create_handler", "logger_id", "handler"), &GameLog::create_handler);
        ClassDB::bind_method(D_METHOD("remove_handler", "logger_id", "handler"), &GameLog::remove_handler);
        ClassDB::bind_method(D_METHOD("log", "logger_id", "loglevel", "line"), &GameLog::log);
        ADD_SIGNAL(MethodInfo(logger_created_signal, PropertyInfo(Variant::STRING, "logger_id")));
        ADD_SIGNAL(MethodInfo(
                message_logged_signal, PropertyInfo(Variant::STRING, "logger_id"),
                PropertyInfo(Variant::INT, "loglevel"), PropertyInfo(Variant::STRING, "line")));
        BIND_ENUM_CONSTANT(DEBUG);
        BIND_ENUM_CONSTANT(INFO);
        BIND_ENUM_CONSTANT(WARNING);
        BIND_ENUM_CONSTANT(ERROR);
    }

    GameLog::GameLog() {
        create_logger(GAME_LOGGER);
    }

    Ref<GameLogger> GameLog::create_logger(const String &p_logger_id) {
        ERR_FAIL_COND_V_MSG(loggers.has(p_logger_id), loggers[p_logger_id], "Logger already exists: " + p_logger_id);
        Ref<GameLogger> logger;
        logger.instantiate();
        logger->setup(this, p_logger_id);
        loggers[p_logger_id] = logger;
        emit_signal(logger_created_signal, p_logger_id);
        return logger;
    }

    Ref<GameLogger> GameLog::get_logger(const String &p_logger_id) {
        if (loggers.has(p_logger_id)) {
            return loggers[p_logger_id];
        }
        return create_logger(p_logger_id);
    }

    PackedStringArray GameLog::get_loggers() const {
        return PackedStringArray(loggers.keys());
    }

    void GameLog::create_handler(const String &p_logger_id, const Ref<GameLogHandler> &p_handler) {
        ERR_FAIL_COND(p_handler.is_null());
        if (!handlers.has(p_logger_id)) {
            handlers[p_logger_id] = Array();
        }
        Array logger_handlers = handlers[p_logger_id];
        logger_handlers.append(p_handler);
    }

    void GameLog::remove_handler(const String &p_logger_id, const Ref<GameLogHandler> &p_handler) {
        ERR_FAIL_COND_MSG(!handlers.has(p_logger_id), "No handlers for logger: " + p_logger_id);
        Array logger_handlers = handlers[p_logger_id];
        logger_handlers.erase(p_handler);
    }

    void GameLog::log(const String &p_logger_id, const LogLevel p_level, const String &p_line) {
        get_logger(p_logger_id)->log(p_level, p_line);
    }

    void GameLog::write(const String &p_logger_id, const LogLevel p_level, const String &p_line) {
        if (handlers.has(p_logger_id)) {
            const Array logger_handlers = handlers[p_logger_id];
            for (int i = 0; i < logger_handlers.size(); i++) {
                const Ref<GameLogHandler> handler = logger_handlers[i];
                handler->handle(p_logger_id, p_level, p_line);
            }
        }
        emit_signal(message_logged_signal, p_logger_id, p_level, p_line);
    }

} // namespace godot
