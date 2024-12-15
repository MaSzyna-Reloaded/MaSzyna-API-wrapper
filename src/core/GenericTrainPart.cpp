#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "GenericTrainPart.hpp"

namespace godot {
    GenericTrainPart::GenericTrainPart() = default;

    void GenericTrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_train_controller_node"), &GenericTrainPart::get_train_controller_node);
        ClassDB::bind_method(D_METHOD("get_train_state"), &GenericTrainPart::get_train_state);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _process_train_part);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_train_part_state);
    }

    void GenericTrainPart::_do_update_internal_mover(TMoverParameters *mover) {};
    void GenericTrainPart::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {};
    void GenericTrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {};
    void GenericTrainPart::_do_process_mover(TMoverParameters *mover, double delta) {};
    void GenericTrainPart::_process_train_part(const double delta) {};
    Dictionary GenericTrainPart::_get_train_part_state() {
        return internal_state;
    };
    void GenericTrainPart::_process_mover(const double delta) {
        call("_process_train_part", delta);
        // FIXME: this should not be called each frame, but only when state changes
        internal_state = call("_get_train_part_state");
        train_controller_node->get_state().merge(internal_state, true);
    };

    TrainController *GenericTrainPart::get_train_controller_node() {
        return train_controller_node;
    }

    Dictionary GenericTrainPart::get_train_state() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_state();
        } else {
            UtilityFunctions::push_error("GenericTrainPart has no train controller node");
            return Dictionary();
        }
    }

} // namespace godot
