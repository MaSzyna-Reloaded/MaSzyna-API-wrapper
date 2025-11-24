#include "GenericTrainPart.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void GenericTrainPart::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_train_state"), &GenericTrainPart::get_train_state);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _process_train_part, 2);
        BIND_VIRTUAL_METHOD(GenericTrainPart, _get_train_part_state, 1);
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
        internal_state = call("_get_train_part_state");
        TrainNode *train_node = Object::cast_to<TrainNode>(get_entity_node());
        if (train_node != nullptr) {
            train_node->update_state();
        }
    };

    Dictionary GenericTrainPart::get_train_state() {
        TrainNode *train_node = Object::cast_to<TrainNode>(get_entity_node());
        if (train_node != nullptr) {
            return train_node->get_state();
        }
        UtilityFunctions::push_error("GenericTrainPart has no train controller node");
        return {};
    }

} // namespace godot
