#include "../physics/RailVehicleServer.hpp"
#include "VehicleComponent.hpp"
#include "VehiclePhysicsNode.hpp"
#include <godot_cpp/classes/class_db_singleton.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot {
    const char *VehiclePhysicsNode::vehicle_changed_signal = "vehicle_changed";
    StringName &VehiclePhysicsNode::controller_implementation() {
        static StringName implementation;
        return implementation;
    }

    void VehiclePhysicsNode::set_controller_implementation(const StringName &p_class) {
        controller_implementation() = p_class;
    }

    void VehiclePhysicsNode::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_model", "model"), &VehiclePhysicsNode::set_model);
        ClassDB::bind_method(D_METHOD("get_model"), &VehiclePhysicsNode::get_model);
        /* Shown, never stored: a model built from a source file (a .fiz) would otherwise be
         * embedded in whatever scene holds this node and drift from the file it came from. A
         * vehicle authored as a .tres is referenced by the subclass that loads it. */
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "model", PROPERTY_HINT_RESOURCE_TYPE, "VehicleModel", PROPERTY_USAGE_EDITOR),
                "set_model", "get_model");

        ClassDB::bind_method(D_METHOD("set_train_id", "train_id"), &VehiclePhysicsNode::set_train_id);
        ClassDB::bind_method(D_METHOD("get_train_id"), &VehiclePhysicsNode::get_train_id);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "train_id"), "set_train_id", "get_train_id");
        ClassDB::bind_method(D_METHOD("set_initial_velocity", "velocity"), &VehiclePhysicsNode::set_initial_velocity);
        ClassDB::bind_method(D_METHOD("get_initial_velocity"), &VehiclePhysicsNode::get_initial_velocity);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "initial_velocity"), "set_initial_velocity", "get_initial_velocity");
        ClassDB::bind_method(D_METHOD("set_driver_type", "driver_type"), &VehiclePhysicsNode::set_driver_type);
        ClassDB::bind_method(D_METHOD("get_driver_type"), &VehiclePhysicsNode::get_driver_type);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "driver_type", PROPERTY_HINT_ENUM, "Nobody,HeadDriver,RearDriver"),
                "set_driver_type", "get_driver_type");
        ClassDB::bind_method(D_METHOD("set_load_name", "load_name"), &VehiclePhysicsNode::set_load_name);
        ClassDB::bind_method(D_METHOD("get_load_name"), &VehiclePhysicsNode::get_load_name);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_name"), "set_load_name", "get_load_name");
        ClassDB::bind_method(D_METHOD("set_load_amount", "load_amount"), &VehiclePhysicsNode::set_load_amount);
        ClassDB::bind_method(D_METHOD("get_load_amount"), &VehiclePhysicsNode::get_load_amount);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "load_amount"), "set_load_amount", "get_load_amount");

        ClassDB::bind_method(D_METHOD("get_vehicle_rid"), &VehiclePhysicsNode::get_vehicle_rid);
        ClassDB::bind_method(D_METHOD("get_controller"), &VehiclePhysicsNode::get_controller);
        ClassDB::bind_method(D_METHOD("add_component", "component"), &VehiclePhysicsNode::add_component);

        ADD_SIGNAL(MethodInfo(vehicle_changed_signal));
    }

    void VehiclePhysicsNode::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        // on entering, not on ready: Godot readies children before their parent, and a
        // component proxy below this node has to find a vehicle already standing
        if (p_what == NOTIFICATION_ENTER_TREE && controller == nullptr) {
            // with whatever model the node was given before it entered; without one the vehicle
            // still comes up, empty - components can be added to it, or a model applied later
            _build(model);
        }
        if (p_what == NOTIFICATION_PREDELETE) {
            if (RailVehicleServer *server = RailVehicleServer::get_instance();
                server != nullptr && vehicle_rid.is_valid()) {
                server->vehicle_free(vehicle_rid);
            }
            vehicle_rid = RID();
            if (controller != nullptr) {
                controller->shutdown();
                memdelete(controller);
                controller = nullptr;
            }
        }
    }

    /* The vehicle is built here and nowhere else: one owner of the handle, one owner of the
     * controller, both freed with this node. */
    void VehiclePhysicsNode::set_model(const Ref<VehicleModel> &p_model) {
        model = p_model;
        if (is_inside_tree()) {
            _build(model);
        }
    }

    void VehiclePhysicsNode::_build(const Ref<VehicleModel> &p_model) {
        if (controller == nullptr) {
            controller = Object::cast_to<VehicleController>(
                    ClassDBSingleton::get_singleton()->instantiate(controller_implementation()));
            ERR_FAIL_NULL_MSG(
                    controller, vformat("Unknown vehicle controller implementation: %s", controller_implementation()));
        } else {
            /* Rebuilding replaces what the vehicle is made of, not the vehicle. Destroying the
             * controller here left every reference taken to it dangling - a sound bank registered
             * against the vehicle before its model arrived held a freed object. */
            controller->release();
        }
        if (p_model.is_valid()) {
            VehicleModel::apply(controller, p_model->get_properties());
        }
        controller->set_train_id(train_id);
        controller->set_initial_velocity(initial_velocity);
        controller->set_driver_type(driver_type);
        controller->set_load_name(load_name);
        controller->set_load_amount(load_amount);
        if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
            if (!vehicle_rid.is_valid()) {
                vehicle_rid = server->vehicle_create();
            }
            server->vehicle_attach_controller(vehicle_rid, controller->get_instance_id());
            controller->set_vehicle_rid(vehicle_rid);
        }
        controller->attach_to_system();

        const TypedArray<VehicleComponentModel> components =
                p_model.is_valid() ? p_model->get_components() : TypedArray<VehicleComponentModel>();
        for (int i = 0; i < components.size(); i++) {
            const Ref<VehicleComponentModel> entry = components[i];
            if (entry.is_null()) {
                continue;
            }
            Object *created = ClassDBSingleton::get_singleton()->instantiate(entry->get_implementation());
            VehicleComponent *component = Object::cast_to<VehicleComponent>(created);
            if (component == nullptr) {
                ERR_PRINT(vformat("Unknown vehicle component implementation: %s", entry->get_implementation()));
                continue;
            }
            VehicleModel::apply(component, entry->get_properties());
            controller->add_component(component);
        }

        controller->initialize();
        emit_signal(vehicle_changed_signal);
    }

    Ref<VehicleModel> VehiclePhysicsNode::get_model() const {
        return model;
    }

    RID VehiclePhysicsNode::get_vehicle_rid() const {
        return vehicle_rid;
    }

    VehicleController *VehiclePhysicsNode::get_controller() const {
        return controller;
    }

    void VehiclePhysicsNode::add_component(VehicleComponent *p_component) {
        ERR_FAIL_NULL(p_component);
        ERR_FAIL_NULL_MSG(controller, "VehiclePhysicsNode has no vehicle to add a component to yet.");
        controller->add_component(p_component);
    }

    void VehiclePhysicsNode::set_train_id(const String &p_train_id) {
        train_id = p_train_id;
        if (controller != nullptr) {
            controller->set_train_id(train_id);
        }
    }

    String VehiclePhysicsNode::get_train_id() const {
        return train_id;
    }

    void VehiclePhysicsNode::set_initial_velocity(const double p_velocity) {
        initial_velocity = p_velocity;
        if (controller != nullptr) {
            controller->set_initial_velocity(initial_velocity);
        }
    }

    double VehiclePhysicsNode::get_initial_velocity() const {
        return initial_velocity;
    }

    void VehiclePhysicsNode::set_driver_type(const VehicleController::DriverType p_driver_type) {
        driver_type = p_driver_type;
        if (controller != nullptr) {
            controller->set_driver_type(driver_type);
        }
    }

    VehicleController::DriverType VehiclePhysicsNode::get_driver_type() const {
        return driver_type;
    }

    void VehiclePhysicsNode::set_load_name(const String &p_load_name) {
        load_name = p_load_name;
        if (controller != nullptr) {
            controller->set_load_name(load_name);
        }
    }

    String VehiclePhysicsNode::get_load_name() const {
        return load_name;
    }

    void VehiclePhysicsNode::set_load_amount(const double p_load_amount) {
        load_amount = p_load_amount;
        if (controller != nullptr) {
            controller->set_load_amount(load_amount);
        }
    }

    double VehiclePhysicsNode::get_load_amount() const {
        return load_amount;
    }
} // namespace godot
