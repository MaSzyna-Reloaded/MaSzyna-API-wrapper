#include "TrainCommand.hpp"
#include "TrainController.hpp"
#include "TrainPart.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainPart::get_supported_commands);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainPart::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &TrainPart::get_mover_state);
        ClassDB::bind_method(D_METHOD("get_train_controller_node"), &TrainPart::get_train_controller_node);
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &TrainPart::emit_config_changed_signal);

        ClassDB::bind_method(D_METHOD("set_enabled"), &TrainPart::set_enabled);
        ClassDB::bind_method(D_METHOD("get_enabled"), &TrainPart::get_enabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

        ADD_SIGNAL(MethodInfo("config_changed"));
        ADD_SIGNAL(MethodInfo("enable_changed", PropertyInfo(Variant::BOOL, "enabled")));
        ADD_SIGNAL(MethodInfo("train_part_enabled"));
        ADD_SIGNAL(MethodInfo("train_part_disabled"));
    }

    void TrainPart::_notification(int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                set_process(true);
                Node *parent = get_parent();
                while (parent != nullptr) {
                    train_controller_node = Object::cast_to<TrainController>(parent);
                    if (train_controller_node != nullptr) {
                        break;
                    }
                    parent = parent->get_parent();
                }

                if (train_controller_node != nullptr) {
                    const Error config_con = train_controller_node->connect(
                            TrainController::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
                    if (config_con != OK) {
                        UtilityFunctions::push_warning(
                                "TrainPart::notification(NOTIFICATION_ENTER_TREE) failed with error code " +
                                String::num(config_con));
                    }

                    const Error init_con = train_controller_node->connect(
                            TrainController::MOVER_INITIALIZED_SIGNAL, Callable(this, "update_mover"));
                    if (init_con != OK) {
                        UtilityFunctions::push_warning(
                                "TrainPart::notification(NOTIFICATION_ENTER_TREE) mover init connect failed with error code " +
                                String::num(init_con));
                    }
                }
            } break;
            case NOTIFICATION_EXIT_TREE: {
                set_process(false);
                if (train_controller_node != nullptr) {
                    train_controller_node->disconnect(
                            TrainController::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
                    train_controller_node->disconnect(
                            TrainController::MOVER_INITIALIZED_SIGNAL, Callable(this, "update_mover"));
                }
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

    void TrainPart::_do_update_internal_mover(TMoverParameters *mover) {}
    void TrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}
    void TrainPart::_do_process_mover(TMoverParameters *mover, double delta) {}

    TypedArray<TrainCommand> TrainPart::get_supported_commands() {
        TypedArray<TrainCommand> commands;
        return commands;
    }

    TMoverParameters *TrainPart::get_mover() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_mover();
        }

        return nullptr;
    }

    void TrainPart::_process_mover(const double delta) {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, delta);
                train_controller_node->_get_state_internal().merge(get_mover_state(), true);
            }
        }
    }

    void TrainPart::update_mover() {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_update_internal_mover(mover);
                Dictionary new_config;
                _do_fetch_config_from_mover(mover, new_config);
                train_controller_node->update_config(new_config);
            } else {
                UtilityFunctions::push_warning(
                        "TrainPart::update_mover() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning(
                    "TrainPart::update_mover() failed: missing train controller node");
        }
    }

    Dictionary TrainPart::get_mover_state() {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_fetch_state_from_mover(mover, state);
            } else {
                UtilityFunctions::push_warning(
                        "TrainPart::get_mover_state() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning(
                    "TrainPart::get_mover_state() failed: missing train controller node");
        }

        return state;
    }

    TrainController *TrainPart::get_train_controller_node() const {
        return train_controller_node;
    }

    void TrainPart::emit_config_changed_signal() {
        emit_signal("config_changed");
    }

    void TrainPart::set_enabled(bool p_value) {
        enabled_changed = (enabled != p_value);
        enabled = p_value;
        _dirty = true;
    }

    bool TrainPart::get_enabled() {
        return enabled;
    }
} // namespace godot
