#include "GenericVehicleComponent.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    void GenericVehicleComponent::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_controller"), &GenericVehicleComponent::get_controller);
        ClassDB::bind_method(D_METHOD("get_vehicle_state"), &GenericVehicleComponent::get_vehicle_state);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _process_component, 2);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _get_component_state, 1);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _get_component_config, 1);
    }

    void GenericVehicleComponent::_do_update_internal_mover(TMoverParameters *p_mover) {};
    void GenericVehicleComponent::_fill_config_dictionary(Dictionary &p_config) const {
        TMoverParameters *mover = get_mover();
        if (mover == nullptr) {
            return;
        }
        // the script subclass cannot be known at build time, and Object::call() is non-const -
        // the call reads the script's own configuration and changes nothing here
        p_config.merge(const_cast<GenericVehicleComponent *>(this)->call("_get_component_config"), true);
    }
    void GenericVehicleComponent::_do_process_mover(TMoverParameters *mover, double p_delta) {};
    void GenericVehicleComponent::_process_component(const double p_delta) {};
    Dictionary GenericVehicleComponent::_get_component_state() {
        return internal_state;
    };
    Dictionary GenericVehicleComponent::_get_component_config() {
        return {};
    };
    void GenericVehicleComponent::_process_mover(const double p_delta) {
        call("_process_component", p_delta);
        internal_state = call("_get_component_state");
    };

    /* The script's own keys. They are still pulled per tick rather than being properties of the
     * component, which stage C replaces - see TODO.md. */
    void GenericVehicleComponent::_fill_state_dictionary(Dictionary &p_state) const {
        p_state.merge(internal_state, true);
    }

    VehicleController *GenericVehicleComponent::get_controller() {
        return train_controller_node;
    }

    Dictionary GenericVehicleComponent::get_vehicle_state() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_state();
        }
        UtilityFunctions::push_error("GenericVehicleComponent has no train controller node");
        return {};
    }

} // namespace godot
