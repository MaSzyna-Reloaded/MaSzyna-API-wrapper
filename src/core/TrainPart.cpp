#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "TrainController.hpp"
#include "TrainPart.hpp"

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("_on_command_received"), &TrainPart::_on_command_received);
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &TrainPart::emit_config_changed_signal);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainPart::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &TrainPart::get_mover_state);

        ClassDB::bind_method(D_METHOD("set_enabled"), &TrainPart::set_enabled);
        ClassDB::bind_method(D_METHOD("get_enabled"), &TrainPart::get_enabled);
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

        ADD_SIGNAL(MethodInfo("config_changed"));
        ADD_SIGNAL(MethodInfo("enable_changed", PropertyInfo(Variant::BOOL, "enabled")));
        ADD_SIGNAL(MethodInfo("train_part_enabled"));
        ADD_SIGNAL(MethodInfo("train_part_disabled"));
    }

    TrainPart::TrainPart() = default;

    void TrainPart::_ready() {
        /* Dear Lord, prevent it from running in the editor. Thanks~ UwU */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        if (train_controller_node != nullptr) {
            train_controller_node->connect(TrainController::COMMAND_RECEIVED, Callable(this, "_on_command_received"));
        }
        _dirty = true;
    }

    void TrainPart::emit_config_changed_signal() {
        emit_signal("config_changed");
    }

    void TrainPart::_enter_tree() {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
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
        }
    }

    void TrainPart::_exit_tree() {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        if (train_controller_node != nullptr) {
            train_controller_node->disconnect(
                    TrainController::MOVER_CONFIG_CHANGED_SIGNAL, Callable(this, "update_mover"));
        }
        train_controller_node = nullptr;
    }

    void TrainPart::_process(double delta) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }

        if (!get_enabled()) {
            if(enabled_changed) {
                enabled_changed = false;
                emit_signal("enable_changed", false);
                emit_signal("train_part_disabled");
            }

            return;
        }

        if (_dirty) {
            // emit_config_changed_signal();
            update_mover();
            _dirty = false;
        }

        _process_mover(delta);

        if(enabled_changed) {
            enabled_changed = false;
            emit_signal("enable_changed", enabled);
            emit_signal(enabled ? "train_part_enabled" : "train_part_disabled");
        }
    }

    void TrainPart::_process_mover(double delta) {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_process_mover(mover, delta);
                train_controller_node->get_state().merge(get_mover_state(), true);
            }
        }
    }

    void TrainPart::update_mover() {
        if (train_controller_node != nullptr) {
            TMoverParameters *mover = train_controller_node->get_mover();
            if (mover != nullptr) {
                _do_update_internal_mover(mover);
            } else {
                UtilityFunctions::push_warning("TrainPart::update_mover() failed: internal mover not initialized");
            }
        } else {
            UtilityFunctions::push_warning("TrainPart::update_mover() failed: missing train controller node");
        }
    }

    Dictionary TrainPart::get_mover_state() {
        if(!get_enabled()) {
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

    void TrainPart::set_enabled(bool p_value) {
        enabled_changed = (enabled != p_value);
        enabled = p_value;
        _dirty = true;
    }

    bool TrainPart::get_enabled() {
        return enabled;
    }

    void TrainPart::_on_command_received(const String &command, const Variant &p1, const Variant &p2) {
    }
} // namespace godot
