#include "GenericVehicleComponent.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void GenericVehicleComponent::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_train_controller_node"), &GenericVehicleComponent::get_train_controller_node);
        ClassDB::bind_method(D_METHOD("get_train_state"), &GenericVehicleComponent::get_train_state);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _process_train_part, 2);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _get_train_part_state, 1);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _get_train_part_config, 1);
    }

    void GenericVehicleComponent::_do_update_internal_mover(TMoverParameters *p_mover) {};
    void GenericVehicleComponent::_do_fetch_config_from_mover(TMoverParameters *p_mover, Dictionary &p_config) {
        p_config.merge(call("_get_train_part_config"), true);
    };
    void GenericVehicleComponent::_do_process_mover(TMoverParameters *p_mover, double p_delta) {};
    void GenericVehicleComponent::_process_train_part(const double p_delta) {};
    Dictionary GenericVehicleComponent::_get_train_part_state() {
        return internal_state;
    };
    Dictionary GenericVehicleComponent::_get_train_part_config() {
        return {};
    };
    void GenericVehicleComponent::_process_mover(const double p_delta) {
        call("_process_train_part", p_delta);
        internal_state = call("_get_train_part_state");
    };

    /* The script's own keys. They are still pulled per tick rather than being properties of the
     * component, which stage C replaces - see TODO.md. */
    void GenericVehicleComponent::_fill_state_dictionary(Dictionary &p_state) const {
        p_state.merge(internal_state, true);
    }

    VehicleController *GenericVehicleComponent::get_train_controller_node() {
        return train_controller_node;
    }

    Dictionary GenericVehicleComponent::get_train_state() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_state();
        }
        UtilityFunctions::push_error("GenericVehicleComponent has no train controller node");
        return {};
    }

} // namespace godot
