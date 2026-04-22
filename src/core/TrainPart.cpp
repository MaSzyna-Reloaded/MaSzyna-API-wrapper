#include "./TrainSystem.hpp"
#include "TrainController.hpp"
#include "TrainPart.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &TrainPart::emit_config_changed_signal);
        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &TrainPart::register_command);
        ClassDB::bind_method(D_METHOD("unregister_command", "command", "callable"), &TrainPart::unregister_command);
        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &TrainPart::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));
        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &TrainPart::broadcast_command, DEFVAL(Variant()),
                DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &TrainPart::log);
        ClassDB::bind_method(D_METHOD("log_debug", "line"), &TrainPart::log_debug);
        ClassDB::bind_method(D_METHOD("log_info", "line"), &TrainPart::log_info);
        ClassDB::bind_method(D_METHOD("log_warning", "line"), &TrainPart::log_warning);
        ClassDB::bind_method(D_METHOD("log_error", "line"), &TrainPart::log_error);

        ClassDB::bind_method(D_METHOD("set_enabled"), &TrainPart::set_enabled);
        ClassDB::bind_method(D_METHOD("get_enabled"), &TrainPart::get_enabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

        ADD_SIGNAL(MethodInfo("config_changed"));
        ADD_SIGNAL(MethodInfo("enable_changed", PropertyInfo(Variant::BOOL, "enabled")));
        ADD_SIGNAL(MethodInfo("train_part_enabled"));
        ADD_SIGNAL(MethodInfo("train_part_disabled"));
    }

    void TrainPart::_register_commands() {};
    void TrainPart::_unregister_commands() {};

    void TrainPart::log(const GameLog::LogLevel level, const String &line) {
        if (train_controller.is_valid()) {
            TrainSystem::get_instance()->log(train_controller->get_train_id(), level, line);
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

    void TrainPart::emit_config_changed_signal() {
        emit_signal("config_changed");
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
        if (_dirty) {
            update_mover();
            _dirty = false;
        }

        if (enabled) {
            _update(delta);
            _process_mover(delta);
        }

        if (enabled_changed) {
            enabled_changed = false;
            if (train_controller.is_valid()) {
                train_controller->refresh_command_registry();
            }
            emit_signal("enable_changed", enabled);
            emit_signal(enabled ? "train_part_enabled" : "train_part_disabled");
        }
    }

    void TrainPart::_do_process_mover(TMoverParameters *mover, double delta) {}
    void TrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}
    void TrainPart::_do_update_internal_mover(TMoverParameters *mover) {}

    void TrainPart::set_enabled(bool p_value) {
        enabled_changed = (enabled != p_value);
        enabled = p_value;
        _dirty = true;
    }

    bool TrainPart::get_enabled() {
        return enabled;
    }

    void TrainPart::send_command(const String &command, const Variant &p1, const Variant &p2) {
        if (train_controller.is_valid()) {
            TrainSystem::get_instance()->send_command(train_controller->get_train_id(), command, p1, p2);
        }
    }

    void TrainPart::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        TrainSystem::get_instance()->broadcast_command(command, p1, p2);
    }

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
