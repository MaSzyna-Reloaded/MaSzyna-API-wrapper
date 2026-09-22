#include "GenericVehicleComponent.hpp"
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
    /* Where the modder's script actually lives: on the proxy node when one authored this
     * component, on the component itself when a script extends it directly. */
    Object *GenericVehicleComponent::script_target() {
        return script_owner != nullptr ? static_cast<Object *>(script_owner) : static_cast<Object *>(this);
    }

    void GenericVehicleComponent::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_vehicle_state"), &GenericVehicleComponent::get_vehicle_state);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _process_component, 2);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _get_component_state, 1);
        BIND_VIRTUAL_METHOD(GenericVehicleComponent, _get_component_config, 1);
    }

    void GenericVehicleComponent::set_script_owner(Node *p_owner) {
        script_owner = p_owner;
    }

    void GenericVehicleComponent::_do_update_internal_mover(TMoverParameters *p_mover) {};
    /* A script's own configuration is its own - it does not come from the backend, so there is
     * nothing to guard against here. The script cannot be known at build time (that is what this
     * class is for) and Object::call() is non-const, though the call only reads. */
    void GenericVehicleComponent::_fill_config_dictionary(Dictionary &p_config) const {
        Object *target = const_cast<GenericVehicleComponent *>(this)->script_target();
        p_config.merge(target->call("_get_component_config"), true);
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
        script_target()->call("_process_component", p_delta);
        internal_state = script_target()->call("_get_component_state");
    };

    /* The script's own keys. They are still pulled per tick rather than being properties of the
     * component, which stage C replaces - see TODO.md. */
    void GenericVehicleComponent::_fill_state_dictionary(Dictionary &p_state) const {
        p_state.merge(internal_state, true);
    }

    Dictionary GenericVehicleComponent::get_vehicle_state() {
        if (train_controller_node != nullptr) {
            return train_controller_node->get_state();
        }
        UtilityFunctions::push_error("GenericVehicleComponent has no train controller node");
        return {};
    }

} // namespace godot
