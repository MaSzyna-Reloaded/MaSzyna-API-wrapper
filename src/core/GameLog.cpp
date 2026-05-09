#include "./GameLog.hpp"

namespace godot {
    const char *GameLog::log_updated_signal = "log_updated";

    void GameLog::_bind_methods() {
        ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &GameLog::log);
        ClassDB::bind_method(D_METHOD("debug", "line"), &GameLog::debug);
        ClassDB::bind_method(D_METHOD("info", "line"), &GameLog::info);
        ClassDB::bind_method(D_METHOD("warning", "line"), &GameLog::warning);
        ClassDB::bind_method(D_METHOD("error", "line"), &GameLog::error);
        ADD_SIGNAL(MethodInfo(
                log_updated_signal, PropertyInfo(Variant::INT, "loglevel"), PropertyInfo(Variant::STRING, "line")));
        BIND_ENUM_CONSTANT(DEBUG);
        BIND_ENUM_CONSTANT(INFO);
        BIND_ENUM_CONSTANT(WARNING);
        BIND_ENUM_CONSTANT(ERROR);
    }

    void GameLog::log(const LogLevel p_level, const String &p_line) {
        emit_signal(log_updated_signal, p_level, p_line);
    }

    void GameLog::debug(const String &p_line) {
        log(LogLevel::DEBUG, p_line);
    }

    void GameLog::info(const String &p_line) {
        log(LogLevel::INFO, p_line);
    }

    void GameLog::warning(const String &p_line) {
        log(LogLevel::WARNING, p_line);
    }

    void GameLog::error(const String &p_line) {
        log(LogLevel::ERROR, p_line);
    }

} // namespace godot
