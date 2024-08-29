#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "TrainController.hpp"
#include "TrainPart.hpp"

namespace godot {
    void TrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("emit_config_changed_signal"), &TrainPart::emit_config_changed_signal);
        ClassDB::bind_method(D_METHOD("update_mover"), &TrainPart::update_mover);
        ClassDB::bind_method(D_METHOD("get_mover_state"), &TrainPart::get_mover_state);
        ADD_SIGNAL(MethodInfo("config_changed"));
    }

    TrainPart::TrainPart() = default;

    void TrainPart::_ready() {
        /* Dear Lord, prevent it from running in the editor. Thanks~ UwU */
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        _dirty = true;
    }

    void TrainPart::emit_config_changed_signal() {
        emit_signal("config_changed");
    }

    void TrainPart::_process(double delta) {
        if (_dirty) {
            emit_config_changed_signal();
            _dirty = false;
        }
    }

    void TrainPart::update_mover(const TrainController *train_controller_node) {
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

    Dictionary TrainPart::get_mover_state(const TrainController *train_controller_node) {
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

} // namespace godot
