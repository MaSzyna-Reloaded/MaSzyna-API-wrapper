#include "./TrainSystem.hpp"
#include "TrainController.hpp"
#include "TrainPart.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &TrainPart::emit_config_changed_signal);
        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &TrainPart::register_command);
        ClassDB::bind_method(D_METHOD("unregister_command", "command", "callable"), &TrainPart::unregister_command);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainPart::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &TrainPart::get_mover_state);
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
    void TrainPart::_do_initialize_train_controller(TrainController *train_controller) {};

    TMoverParameters *TrainPart::get_mover() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_mover();
        }

        return nullptr;
    }

    void TrainPart::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                Node *p = get_parent();
                while (p != nullptr) {
                    train_controller_node = Object::cast_to<TrainController>(p);
                    if (train_controller_node != nullptr) {
                        break;
                    }
                    p = p->get_parent();
                }
                if (train_controller_node != nullptr) {
                    train_controller_node->connect(
                            TrainController::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
                    _do_initialize_train_controller(train_controller_node);
                }
                if (enabled) {
                    _register_commands();
                    _commands_registered = true;
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                if (get_enabled()) {
                    _unregister_commands();
                    _commands_registered = false;
                }
                if (train_controller_node != nullptr) {
                    train_controller_node->disconnect(
                            TrainController::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
                }
                train_controller_node = nullptr;
            } break;
            default:;
        }
    }

    void TrainPart::log(const LogSystem::LogLevel level, const String &line) {
        if (train_controller_node != nullptr) {
            TrainSystem::get_instance()->log(train_controller_node->get_train_id(), level, line);
        }
    }
    void TrainPart::log_debug(const String &line) {
        log(LogSystem::LogLevel::DEBUG, line);
    }

    void TrainPart::log_info(const String &line) {
        log(LogSystem::LogLevel::INFO, line);
    }

    void TrainPart::log_warning(const String &line) {
        log(LogSystem::LogLevel::WARNING, line);
    }

    void TrainPart::log_error(const String &line) {
        log(LogSystem::LogLevel::ERROR, line);
    }

    void TrainPart::register_command(const String &command, const Callable &callback) {
        TrainSystem::get_instance()->register_command(train_controller_node->get_train_id(), command, callback);
    }

    void TrainPart::unregister_command(const String &command, const Callable &callback) {
        TrainSystem::get_instance()->unregister_command(train_controller_node->get_train_id(), command, callback);
    }

    void TrainPart::emit_config_changed_signal() {
        emit_signal("config_changed");
    }

    void TrainPart::_process(const double delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        if (_dirty) {
            // emit_config_changed_signal();
            update_mover();
            _dirty = false;
        }

        if (enabled && train_controller_node->get_occupied()) {
            _process_mover(delta);
        }

        if (enabled_changed) {
            enabled_changed = false;
            if (enabled && !_commands_registered) {
                log_debug("Registering commands for train part " + get_name());
                _register_commands();
                _commands_registered = true;
            } else if (!enabled && _commands_registered) {
                log_debug("Unregistering commands for train part " + get_name());
                _unregister_commands();
                _commands_registered = false;
            }
            emit_signal("enable_changed", enabled);
            emit_signal(enabled ? "train_part_enabled" : "train_part_disabled");
        }
    }

    void TrainPart::_process_mover(const double delta) {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, delta);
                train_controller_node->get_state().merge(get_mover_state(), true);
                train_controller_node->mark_state_changed();
            }
        }
    }

    void TrainPart::_do_process_mover(TMoverParameters *mover, double delta) {}
    void TrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {};
    void TrainPart::_do_update_internal_mover(TMoverParameters *mover) {};

    void TrainPart::update_mover() {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_update_internal_mover(mover);
                Dictionary new_config;
                _do_fetch_config_from_mover(mover, new_config);
                train_controller_node->update_config(new_config);
            } else {
                UtilityFunctions::push_warning("TrainPart::update_mover() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning("TrainPart::update_mover() failed: missing train controller node");
        }
    }

    Dictionary TrainPart::get_mover_state() {
        if (!get_enabled()) {
            return state;
        }
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_fetch_state_from_mover(mover, state);
            } else {
                UtilityFunctions::push_warning("TrainPart::get_mover_state() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning("TrainPart::get_mover_state() failed: missing train controller node");
        }
        return state;
    }

    void TrainPart::set_enabled(const bool p_value) {
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
