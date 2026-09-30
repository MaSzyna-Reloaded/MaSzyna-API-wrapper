#include "VehicleComponent.hpp"
#include "VehiclePhysicsNode.hpp"
#include "vehicles/base/VehicleServer.hpp"
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
        ClassDB::bind_method(D_METHOD("set_description", "description"), &VehiclePhysicsNode::set_description);
        ClassDB::bind_method(D_METHOD("get_description"), &VehiclePhysicsNode::get_description);
        /* Shown, never stored: a description built from a source file (a .fiz) would otherwise be
         * embedded in whatever scene holds this node and drift from the file it came from. A
         * vehicle authored as a .tres is referenced by the subclass that loads it. */
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::OBJECT, "description", PROPERTY_HINT_RESOURCE_TYPE, "VehicleController",
                        PROPERTY_USAGE_EDITOR),
                "set_description", "get_description");

        ClassDB::bind_method(D_METHOD("set_train_id", "train_id"), &VehiclePhysicsNode::set_train_id);
        ClassDB::bind_method(D_METHOD("get_train_id"), &VehiclePhysicsNode::get_train_id);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "train_id"), "set_train_id", "get_train_id");
        ClassDB::bind_method(D_METHOD("set_type_name", "type_name"), &VehiclePhysicsNode::set_type_name);
        ClassDB::bind_method(D_METHOD("get_type_name"), &VehiclePhysicsNode::get_type_name);
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "type_name"), "set_type_name", "get_type_name");
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
        if (p_what == NOTIFICATION_ENTER_TREE && controller.is_null()) {
            // with whatever description the node was given before it entered; without one the
            // vehicle still comes up, empty - components can be added to it, or a description
            // given later
            _build();
        }
        if (p_what == NOTIFICATION_PREDELETE) {
            if (VehicleServer *server = VehicleServer::get_instance(); server != nullptr && vehicle_rid.is_valid()) {
                server->vehicle_free(vehicle_rid);
            }
            vehicle_rid = RID();
            // the vehicle lets go of its components and commands; whoever still holds it keeps
            // an empty vehicle, never a freed one
            if (controller.is_valid()) {
                controller->release();
                controller.unref();
            }
        }
    }

    /* The vehicle is built here and nowhere else: one owner of the handle, one owner of the
     * controller, both freed with this node. */
    void VehiclePhysicsNode::set_description(const Ref<VehicleController> &p_description) {
        description = p_description;
        if (is_inside_tree()) {
            _build();
        }
    }

    void VehiclePhysicsNode::_build() {
        /* Rebuilding replaces what the vehicle is made of, not the vehicle: its handle stays, and
         * everything outside the vehicle layer holds that. The controller built before lets go of
         * its components, commands and simulation. */
        if (controller.is_valid()) {
            controller->release();
        }
        // a copy: the description is shared by every vehicle built from it (a cached FIZ); held
        // as it is created - a reference counted object left in the Variant alone is freed with it
        controller = description.is_valid()
                             ? Ref<VehicleController>(description->duplicate_deep(Resource::DEEP_DUPLICATE_INTERNAL))
                             : Ref<VehicleController>(
                                       ClassDBSingleton::get_singleton()->instantiate(controller_implementation()));
        ERR_FAIL_COND_MSG(controller.is_null(), "The vehicle's controller could not be made");
        controller->set_train_id(train_id);
        controller->set_type_name(type_name);
        controller->set_initial_velocity(initial_velocity);
        controller->set_driver_type(driver_type);
        controller->set_load_name(load_name);
        controller->set_load_amount(load_amount);
        if (VehicleServer *server = VehicleServer::get_instance(); server != nullptr) {
            if (!vehicle_rid.is_valid()) {
                vehicle_rid = server->vehicle_create();
            }
            server->vehicle_attach_controller(vehicle_rid, controller->get_instance_id());
        }
        controller->attach_to_system();
        controller->initialize();
        emit_signal(vehicle_changed_signal);
    }

    Ref<VehicleController> VehiclePhysicsNode::get_description() const {
        return description;
    }

    RID VehiclePhysicsNode::get_vehicle_rid() const {
        return vehicle_rid;
    }

    Ref<VehicleController> VehiclePhysicsNode::get_controller() const {
        return controller;
    }

    void VehiclePhysicsNode::add_component(const Ref<VehicleComponent> &p_component) {
        ERR_FAIL_COND(p_component.is_null());
        ERR_FAIL_COND_MSG(controller.is_null(), "VehiclePhysicsNode has no vehicle to add a component to yet.");
        controller->add_component(p_component);
    }

    void VehiclePhysicsNode::set_train_id(const String &p_train_id) {
        train_id = p_train_id;
        if (controller.is_valid()) {
            controller->set_train_id(train_id);
        }
    }

    String VehiclePhysicsNode::get_train_id() const {
        return train_id;
    }

    void VehiclePhysicsNode::set_type_name(const String &p_type_name) {
        type_name = p_type_name;
        if (controller.is_valid()) {
            controller->set_type_name(type_name);
        }
    }

    String VehiclePhysicsNode::get_type_name() const {
        return type_name;
    }

    void VehiclePhysicsNode::set_initial_velocity(const double p_velocity) {
        initial_velocity = p_velocity;
        if (controller.is_valid()) {
            controller->set_initial_velocity(initial_velocity);
        }
    }

    double VehiclePhysicsNode::get_initial_velocity() const {
        return initial_velocity;
    }

    void VehiclePhysicsNode::set_driver_type(const VehicleController::DriverType p_driver_type) {
        driver_type = p_driver_type;
        if (controller.is_valid()) {
            controller->set_driver_type(driver_type);
        }
    }

    VehicleController::DriverType VehiclePhysicsNode::get_driver_type() const {
        return driver_type;
    }

    void VehiclePhysicsNode::set_load_name(const String &p_load_name) {
        load_name = p_load_name;
        if (controller.is_valid()) {
            controller->set_load_name(load_name);
        }
    }

    String VehiclePhysicsNode::get_load_name() const {
        return load_name;
    }

    void VehiclePhysicsNode::set_load_amount(const double p_load_amount) {
        load_amount = p_load_amount;
        if (controller.is_valid()) {
            controller->set_load_amount(load_amount);
        }
    }

    double VehiclePhysicsNode::get_load_amount() const {
        return load_amount;
    }
} // namespace godot
