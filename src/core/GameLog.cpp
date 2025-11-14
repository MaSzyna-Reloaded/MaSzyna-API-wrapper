#include "./GameLog.hpp"

namespace godot {
    const char *GameLog::LOG_UPDATED_SIGNAL = "log_updated";

    void GameLog::_bind_methods() {
        ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &GameLog::log);
        ClassDB::bind_method(D_METHOD("debug", "line"), &GameLog::debug);
        ClassDB::bind_method(D_METHOD("info", "line"), &GameLog::info);
        ClassDB::bind_method(D_METHOD("warning", "line"), &GameLog::warning);
        ClassDB::bind_method(D_METHOD("error", "line"), &GameLog::error);
        ADD_SIGNAL(MethodInfo(
                LOG_UPDATED_SIGNAL, PropertyInfo(Variant::INT, "loglevel"), PropertyInfo(Variant::STRING, "line")));
        BIND_ENUM_CONSTANT(DEBUG);
        BIND_ENUM_CONSTANT(INFO);
        BIND_ENUM_CONSTANT(WARNING);
        BIND_ENUM_CONSTANT(ERROR);
    }

    void GameLog::log(const LogLevel level, const String &line) {
        emit_signal(LOG_UPDATED_SIGNAL, level, line);
    }

    void GameLog::debug(const String &line) {
        log(LogLevel::DEBUG, line);
    }

    void GameLog::info(const String &line) {
        log(LogLevel::INFO, line);
    }

    void GameLog::warning(const String &line) {
        log(LogLevel::WARNING, line);
    }

    void GameLog::error(const String &line) {
        log(LogLevel::ERROR, line);
    }

} // namespace godot
