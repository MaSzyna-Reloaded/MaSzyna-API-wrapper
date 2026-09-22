#include "./TrainSystem.hpp"
#include "VehicleController.hpp"
#include "VehicleComponent.hpp"
#include "../physics/VehiclePropertyRegistry.hpp"
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
        ClassDB::bind_method(D_METHOD("get_state"), &VehicleComponent::get_state);
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
        ADD_SIGNAL(MethodInfo("train_part_enabled"));
        ADD_SIGNAL(MethodInfo("train_part_disabled"));
    }

    void VehicleComponent::_declare_state_properties() {}

    void VehicleComponent::_do_fetch_state_from_mover(TMoverParameters *, Dictionary &) {}

    Variant VehicleComponent::_get_state_property(int) const {
        return Variant();
    }

    int VehicleComponent::get_state_property_count() const {
        return static_cast<int>(state_property_ids.size());
    }

    int VehicleComponent::declare_state_property(
            const StringName &p_name, const Variant::Type p_type, const StringName &p_group) {
        const int id = VehiclePropertyRegistry::declare(p_name, p_type, get_class(), p_group);
        ERR_FAIL_COND_V(id < 0, -1);
        const int local_index = state_property_ids.size();
        state_property_ids.push_back(id);
        if (train_controller_node != nullptr) {
            train_controller_node->register_state_property(id, this, local_index);
        }
        return local_index;
    }

    void VehicleComponent::_register_commands() {};
    void VehicleComponent::_unregister_commands() {};

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
                    const Error con = train_controller_node->connect(
                            VehicleController::mover_config_changed_signal, Callable(this, "apply_config"));
                    if (con != OK) {
                        log_warning(
                                "VehicleComponent::notification(NOTIFICATION_ENTER_TREE) failed with error code " +
                                String::num(con));
                    }
                }
                if (train_controller_node != nullptr) {
                    state_property_ids.clear();
                    _declare_state_properties();
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
                    train_controller_node->unregister_state_properties(this);
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
            TrainSystem::get_instance()->log(train_controller_node->get_train_id(), p_level, p_line);
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
        TrainSystem::get_instance()->register_command(train_controller_node->get_train_id(), p_command, p_callback);
    }

    void VehicleComponent::unregister_command(const String &p_command, const Callable &p_callback) {
        TrainSystem::get_instance()->unregister_command(train_controller_node->get_train_id(), p_command, p_callback);
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

    void VehicleComponent::_process_mover(const double p_delta) {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, p_delta);
                train_controller_node->get_state().merge(get_state(), true);
            }
        }
    }

    void VehicleComponent::_do_process_mover(TMoverParameters *p_mover, double p_delta) {}
    void VehicleComponent::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {};
    void VehicleComponent::_do_update_internal_mover(TMoverParameters *p_mover) {};

    void VehicleComponent::apply_config() {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_update_internal_mover(mover);
                Dictionary new_config;
                _do_fetch_config_from_mover(mover, new_config);
                train_controller_node->update_config(new_config);
            } else {
                UtilityFunctions::push_warning("VehicleComponent::apply_config() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning("VehicleComponent::apply_config() failed: missing train controller node");
        }
    }

    Dictionary VehicleComponent::get_state() {
        if (!get_enabled()) {
            return state;
        }
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_fetch_state_from_mover(mover, state);
            } else {
                UtilityFunctions::push_warning("VehicleComponent::get_state() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning("VehicleComponent::get_state() failed: missing train controller node");
        }
        return state;
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
            TrainSystem::get_instance()->send_command(train_controller_node->get_train_id(), p_command, p_p1, p_p2);
        }
    }

    void VehicleComponent::broadcast_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        TrainSystem::get_instance()->broadcast_command(p_command, p_p1, p_p2);
    }

} // namespace godot
