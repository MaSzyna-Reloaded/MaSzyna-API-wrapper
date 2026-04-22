#include "GenericTrainPart.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void GenericTrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_train_controller"), &GenericTrainPart::get_train_controller);
        ClassDB::bind_method(D_METHOD("get_train_state"), &GenericTrainPart::get_train_state);
        ClassDB::bind_method(D_METHOD("update_state"), &GenericTrainPart::update_state);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _process_train_part, 2);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_train_part_state, 1);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _initialize, 0);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_supported_commands, 1);
    }

    void GenericTrainPart::_do_update_internal_mover(TMoverParameters *mover) {};
    void GenericTrainPart::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {};
    void GenericTrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {};
    void GenericTrainPart::_do_process_mover(TMoverParameters *mover, double delta) {};
    void GenericTrainPart::_process_train_part(const double delta) {};
    Dictionary GenericTrainPart::_get_supported_commands() {
        Dictionary commands;
        return commands;
    };
    Dictionary GenericTrainPart::get_supported_commands() {
        Dictionary commands = TrainPart::get_supported_commands();
        commands.merge(call("_get_supported_commands"));
        return commands;
    }

    void GenericTrainPart::_initialize() {
        TrainPart::_initialize();
        if (has_method("_initialize")) {
            call("_initialize");
        }
    }

    Dictionary GenericTrainPart::_get_train_part_state() {
        return internal_state;
    };

    void GenericTrainPart::update_state(const Dictionary &state) {
        internal_state.merge(state, true);
        if (train_controller != nullptr) {
            train_controller->get_state().merge(internal_state, true);
        }
    }

    void GenericTrainPart::_process_mover(const double delta) {
        call("_process_train_part", delta);
        internal_state = call("_get_train_part_state");
        if (train_controller != nullptr) {
            train_controller->_get_state_internal().merge(internal_state, true);
        }
    }

    TrainController *GenericTrainPart::get_train_controller() {
        return train_controller;
    }

    Dictionary GenericTrainPart::get_train_state() {
        if (train_controller != nullptr) {
            return train_controller->get_state();
        }
        UtilityFunctions::push_error("GenericTrainPart has no train controller");
        return {};
    }

} // namespace godot
