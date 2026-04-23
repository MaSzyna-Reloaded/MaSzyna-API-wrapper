#include "./TrainSystem.hpp"
#include "TrainCommand.hpp"
#include "TrainController.hpp"
#include "TrainPart.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &TrainPart::emit_config_changed_signal);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainPart::get_supported_commands);
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

    TypedArray<TrainCommand> TrainPart::get_supported_commands() {
        TypedArray<TrainCommand> commands;
        return commands;
    }

    void TrainPart::log(const GameLog::LogLevel level, const String &line) {
        if (train_controller_node != nullptr) {
            TrainSystem::get_instance()->log(train_controller_node->get_train_id(), level, line);
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

    void TrainPart::emit_config_changed_signal() {
        emit_signal("config_changed");
    }

    void TrainPart::_notification(int what) {
        LegacyRailVehicleModule::_notification(what);

        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        switch (what) {
            case NOTIFICATION_ENTER_TREE: {
                Node *parent = get_parent();
                while (parent != nullptr) {
                    train_controller_node = Object::cast_to<TrainController>(parent);
                    if (train_controller_node != nullptr) {
                        break;
                    }
                    parent = parent->get_parent();
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                train_controller_node = nullptr;
            } break;
            default:
                break;
        }
    }

    void TrainPart::_process(const double delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        if (_dirty) {
            update_mover();
            _dirty = false;
        }

        if (enabled) {
            _process_mover(delta);
        }

        if (enabled_changed) {
            enabled_changed = false;
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
        if (train_controller_node != nullptr) {
            TrainSystem::get_instance()->send_command(train_controller_node->get_train_id(), command, p1, p2);
        }
    }

    void TrainPart::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        TrainSystem::get_instance()->broadcast_command(command, p1, p2);
    }

} // namespace godot
