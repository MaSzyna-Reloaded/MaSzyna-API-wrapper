#include "vehicles/base/VehicleComponent.hpp"
#include "vehicles/base/VehicleController.hpp"
#include "vehicles/rail/RailVehicleEngine.hpp"
#include "vehicles/rail/RailVehicleLighting.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_extension.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    const char *VehicleController::simulation_configured_signal = "simulation_configured";
    const char *VehicleController::simulation_initialized_signal = "simulation_initialized";
    const char *VehicleController::command_received = "command_received";
    const char *VehicleController::roof_light_changed = "roof_light_changed";
    const char *VehicleController::config_changed = "config_changed";
    const char *VehicleController::position_changed_signal = "position_changed";

    void VehicleController::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_state"), &VehicleController::get_state);
        ClassDB::bind_method(D_METHOD("get_config"), &VehicleController::get_config);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "state", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_state");
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::DICTIONARY, "config", PROPERTY_HINT_NONE, "",
                        PROPERTY_USAGE_READ_ONLY | PROPERTY_USAGE_DEFAULT),
                "", "get_config");

        ClassDB::bind_method(
                D_METHOD("send_command", "command", "p1", "p2"), &VehicleController::send_command, DEFVAL(Variant()),
                DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("get_commands"), &VehicleController::get_commands);

        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &VehicleController::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));


        ClassDB::bind_method(D_METHOD("register_command", "command", "callable"), &VehicleController::register_command);
        ClassDB::bind_method(D_METHOD("unregister_command", "command"), &VehicleController::unregister_command);
        ClassDB::bind_method(D_METHOD("apply_config"), &VehicleController::apply_config);
        ClassDB::bind_method(D_METHOD("initialize"), &VehicleController::initialize);
        ClassDB::bind_method(D_METHOD("process_components", "delta"), &VehicleController::process_components);
        ClassDB::bind_method(D_METHOD("update_state"), &VehicleController::update_state);
        ClassDB::bind_method(D_METHOD("get_velocity"), &VehicleController::get_velocity);
        ClassDB::bind_method(D_METHOD("get_speed"), &VehicleController::get_speed);
        ClassDB::bind_method(D_METHOD("get_acceleration"), &VehicleController::get_acceleration);
        ClassDB::bind_method(D_METHOD("get_mass_total"), &VehicleController::get_mass_total);
        ClassDB::bind_method(D_METHOD("get_total_distance"), &VehicleController::get_total_distance);
        ClassDB::bind_method(D_METHOD("get_direction"), &VehicleController::get_direction);
        ClassDB::bind_method(D_METHOD("emit_config_changed"), &VehicleController::emit_config_changed);
        ClassDB::bind_method(D_METHOD("apply_configuration"), &VehicleController::apply_configuration);
        ClassDB::bind_method(D_METHOD("is_simulation_ready"), &VehicleController::is_simulation_ready);
        ClassDB::bind_method(D_METHOD("add_component", "component"), &VehicleController::add_component);
        ClassDB::bind_method(D_METHOD("get_component", "type"), &VehicleController::get_component);
        /* Read by whoever caches this vehicle's dump: a command runs synchronously, in the middle
         * of a step, so the step alone does not say whether a dump is still current. */
        ClassDB::bind_method(D_METHOD("get_command_serial"), &VehicleController::get_command_serial);
        ClassDB::bind_method(D_METHOD("find_generic_components", "tag"), &VehicleController::find_generic_components);
        ClassDB::bind_method(D_METHOD("is_physics_active"), &VehicleController::is_physics_active);
        ClassDB::bind_method(D_METHOD("get_world_transform"), &VehicleController::get_world_transform);
        ClassDB::bind_method(D_METHOD("get_world_position"), &VehicleController::get_world_position);
        ClassDB::bind_method(D_METHOD("get_rid"), &VehicleController::get_rid);
        ClassDB::bind_method(
                D_METHOD("emit_position_changed_if_needed"), &VehicleController::emit_position_changed_if_needed);
        ClassDB::bind_method(D_METHOD("get_occupied_cab"), &VehicleController::get_occupied_cab);
        ClassDB::bind_method(D_METHOD("set_vehicle_rid", "vehicle"), &VehicleController::set_vehicle_rid);

        BIND_PROPERTY(VehicleController, Variant::STRING, train_id);
        BIND_PROPERTY(VehicleController, Variant::STRING, type_name);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, mass);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, power);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, max_velocity);
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, category, PROPERTY_HINT_ENUM,
                enum_hint(
                        {{"Train", CATEGORY_TRAIN},
                         {"Road", CATEGORY_ROAD},
                         {"Ship", CATEGORY_SHIP},
                         {"Airplane", CATEGORY_AIRPLANE}}));
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_length, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_height, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_width, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_drag_coefficient, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, dimensions_floor_height, "dimensions");
        BIND_PROPERTY(VehicleController, Variant::FLOAT, initial_velocity);
        BIND_PROPERTY_W_HINT(
                VehicleController, Variant::INT, driver_type, "", PROPERTY_HINT_ENUM, "Nobody,HeadDriver,RearDriver");
        BIND_ENUM_CONSTANT(DRIVER_NOBODY);
        BIND_ENUM_CONSTANT(DRIVER_HEAD);
        BIND_ENUM_CONSTANT(DRIVER_REAR);
        BIND_PROPERTY(VehicleController, Variant::STRING, load_name);
        BIND_PROPERTY(VehicleController, Variant::FLOAT, load_amount);

        ADD_SIGNAL(MethodInfo(simulation_configured_signal));
        ADD_SIGNAL(MethodInfo(simulation_initialized_signal));
        ADD_SIGNAL(MethodInfo(roof_light_changed, PropertyInfo(Variant::BOOL, "is_enabled")));
        ADD_SIGNAL(MethodInfo(config_changed));
        ADD_SIGNAL(MethodInfo(position_changed_signal, PropertyInfo(Variant::VECTOR3, "position")));
        ADD_SIGNAL(MethodInfo(
                command_received, PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::NIL, "p1"),
                PropertyInfo(Variant::NIL, "p2")));

        BIND_ENUM_CONSTANT(CATEGORY_TRAIN);
        BIND_ENUM_CONSTANT(CATEGORY_ROAD);
        BIND_ENUM_CONSTANT(CATEGORY_SHIP);
        BIND_ENUM_CONSTANT(CATEGORY_AIRPLANE);
    }

    void VehicleController::register_command(const StringName &p_command, const Callable &p_callable) {
        ERR_FAIL_COND_MSG(commands.has(p_command), vformat("Command is already registered: %s", p_command));
        commands[p_command] = p_callable;
    }

    void VehicleController::unregister_command(const StringName &p_command) {
        commands.erase(p_command);
    }

    PackedStringArray VehicleController::get_commands() const {
        PackedStringArray names;
        for (const KeyValue<StringName, Callable> &entry: commands) {
            names.push_back(entry.key);
        }
        return names;
    }

    void VehicleController::_notification(const int p_what) {
        if (Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        if (p_what == NOTIFICATION_PREDELETE) {
            release();
        }
    }

    /* Applying the vehicle's configuration to the backend: the vehicle's own, then every
     * component's, in registration order. The signal is emitted afterwards and means exactly
     * "the backend now carries this" - it is not how the components are reached, because a
     * component of this vehicle is applied by name here rather than by whoever happens to be
     * connected. */
    void VehicleController::apply_configuration() {
        apply_config();
        for (VehicleComponent *component: components) {
            if (component != nullptr) {
                component->apply_config();
            }
        }
        emit_signal(simulation_configured_signal);
    }

    /* Registering the vehicle's name and its own commands, where the vehicle comes into being -
     * it used to wait for NOTIFICATION_ENTER_TREE, which a vehicle outside a tree never gets. */
    /* A vehicle that is rebuilt keeps its identity - every reference taken to it stays valid -
     * so what it holds is handed back by name rather than by destroying the vehicle. */
    void VehicleController::release() {
        shutdown();
        free_components();
    }

    void VehicleController::attach_to_system() {
        /* The name the scenery gave this vehicle goes to the server that owns its handle, so that
         * whoever knows the vehicle only by name - an event, the console, a `.scn` command - can
         * find the handle. Everything that already holds the vehicle uses the handle. */
        if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
            server->vehicle_set_name(rid, train_id);
        }
        _register_commands();
    }

    /* The simulation, once every component is attached - _initialize_simulation() pushes the
     * configuration out to all of them (simulation_configured). */
    void VehicleController::initialize() {
        _initialize_simulation();
        update_state();
        emit_signal(roof_light_changed, prev_roof_light_enabled);
    }

    void VehicleController::process_components(const double p_delta) {
        for (VehicleComponent *component: components) {
            component->process(p_delta);
        }
    }

    /// Only marks the state for a rebuild - whoever reads it gets it fresh (see get_state()). The
    /// signals below have to be decided every step though, so they read the live getters directly
    /// rather than through a dictionary that may not be built at all.
    void VehicleController::update_state() {
        if (!is_simulation_ready()) {
            return;
        }
        if (const bool new_roof_light_enabled = lighting != nullptr && lighting->get_roof_light_enabled();
            prev_roof_light_enabled != new_roof_light_enabled) {
            prev_roof_light_enabled = new_roof_light_enabled; // FIXME: I don't like this
            emit_signal(roof_light_changed, new_roof_light_enabled);
        }
    }

    void VehicleController::emit_position_changed_if_needed() {
        const Vector3 position = get_world_position();
        if (position.distance_to(last_emitted_position) < 1.0) {
            return;
        }
        last_emitted_position = position;
        emit_signal(position_changed_signal, position);
    }


    void VehicleController::_fill_state_dictionary(Dictionary &p_state) const {
        if (!is_simulation_ready()) {
            return;
        }
        p_state["mass_total"] = get_mass_total();
        p_state["velocity"] = get_velocity();
        p_state["speed"] = get_speed();
        p_state["acceleration"] = get_acceleration();
        p_state["total_distance"] = get_total_distance();
        p_state["direction"] = get_direction();
    }

    /* The whole vehicle's configuration: its own plus every component's, composed when asked.
     * Unlike the state it is not a view on the backend - the wrapper's own properties and enums
     * are the authoring source of truth, and the backend is configured from them. */
    Dictionary VehicleController::get_config() const {
        Dictionary result;
        _fill_config_dictionary(result);
        for (const VehicleComponent *component: components) {
            component->_fill_config_dictionary(result);
        }
        return result;
    }

    void VehicleController::emit_config_changed() {
        emit_signal(config_changed);
    }

    /// A proxy, not a store: the vehicle keeps no state Dictionary of its own. Every value is
    /// answered by the component that owns it, and this walks them by name for the callers that
    /// still want one - a console, a test, a diagnostic dump. Nothing on the frame path builds it.
    /* The whole vehicle's dump: its own share plus every component's. Expensive on purpose -
     * a console, a test or a diagnostic asks for it, never a per-frame reader. */
    VehicleComponent *VehicleController::get_component(const VehicleComponentType::Type p_type) const {
        for (VehicleComponent *component: components) {
            if (component->get_component_type() == p_type) {
                return component;
            }
        }
        return nullptr;
    }

    TypedArray<VehicleComponent> VehicleController::find_generic_components(const StringName &p_tag) const {
        TypedArray<VehicleComponent> found;
        for (VehicleComponent *component: components) {
            if (component->get_component_type() == VehicleComponentType::COMPONENT_GENERIC &&
                component->get_component_tag() == p_tag) {
                found.push_back(component);
            }
        }
        return found;
    }

    /* Attaching a component is a complete operation: it joins the vehicle and, when the vehicle
     * is already running, its configuration is written to the backend and announced there and
     * then. A component added to a built vehicle - a modder's, or one a test adds - must not
     * leave the vehicle describing geometry it does not have. */
    void VehicleController::add_component(VehicleComponent *p_component) {
        ERR_FAIL_NULL(p_component);
        p_component->attach(this);
        if (is_simulation_ready()) {
            p_component->apply_config();
        }
    }

    /* Every component goes with the vehicle; nothing outside it holds one. */
    void VehicleController::shutdown() {
        _unregister_commands();
        // the handle belongs to RailVehicle3D, which frees it with itself
        rid = RID();
    }

    void VehicleController::free_components() {
        const Vector<VehicleComponent *> owned = components;
        components.clear();
        lighting = nullptr;
        for (VehicleComponent *component: owned) {
            component->detach();
            memdelete(component);
        }
    }

    void VehicleController::register_component(VehicleComponent *p_component) {
        components.push_back(p_component);
        _component_attached(p_component);
        if (RailVehicleLighting *component_lighting = Object::cast_to<RailVehicleLighting>(p_component);
            component_lighting != nullptr) {
            lighting = component_lighting;
        }
    }

    void VehicleController::unregister_component(VehicleComponent *p_component) {
        _component_detached(p_component);
        components.erase(p_component);
        if (static_cast<VehicleComponent *>(lighting) == p_component) {
            lighting = nullptr;
        }
    }

    Dictionary VehicleController::get_state() {
        Dictionary result;
        _fill_state_dictionary(result);
        for (VehicleComponent *component: components) {
            if (component->get_enabled()) {
                component->_fill_state_dictionary(result);
            }
        }
        return result;
    }

    Vector3 VehicleController::get_world_position() const {
        return get_world_transform().get_origin();
    }

    Transform3D VehicleController::get_world_transform() const {
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr || !rid.is_valid()) {
            return Transform3D();
        }
        return server->vehicle_get_transform(rid);
    }

    void VehicleController::set_vehicle_rid(const RID &p_vehicle_rid) {
        rid = p_vehicle_rid;
    }

    RID VehicleController::get_rid() const {
        return rid;
    }

    void VehicleController::command_executed(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        ++command_serial;
        update_state();
        emit_signal(command_received, p_command, p_p1, p_p2);
    }

    uint64_t VehicleController::get_command_serial() const {
        return command_serial;
    }

    void VehicleController::broadcast_command(const String &p_command, const Variant &p_p1, const Variant &p_p2) {
        if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
            server->broadcast_command(p_command, p_p1, p_p2);
        }
    }

    /* A handler takes as many of the two arguments as it declares; its return value says whether
     * the command was accepted (#43), and Variant() means nothing handled it. */
    Variant VehicleController::send_command(const StringName &p_command, const Variant &p_p1, const Variant &p_p2) {
        Variant result;
        if (const Callable *handler = commands.getptr(p_command); handler != nullptr) {
            Array args;
            const int argc = static_cast<int>(handler->get_argument_count());
            if (argc > 0) {
                args.append(p_p1);
            }
            if (argc > 1) {
                args.append(p_p2);
            }
            result = handler->callv(args);
        } else {
            UtilityFunctions::push_error(vformat("%s: Unknown command: %s", train_id, p_command));
        }
        command_executed(p_command, p_p1, p_p2);
        return result;
    }

    void VehicleController::set_driver_type(const DriverType p_value) {
        driver_type = p_value;
    }

    VehicleController::DriverType VehicleController::get_driver_type() const {
        return driver_type;
    }

    /* The backend counts the occupied cab as +1 for the front one and -1 for the rear
     * (DynObj.cpp:1812-1825); nobody aboard is 0, and that is what keeps an unmanned vehicle out
     * of the physics. */
    int VehicleController::get_occupied_cab() const {
        switch (driver_type) {
            case DRIVER_HEAD:
                return 1;
            case DRIVER_REAR:
                return -1;
            default:
                return 0;
        }
    }

} // namespace godot
