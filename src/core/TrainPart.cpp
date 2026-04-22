#include "TrainController.hpp"
#include "TrainPart.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &TrainPart::log);
        ClassDB::bind_method(D_METHOD("log_debug", "line"), &TrainPart::log_debug);
        ClassDB::bind_method(D_METHOD("log_info", "line"), &TrainPart::log_info);
        ClassDB::bind_method(D_METHOD("log_warning", "line"), &TrainPart::log_warning);
        ClassDB::bind_method(D_METHOD("log_error", "line"), &TrainPart::log_error);
    }

    void TrainPart::_register_commands() {};
    void TrainPart::_unregister_commands() {};

    void TrainPart::log(const GameLog::LogLevel level, const String &line) {
        if (train_controller.is_valid()) {
            GameLog::get_instance()->log(level, line);
        }
    }

    void TrainPart::log_debug(const String &line) {
        log(GameLog::LogLevel::DEBUG, line);
    }

    void TrainPart::log_info(const String &line) {
        log(GameLog::LogLevel::INFO, line);
    }

    void TrainPart::log_warning(const String &line) {
        log(GameLog::LogLevel::WARNING, line);
    }

    void TrainPart::log_error(const String &line) {
        log(GameLog::LogLevel::ERROR, line);
    }

    void TrainPart::register_command(const String &command, const Callable &callback) {
        if (collecting_commands) {
            command_registry[command] = callback;
            return;
        }

        log_debug("TrainPart::register_command() outside of command collection: " + command);
    }

    void TrainPart::unregister_command(const String &command, const Callable &callback) {
        if (collecting_commands && command_registry.has(command)) {
            command_registry.erase(command);
            return;
        }

        log_debug("TrainPart::unregister_command() outside of command collection: " + command);
    }

    void TrainPart::_initialize() {
        LegacyRailVehicleModule::_initialize();
        train_controller = Ref<TrainController>(Object::cast_to<TrainController>(get_rail_vehicle().ptr()));
    }

    void TrainPart::_finalize() {
        train_controller.unref();
        LegacyRailVehicleModule::_finalize();
    }

    void TrainPart::update(const double delta) {
        LegacyRailVehicleModule::update(delta);
    }

    void TrainPart::_do_process_mover(TMoverParameters *mover, double delta) {}
    void TrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}
    void TrainPart::_do_update_internal_mover(TMoverParameters *mover) {}

    Dictionary TrainPart::get_supported_commands() {
        command_registry.clear();
        collecting_commands = true;
        if (enabled) {
            _register_commands();
        }
        collecting_commands = false;
        return command_registry;
    }

} // namespace godot
