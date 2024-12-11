#include "GenericTrainPart.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void GenericTrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_train_controller_node"), &GenericTrainPart::get_train_controller_node);
        ClassDB::bind_method(D_METHOD("get_train_state"), &GenericTrainPart::get_train_state);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _process_train_part, 2);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_train_part_state, 1);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_supported_commands, 1);
    }

    void GenericTrainPart::_do_update_internal_mover(TMoverParameters *mover) {}
    void GenericTrainPart::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {}
    void GenericTrainPart::_do_fetch_config_from_mover(TMoverParameters *mover, Dictionary &config) {}
    void GenericTrainPart::_do_process_mover(TMoverParameters *mover, double delta) {}
    void GenericTrainPart::_process_train_part(const double delta) {}

    Dictionary GenericTrainPart::_get_train_part_state() {
        return internal_state;
    }

    Array GenericTrainPart::_get_supported_commands() {
        return {};
    }

    void GenericTrainPart::_process_mover(const double delta) {
        call("_process_train_part", delta);
        internal_state = call("_get_train_part_state");
        if (train_controller_node != nullptr) {
            train_controller_node->_get_state_internal().merge(internal_state, true);
        }
    }

    TrainController *GenericTrainPart::get_train_controller_node() {
        return train_controller_node;
    }

    Dictionary GenericTrainPart::get_train_state() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_state();
        }
        UtilityFunctions::push_error("GenericTrainPart has no train controller node");
        return {};
    }

    TypedArray<TrainCommand> GenericTrainPart::get_supported_commands() {
        TypedArray<TrainCommand> commands;
        const Variant result = call("_get_supported_commands");
        if (result.get_type() != Variant::ARRAY) {
            return commands;
        }

        const Array script_commands = result;
        for (const Variant &command_variant : script_commands) {
            if (command_variant.get_type() == Variant::OBJECT) {
                Ref<TrainCommand> command = command_variant;
                if (command.is_valid()) {
                    commands.append(command);
                }
            }
        }
        return commands;
    }
} // namespace godot
