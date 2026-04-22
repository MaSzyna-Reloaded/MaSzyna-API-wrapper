#include "GenericTrainPart.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void GenericTrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_train_controller"), &GenericTrainPart::get_train_controller);
        ClassDB::bind_method(D_METHOD("get_train_state"), &GenericTrainPart::get_train_state);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_train_part_state, 1);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_supported_commands, 0);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _initialize, 0);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _finalize, 0);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _update, 1);
    }

    void GenericTrainPart::_do_update_internal_mover(TMoverParameters *mover) {};
    void GenericTrainPart::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {};
    void GenericTrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {};
    void GenericTrainPart::_do_process_mover(TMoverParameters *mover, double delta) {};

    void GenericTrainPart::_initialize() {
        TrainPart::_initialize();
        call("_initialize");
    }

    void GenericTrainPart::_finalize() {
        call("_finalize");
        TrainPart::_finalize();
    }

    void GenericTrainPart::_update(const double delta) {
        call("_update", delta);
    }

    Dictionary GenericTrainPart::get_mover_state() {
        TrainPart::get_mover_state();
        Dictionary custom_state = call("_get_train_part_state");
        state.merge(custom_state);
        return state;
    }

    Dictionary GenericTrainPart::get_supported_commands() {
        Dictionary commands;
        commands.merge(call("_get_supported_commands"));
        return commands;
    }

    Dictionary GenericTrainPart::_get_supported_commands() {
        return {};
    }


    Dictionary GenericTrainPart::_get_train_part_state() {
        return internal_state;
    };

    void GenericTrainPart::_process_mover(const double delta) {
        internal_state = call("_get_train_part_state");
        if (train_controller.is_valid()) {
            train_controller->_get_state_internal().merge(internal_state, true);
        }
    }

    Ref<TrainController> GenericTrainPart::get_train_controller() const {
        return train_controller;
    }

    Dictionary GenericTrainPart::get_train_state() {
        if (train_controller.is_valid()) {
            return train_controller->get_state();
        }
        UtilityFunctions::push_error("GenericTrainPart has no train controller");
        return {};
    }

} // namespace godot
