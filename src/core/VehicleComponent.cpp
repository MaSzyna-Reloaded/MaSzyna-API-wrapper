#include "./TrainSystem.hpp"
#include "VehicleController.hpp"
#include "VehicleComponent.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void VehicleComponent::_bind_methods() {
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &VehicleComponent::emit_config_changed_signal);
        ClassDB::bind_method(D_METHOD("mark_dirty"), &VehicleComponent::mark_dirty);
        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &VehicleComponent::register_command);
        ClassDB::bind_method(D_METHOD("unregister_command", "command", "callable"), &VehicleComponent::unregister_command);
        ClassDB::bind_method(D_METHOD("apply_config"), &VehicleComponent::apply_config);
        ClassDB::bind_method(D_METHOD("get_controller"), &VehicleComponent::get_controller);
        ClassDB::bind_method(D_METHOD("get_state"), &VehicleComponent::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &VehicleComponent::get_config);
        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &VehicleComponent::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));
        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &VehicleComponent::broadcast_command, DEFVAL(Variant()),
                DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("log", "loglevel", "line"), &VehicleComponent::log);
        ClassDB::bind_method(D_METHOD("log_debug", "line"), &VehicleComponent::log_debug);
        ClassDB::bind_method(D_METHOD("log_info", "line"), &VehicleComponent::log_info);
        ClassDB::bind_method(D_METHOD("log_warning", "line"), &VehicleComponent::log_warning);
        ClassDB::bind_method(D_METHOD("log_error", "line"), &VehicleComponent::log_error);

        ClassDB::bind_method(D_METHOD("set_enabled"), &VehicleComponent::set_enabled);
        ClassDB::bind_method(D_METHOD("get_enabled"), &VehicleComponent::get_enabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

        ADD_SIGNAL(MethodInfo("config_changed"));
        ADD_SIGNAL(MethodInfo("enable_changed", PropertyInfo(Variant::BOOL, "enabled")));
        ADD_SIGNAL(MethodInfo("component_enabled"));
        ADD_SIGNAL(MethodInfo("component_disabled"));
    }

    void VehicleComponent::_fill_state_dictionary(Dictionary &p_state) const {}

    void VehicleComponent::_register_commands() {};
    void VehicleComponent::_unregister_commands() {};

    VehicleController *VehicleComponent::get_controller() const {
        return train_controller_node;
    }

    TMoverParameters *VehicleComponent::get_mover() const {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_mover();
        }

        return nullptr;
    }

    void VehicleComponent::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                Node *p = get_parent();
                while (p != nullptr) {
                    train_controller_node = Object::cast_to<VehicleController>(p);
                    if (train_controller_node != nullptr) {
                        break;
                    }
                    p = p->get_parent();
                }
                if (train_controller_node != nullptr) {
                    train_controller_node->register_component(this);
                    const Error con = train_controller_node->connect(
                            VehicleController::mover_config_changed_signal, Callable(this, "apply_config"));
                    if (con != OK) {
                        log_warning(
                                "VehicleComponent::notification(NOTIFICATION_ENTER_TREE) failed with error code " +
                                String::num(con));
                    }
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
                    train_controller_node->unregister_component(this);
                    train_controller_node->disconnect(
                            VehicleController::mover_config_changed_signal, Callable(this, "apply_config"));
                }
                train_controller_node = nullptr;
            } break;
            default:;
        }
    }

    void VehicleComponent::log(const GameLog::LogLevel p_level, const String &p_line) {
        if (train_controller_node != nullptr) {
            if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
                system->log(train_controller_node->get_train_id(), p_level, p_line);
            }
        }
    }
    void VehicleComponent::log_debug(const String &p_line) {
        log(GameLog::LogLevel::DEBUG, p_line);
    }

    void VehicleComponent::log_info(const String &p_line) {
        log(GameLog::LogLevel::INFO, p_line);
    }

    void VehicleComponent::log_warning(const String &p_line) {
        log(GameLog::LogLevel::WARNING, p_line);
    }

    void VehicleComponent::log_error(const String &p_line) {
        log(GameLog::LogLevel::ERROR, p_line);
    }

    void VehicleComponent::register_command(const String &p_command, const Callable &p_callback) {
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->register_command(train_controller_node->get_train_id(), p_command, p_callback);
        }
    }

    void VehicleComponent::unregister_command(const String &p_command, const Callable &p_callback) {
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->unregister_command(train_controller_node->get_train_id(), p_command, p_callback);
        }
    }

    void VehicleComponent::emit_config_changed_signal() {
        emit_signal("config_changed");
    }

    void VehicleComponent::mark_dirty() {
        dirty = true;
    }

    void VehicleComponent::_process(const double p_delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        if (dirty) {
            // emit_config_changed_signal();
            apply_config();
            dirty = false;
        }

        if (enabled) {
            _process_mover(p_delta);
        }

        if (enabled_changed) {
            enabled_changed = false;
            if (enabled && !_commands_registered) {
                log_debug("Registering commands for component " + get_name());
                _register_commands();
                _commands_registered = true;
            } else if (!enabled && _commands_registered) {
                log_debug("Unregistering commands for component " + get_name());
                _unregister_commands();
                _commands_registered = false;
            }
            emit_signal("enable_changed", enabled);
            emit_signal(enabled ? "component_enabled" : "component_disabled");
        }
    }

    void VehicleComponent::_process_mover(const double p_delta) {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, p_delta);
            }
        }
    }

    void VehicleComponent::_do_process_mover(TMoverParameters *p_mover, double p_delta) {}
    void VehicleComponent::_fill_config_dictionary(Dictionary &p_config) const {}

    Dictionary VehicleComponent::get_config() {
        Dictionary result;
        _fill_config_dictionary(result);
        return result;
    }
    void VehicleComponent::_do_update_internal_mover(TMoverParameters *p_mover) {};

    void VehicleComponent::apply_config() {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_update_internal_mover(mover);
                train_controller_node->emit_config_changed();
            } else {
                UtilityFunctions::push_warning("VehicleComponent::apply_config() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning("VehicleComponent::apply_config() failed: missing train controller node");
        }
    }

    /// The dump of this component alone. Nothing is stored and nothing is computed until asked:
    /// the live values are this component's own typed properties, read straight from the backend.
    Dictionary VehicleComponent::get_state() {
        Dictionary result;
        _fill_state_dictionary(result);
        return result;
    }

    void VehicleComponent::set_enabled(const bool p_value) {
        enabled_changed = (enabled != p_value);
        enabled = p_value;
        dirty = true;
    }

    bool VehicleComponent::get_enabled() {
        return enabled;
    }

    void VehicleComponent::send_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        if (train_controller_node != nullptr) {
            if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
                system->send_command(train_controller_node->get_train_id(), p_command, p_p1, p_p2);
            }
        }
    }

    void VehicleComponent::broadcast_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        if (TrainSystem *system = TrainSystem::get_instance(); system != nullptr) {
            system->broadcast_command(p_command, p_p1, p_p2);
        }
    }

} // namespace godot
