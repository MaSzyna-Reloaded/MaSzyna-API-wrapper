#include "./LogSystem.hpp"

namespace godot {
    const char *LogSystem::LOG_UPDATED_SIGNAL = "log_updated";

    void LogSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &LogSystem::log);
        ClassDB::bind_method(D_METHOD("debug", "line"), &LogSystem::debug);
        ClassDB::bind_method(D_METHOD("info", "line"), &LogSystem::info);
        ClassDB::bind_method(D_METHOD("warning", "line"), &LogSystem::warning);
        ClassDB::bind_method(D_METHOD("error", "line"), &LogSystem::error);
        ADD_SIGNAL(MethodInfo(
                LOG_UPDATED_SIGNAL, PropertyInfo(Variant::INT, "loglevel"), PropertyInfo(Variant::STRING, "line")));
        BIND_ENUM_CONSTANT(DEBUG);
        BIND_ENUM_CONSTANT(INFO);
        BIND_ENUM_CONSTANT(WARNING);
        BIND_ENUM_CONSTANT(ERROR);
    }

    LogSystem::LogSystem() = default;

    void LogSystem::log(const LogLevel level, const String &line) {
        emit_signal(LOG_UPDATED_SIGNAL, level, line);
    }

    void LogSystem::debug(const String &line) {
        log(LogLevel::DEBUG, line);
    }

    void LogSystem::info(const String &line) {
        log(LogLevel::INFO, line);
    }

    void LogSystem::warning(const String &line) {
        log(LogLevel::WARNING, line);
    }

    void LogSystem::error(const String &line) {
        log(LogLevel::ERROR, line);
    }

} // namespace godot
