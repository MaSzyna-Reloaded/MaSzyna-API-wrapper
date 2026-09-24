#include "GenericVehicleComponentNode.hpp"
#include "VehiclePhysicsNode.hpp"
#include <godot_cpp/classes/engine.hpp>

namespace godot {
    void GenericVehicleComponentNode::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_component"), &GenericVehicleComponentNode::get_component);
        ClassDB::bind_method(
                D_METHOD("register_command", "command", "callable"), &GenericVehicleComponentNode::register_command);
        ClassDB::bind_method(
                D_METHOD("unregister_command", "command", "callable"),
                &GenericVehicleComponentNode::unregister_command);
        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &GenericVehicleComponentNode::send_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("get_vehicle_state"), &GenericVehicleComponentNode::get_vehicle_state);
        ClassDB::bind_method(D_METHOD("get_controller"), &GenericVehicleComponentNode::get_controller);
        ClassDB::bind_method(D_METHOD("log_debug", "line"), &GenericVehicleComponentNode::log_debug);
        ClassDB::bind_method(D_METHOD("log_info", "line"), &GenericVehicleComponentNode::log_info);
        ClassDB::bind_method(D_METHOD("log_warning", "line"), &GenericVehicleComponentNode::log_warning);
        ClassDB::bind_method(D_METHOD("log_error", "line"), &GenericVehicleComponentNode::log_error);
    }

    void GenericVehicleComponentNode::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        switch (p_what) {
            case NOTIFICATION_ENTER_TREE: {
                // the vehicle is whatever this node sits under - that is the whole point of it
                Node *parent = get_parent();
                VehiclePhysicsNode *vehicle = nullptr;
                while (parent != nullptr && vehicle == nullptr) {
                    vehicle = Object::cast_to<VehiclePhysicsNode>(parent);
                    parent = parent->get_parent();
                }
                if (vehicle == nullptr) {
                    ERR_PRINT("GenericVehicleComponentNode has no VehiclePhysicsNode above it.");
                    return;
                }
                component = memnew(GenericVehicleComponent);
                component->set_script_owner(this);
                vehicle->add_component(component);
            } break;
            case NOTIFICATION_EXIT_TREE:
            case NOTIFICATION_PREDELETE: {
                if (component != nullptr) {
                    component->detach();
                    memdelete(component);
                    component = nullptr;
                }
            } break;
            default:;
        }
    }

    GenericVehicleComponent *GenericVehicleComponentNode::get_component() const {
        return component;
    }

    void GenericVehicleComponentNode::register_command(const String &p_command, const Callable &p_callback) {
        ERR_FAIL_NULL(component);
        component->register_command(p_command, p_callback);
    }

    void GenericVehicleComponentNode::unregister_command(const String &p_command, const Callable &p_callback) {
        ERR_FAIL_NULL(component);
        component->unregister_command(p_command, p_callback);
    }

    Variant GenericVehicleComponentNode::send_command(
            const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        ERR_FAIL_NULL_V(component, Variant());
        component->send_command(p_command, p_p1, p_p2);
        return Variant();
    }

    Dictionary GenericVehicleComponentNode::get_vehicle_state() {
        ERR_FAIL_NULL_V(component, Dictionary());
        return component->get_vehicle_state();
    }

    VehicleController *GenericVehicleComponentNode::get_controller() const {
        GenericVehicleComponent *component = get_component();
        return component != nullptr ? component->get_controller() : nullptr;
    }

    void GenericVehicleComponentNode::log_debug(const String &p_line) {
        ERR_FAIL_NULL(component);
        component->log_debug(p_line);
    }

    void GenericVehicleComponentNode::log_info(const String &p_line) {
        ERR_FAIL_NULL(component);
        component->log_info(p_line);
    }

    void GenericVehicleComponentNode::log_warning(const String &p_line) {
        ERR_FAIL_NULL(component);
        component->log_warning(p_line);
    }

    void GenericVehicleComponentNode::log_error(const String &p_line) {
        ERR_FAIL_NULL(component);
        component->log_error(p_line);
    }
} // namespace godot
