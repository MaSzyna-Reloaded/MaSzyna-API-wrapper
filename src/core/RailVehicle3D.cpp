#include "../scenery/SceneryStreamingServer.hpp"
#include "../load/VehicleLoad.hpp"
#include "RailVehicle3D.hpp"
#include "../traction/TractionPowerServer.hpp"
#include "../cabin/Cabin3D.hpp"
#include "../buffers/VehicleBuffCoupl.hpp"
#include "../wheels/VehicleWheels.hpp"
#include "VehiclePhysicsNode.hpp"

#include "../engines/VehicleElectricEngine.hpp"
#include "../physics/RailVehicleServer.hpp"
#include "../tracks/TrackManager.hpp"
#include "GameLog.hpp"

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/visible_on_screen_notifier3d.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <array>

namespace godot {
    const char *RailVehicle3D::controller_changed_signal = "controller_changed";

    namespace {
        struct LightStateBinding {
                const char *light_name;
                const char *state_name;
        };

        constexpr std::array<LightStateBinding, 10> LIGHT_STATE_BINDINGS = {{
                {"headlamp11", "lights/front_headlight_upper_enabled"},
                {"headlamp12", "lights/front_headlight_right_enabled"},
                {"headlamp13", "lights/front_headlight_left_enabled"},
                {"headlamp21", "lights/rear_headlight_upper_enabled"},
                {"headlamp22", "lights/rear_headlight_right_enabled"},
                {"headlamp23", "lights/rear_headlight_left_enabled"},
                {"endsignal12", "lights/front_redmarker_right_enabled"},
                {"endsignal13", "lights/front_redmarker_left_enabled"},
                {"endsignal22", "lights/rear_redmarker_right_enabled"},
                {"endsignal23", "lights/rear_redmarker_left_enabled"},
        }};
    } // namespace

    template<typename T>
    static T *node_at(Node *p_owner, const NodePath &p_path) {
        return Object::cast_to<T>(p_owner->get_node_or_null(p_path));
    }

    RailVehicle3D::RailVehicle3D() {
        set_process(true);
        pantograph_wire_cache.resize(4);
        for (int index = 0; index < pantograph_wire_cache.size(); ++index) {
            pantograph_wire_cache[index] = Dictionary();
        }
    }

    void RailVehicle3D::_bind_methods() {
#define BIND_RAIL_PROPERTY(name, type)                                                                                 \
    ClassDB::bind_method(D_METHOD("set_" #name, "value"), &RailVehicle3D::set_##name);                                 \
    ClassDB::bind_method(D_METHOD("get_" #name), &RailVehicle3D::get_##name);                                          \
    ADD_PROPERTY(PropertyInfo(type, #name), "set_" #name, "get_" #name)
#define BIND_RAIL_NODE_PATH(name, valid_types)                                                                         \
    ClassDB::bind_method(D_METHOD("set_" #name, "value"), &RailVehicle3D::set_##name);                                 \
    ClassDB::bind_method(D_METHOD("get_" #name), &RailVehicle3D::get_##name);                                          \
    ADD_PROPERTY(                                                                                                      \
            PropertyInfo(Variant::NODE_PATH, #name, PROPERTY_HINT_NODE_PATH_VALID_TYPES, valid_types), "set_" #name,   \
            "get_" #name)
#define BIND_RAIL_NODE_PATH_ARRAY(name)                                                                                \
    ClassDB::bind_method(D_METHOD("set_" #name, "value"), &RailVehicle3D::set_##name);                                 \
    ClassDB::bind_method(D_METHOD("get_" #name), &RailVehicle3D::get_##name);                                          \
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, #name, PROPERTY_HINT_ARRAY_TYPE, "NodePath"), "set_" #name, "get_" #name)

        BIND_RAIL_NODE_PATH(model_instance_path, "E3DModelInstance");
        ClassDB::bind_method(D_METHOD("set_lights", "value"), &RailVehicle3D::set_lights);
        ClassDB::bind_method(D_METHOD("get_lights"), &RailVehicle3D::get_lights);
        PropertyInfo lights_property = GetTypeInfo<TypedDictionary<String, bool>>::get_class_info();
        lights_property.name = "lights";
        ADD_PROPERTY(lights_property, "set_lights", "get_lights");
        BIND_RAIL_NODE_PATH(controller_path, "VehicleController,FIZTrainController");
        BIND_RAIL_NODE_PATH(front_bogie_path, "Node3D");
        BIND_RAIL_NODE_PATH(rear_bogie_path, "Node3D");
        BIND_RAIL_NODE_PATH_ARRAY(front_rolling_wheel_paths);
        BIND_RAIL_NODE_PATH_ARRAY(powered_wheel_paths);
        BIND_RAIL_NODE_PATH_ARRAY(rear_rolling_wheel_paths);
        BIND_RAIL_PROPERTY(pantograph_front_offset, Variant::VECTOR3);
        BIND_RAIL_PROPERTY(pantograph_rear_offset, Variant::VECTOR3);
        BIND_RAIL_PROPERTY(pantograph_collector_width, Variant::FLOAT);
        BIND_RAIL_NODE_PATH_ARRAY(pantograph_front_arm_paths);
        BIND_RAIL_NODE_PATH_ARRAY(pantograph_rear_arm_paths);
        BIND_RAIL_NODE_PATH_ARRAY(wiper_arm_paths);
        BIND_RAIL_PROPERTY(coupler_submodel_paths, Variant::DICTIONARY);
        BIND_RAIL_PROPERTY(start_track_name, Variant::STRING);
        BIND_RAIL_PROPERTY(start_track_offset, Variant::FLOAT);
        ClassDB::bind_method(D_METHOD("set_start_direction", "value"), &RailVehicle3D::set_start_direction);
        ClassDB::bind_method(D_METHOD("get_start_direction"), &RailVehicle3D::get_start_direction);
        ADD_PROPERTY(
                PropertyInfo(Variant::INT, "start_direction", PROPERTY_HINT_ENUM, "NORMAL,REVERSED"),
                "set_start_direction", "get_start_direction");
        ClassDB::bind_method(D_METHOD("set_cabin_scene", "value"), &RailVehicle3D::set_cabin_scene);
        ClassDB::bind_method(D_METHOD("get_cabin_scene"), &RailVehicle3D::get_cabin_scene);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "cabin_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                "set_cabin_scene", "get_cabin_scene");
        BIND_RAIL_PROPERTY(cabin_rotate_180deg, Variant::BOOL);
        BIND_RAIL_PROPERTY(joint_cabs, Variant::BOOL);
        BIND_RAIL_NODE_PATH(low_poly_cabin_path, "E3DModelInstance");
        BIND_RAIL_NODE_PATH(load_model_path, "E3DModelInstance");
        BIND_RAIL_PROPERTY(low_poly_cabin_emission_energy, Variant::FLOAT);
        BIND_RAIL_PROPERTY(low_poly_cabin_emission_fade_time, Variant::FLOAT);
        BIND_RAIL_NODE_PATH(head_display_e3d_path, "E3DModelInstance");
        ClassDB::bind_method(D_METHOD("set_head_display_material", "value"), &RailVehicle3D::set_head_display_material);
        ClassDB::bind_method(D_METHOD("get_head_display_material"), &RailVehicle3D::get_head_display_material);
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "head_display_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"),
                "set_head_display_material", "get_head_display_material");
        BIND_RAIL_NODE_PATH(head_display_node_path, "MeshInstance3D");

        ClassDB::bind_method(D_METHOD("enter_cabin", "player"), &RailVehicle3D::enter_cabin);
        ClassDB::bind_method(D_METHOD("leave_cabin", "player"), &RailVehicle3D::leave_cabin);
        ClassDB::bind_method(D_METHOD("get_controller"), &RailVehicle3D::get_controller);
        ClassDB::bind_method(D_METHOD("get_rid"), &RailVehicle3D::get_rid);
        ClassDB::bind_method(D_METHOD("move_on_track", "distance"), &RailVehicle3D::move_on_track);
        ClassDB::bind_method(D_METHOD("_process", "delta"), &RailVehicle3D::process_manually);
        ClassDB::bind_method(D_METHOD("_process_dirty"), &RailVehicle3D::_process_dirty);
        ClassDB::bind_method(D_METHOD("_jump_into_cabin", "cabin", "player"), &RailVehicle3D::_jump_into_cabin);
        ClassDB::bind_method(D_METHOD("_show_cabin_after_frames"), &RailVehicle3D::_show_cabin_after_frames);
        ClassDB::bind_method(
                D_METHOD("_apply_cabin_camera_configuration"), &RailVehicle3D::_apply_cabin_camera_configuration);
        ClassDB::bind_method(D_METHOD("_on_controller_changed", "controller"), &RailVehicle3D::_on_controller_changed);
        ClassDB::bind_method(D_METHOD("_schedule_head_display_update"), &RailVehicle3D::_schedule_head_display_update);
        ClassDB::bind_method(D_METHOD("_on_model_node_e3d_loading"), &RailVehicle3D::_on_model_node_e3d_loading);
        ClassDB::bind_method(D_METHOD("_on_model_node_e3d_loaded"), &RailVehicle3D::_on_model_node_e3d_loaded);
        ClassDB::bind_method(D_METHOD("_on_low_poly_cabin_e3d_loaded"), &RailVehicle3D::_on_low_poly_cabin_e3d_loaded);
        ClassDB::bind_method(
                D_METHOD("_update_low_poly_cabs_visibility"), &RailVehicle3D::_update_low_poly_cabs_visibility);
        ClassDB::bind_method(D_METHOD("_on_roof_light_changed", "enabled"), &RailVehicle3D::_on_roof_light_changed);
        ClassDB::bind_method(
                D_METHOD("_set_low_poly_emission_energy", "value"), &RailVehicle3D::_set_low_poly_emission_energy);
        ClassDB::bind_method(D_METHOD("_on_screen_entered"), &RailVehicle3D::_on_screen_entered);
        ClassDB::bind_method(D_METHOD("_on_screen_exited"), &RailVehicle3D::_on_screen_exited);
        ClassDB::bind_method(
                D_METHOD("_on_track_manager_tracks_changed"), &RailVehicle3D::_on_track_manager_tracks_changed);

#undef BIND_RAIL_NODE_PATH_ARRAY
#undef BIND_RAIL_NODE_PATH
#undef BIND_RAIL_PROPERTY
        /* The vehicle this node renders has changed - it has one now, it has a different one, or
         * it has none. Whoever needs the vehicle reacts to this instead of looking for it again
         * later: a consumer wired up before the vehicle is built would otherwise have to poll. */
        ADD_SIGNAL(MethodInfo(controller_changed_signal));
    }

    void RailVehicle3D::enter_cabin(Node *p_player) {
        if (cabin_scene.is_null()) {
            UtilityFunctions::push_warning(get_name(), " has no cabin_scene; cabin entry not yet supported");
            return;
        }

        camera = Object::cast_to<Node3D>(p_player->call("get_camera"));
        Cabin3D *new_cabin = Object::cast_to<Cabin3D>(cabin_scene->instantiate());
        if (new_cabin == nullptr) {
            UtilityFunctions::push_error("Root node of cabin scene must be a Cabin3D");
            if (new_cabin != nullptr) {
                new_cabin->queue_free();
            }
            return;
        }
        cabin = new_cabin;
        cabin_player = p_player;
        // taking over the vehicle activates its cab when the FIZ allows it (Train.cpp:9147)
        if (controller != nullptr) {
            controller->cab_activation_auto();
        }

        cabin->set_visible(false);
        cabin->connect(
                Cabin3D::cabin_ready_signal, Callable(this, "_jump_into_cabin").bind(cabin, p_player),
                Object::CONNECT_ONE_SHOT);
        cabin->connect(Cabin3D::camera_configuration_changed_signal, callable_mp(this, &RailVehicle3D::_apply_cabin_camera_configuration));
        cabin->connect(Cabin3D::camera_configuration_changed_signal, callable_mp(this, &RailVehicle3D::_update_low_poly_cabs_visibility));
        cabin->set_transform(Transform3D());
        if (cabin_rotate_180deg) {
            cabin->rotate_y(static_cast<real_t>(Math::deg_to_rad(180.0)));
        }
        add_child(cabin);
        /* A cabin names the vehicle it sits in and takes everything else from CabinSystem. Told
         * once it is in the tree, because building its interior puts nodes there, and told here
         * rather than at the next controller change, which for an existing vehicle never comes. */
        cabin->set_train_id(controller != nullptr ? controller->get_train_id() : String());

        cabin_show_frames = 2;
        get_tree()->connect("process_frame", Callable(this, "_show_cabin_after_frames"), Object::CONNECT_ONE_SHOT);
    }

    void RailVehicle3D::_jump_into_cabin(Node3D *p_cabin, Node *p_player) {
        if (cabin != p_cabin) {
            return;
        }

        _update_low_poly_cabs_visibility();

        p_player->remove_child(camera);
        cabin->add_child(camera);
        _apply_cabin_camera_configuration();
        camera->set("velocity_multiplier", 0.2);
    }

    void RailVehicle3D::_show_cabin_after_frames() {
        --cabin_show_frames;
        if (cabin_show_frames > 0) {
            get_tree()->connect("process_frame", Callable(this, "_show_cabin_after_frames"), Object::CONNECT_ONE_SHOT);
        } else if (cabin != nullptr) {
            cabin->set_visible(true);
        }
    }

    void RailVehicle3D::leave_cabin(Node *p_player) {
        Transform3D camera_transform = camera->get_global_transform();
        cabin->remove_child(camera);
        p_player->add_child(camera);
        camera->set("bound_enabled", false);
        camera->set_global_transform(camera_transform);
        camera_transform = camera->get_global_transform();
        camera_transform.origin = get_global_transform().origin + Vector3(5.0, 1.75, 0.0);
        camera->set_global_transform(camera_transform);
        camera->look_at(get_global_position() + Vector3(0.0, 1.75, -5.0));
        camera->set("velocity_multiplier", 1.0);
        cabin->disconnect(Cabin3D::camera_configuration_changed_signal, callable_mp(this, &RailVehicle3D::_apply_cabin_camera_configuration));
        cabin->disconnect(Cabin3D::camera_configuration_changed_signal, callable_mp(this, &RailVehicle3D::_update_low_poly_cabs_visibility));
        cabin->get_parent()->remove_child(cabin);
        cabin->queue_free();
        cabin = nullptr;
        _update_low_poly_cabs_visibility();
    }

    void RailVehicle3D::_apply_cabin_camera_configuration() {
        if (cabin == nullptr || camera == nullptr || camera->get_parent() != cabin) {
            return;
        }
        camera->set("bound_enabled", cabin->get_camera_bound_enabled());
        Vector3 bound_min = cabin->get_camera_bound_min();
        Vector3 bound_max = cabin->get_camera_bound_max();
        bound_min.y += 0.5;
        bound_max.y += 1.8;
        camera->set("bound_min", bound_min);
        camera->set("bound_max", bound_max);
        camera->set_global_transform(cabin->get_camera_transform());
        // Original engine looks along VectorFront * CabOccupied (drivermode.cpp:1071), so cab 2
        // faces the opposite way.
        const bool rear_cab = static_cast<int>(cabin->get_cab_number()) < 0;
        if (cabin_rotate_180deg != rear_cab) {
            camera->set_global_basis(get_global_basis());
        } else {
            camera->set_global_basis(
                    get_global_basis().rotated(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(Math::deg_to_rad(180.0))));
        }
    }

    VehicleController *RailVehicle3D::_resolve_controller(const NodePath &p_node_path) const {
        // a vehicle in the tree is a VehiclePhysicsNode; the controller is the object it owns
        VehiclePhysicsNode *physics = Object::cast_to<VehiclePhysicsNode>(get_node_or_null(p_node_path));
        return physics == nullptr ? nullptr : physics->get_controller();
    }

    RID RailVehicle3D::get_rid() const {
        return rid;
    }

    VehicleController *RailVehicle3D::get_controller() const {
        return controller_path.is_empty() ? nullptr : _resolve_controller(controller_path);
    }

    /* The vehicle this node stands on was rebuilt - take whatever it owns now. */
    /* The vehicle's configuration reached the backend. The bogie placement is derived from it
     * (the pivot spacing), so it is recomputed here - not polled for. */
    /* Where the cargo sits: the original sinks it into the body as the vehicle empties, lerping
     * from the cargo's own offset_min to zero with how full it is (DynObj.cpp:3070-3080), and
     * leaves it alone when that cargo declares no offset. Both numbers are the vehicle's
     * configuration, so this runs when the configuration lands rather than when the model is
     * built - the load component does not exist yet at that point (see `FINDINGS.md`,
     * 2026-09-23, for the same lifecycle biting the bogie spacing). */
    void RailVehicle3D::_apply_load_offset() {
        if (load_model == nullptr || controller == nullptr) {
            return;
        }
        VehicleLoad *load = Object::cast_to<VehicleLoad>(
                controller->get_component(VehicleComponentType::COMPONENT_LOAD));
        if (load == nullptr) {
            return;
        }
        const TypedArray<String> accepted = load->get_accepted_loads();
        const TypedArray<float> offsets = load->get_minimum_load_offsets();
        const String cargo = controller->get_load_name().to_lower();
        double offset_min = 0.0;
        for (int index = 0; index < accepted.size() && index < offsets.size(); ++index) {
            if (String(accepted[index]).to_lower() == cargo) {
                offset_min = double(offsets[index]);
                break;
            }
        }
        if (Math::is_zero_approx(offset_min)) {
            return;
        }
        const double max_load = load->get_max_load();
        const double fill =
                max_load > 0.0 ? CLAMP(controller->get_load_amount() / max_load, 0.0, 1.0) : 0.0;
        Vector3 position = load_model->get_position();
        position.y = static_cast<real_t>(Math::lerp(offset_min, 0.0, fill));
        load_model->set_position(position);
    }

    void RailVehicle3D::_on_vehicle_config_changed() {
        force_detail_refresh = true;
        _apply_load_offset();
        apply_track_placement();
    }

    /* The vehicle this node draws has been (re)built. Everything this node sets up needs a
     * vehicle, so this is where its own initialisation starts - and where processing begins. */
    void RailVehicle3D::_on_vehicle_changed() {
        VehicleController *vehicle = fiz_controller != nullptr ? fiz_controller->get_controller() : nullptr;
        _on_controller_changed(vehicle);
        // the vehicle was rebuilt in place, so its parts changed even though it did not
        _adopt_vehicle_parts();
        dirty = true;
        set_process(true);
    }

    /* Resolving the vehicle node and subscribing to it. Done on entering the tree, before any
     * processing: with the subscription made in _process() instead, a node that waits for its
     * vehicle would never hear about it. */
    void RailVehicle3D::_bind_vehicle_node() {
        Node *controller_node = controller_path.is_empty() ? nullptr : get_node_or_null(controller_path);
        VehiclePhysicsNode *new_fiz_controller = Object::cast_to<VehiclePhysicsNode>(controller_node);
        if (new_fiz_controller == nullptr && !controller_path.is_empty()) {
            const NodePath parent_path = NodePath(String(controller_path).get_base_dir());
            Node *parent_node = parent_path.is_empty() ? nullptr : get_node_or_null(parent_path);
            new_fiz_controller = Object::cast_to<VehiclePhysicsNode>(parent_node);
        }
        if (fiz_controller != new_fiz_controller) {
            if (fiz_controller != nullptr) {
                fiz_controller->disconnect(
                        VehiclePhysicsNode::vehicle_changed_signal,
                        callable_mp(this, &RailVehicle3D::_on_vehicle_changed));
            }
            fiz_controller = new_fiz_controller;
            if (fiz_controller != nullptr) {
                fiz_controller->connect(
                        VehiclePhysicsNode::vehicle_changed_signal,
                        callable_mp(this, &RailVehicle3D::_on_vehicle_changed));
            }
        }
        _on_controller_changed(get_controller());
    }

    /* What the vehicle is made of, re-read from it. Kept apart from taking a *different*
     * controller because a rebuild keeps the same one - the vehicle is first built empty and its
     * components arrive with its model, so a guard on the controller's identity would leave this
     * node holding the parts of the empty vehicle forever. */
    void RailVehicle3D::_adopt_vehicle_parts() {
        electric_engine = nullptr;
        for (int index = 0; index < pantograph_wire_cache.size(); ++index) {
            pantograph_wire_cache[index] = Dictionary();
        }
        if (controller == nullptr) {
            return;
        }
        electric_engine = Object::cast_to<VehicleElectricEngine>(
                controller->get_component(VehicleComponentType::COMPONENT_ENGINE));
        /* The collector's half width belongs to the vehicle, not to this node: the FIZ declares
         * the slider's full width (CSW) and the original halves it (DynObj.cpp:5718). The
         * exported width stands in for a vehicle with no electric engine to read it from. */
        const double sliding_width =
                electric_engine != nullptr ? electric_engine->get_power_current_collector_sliding_width() : 0.0;
        pantograph_slider_half_width = sliding_width > 0.0 ? 0.5 * sliding_width : pantograph_collector_width;
    }

    void RailVehicle3D::_on_controller_changed(VehicleController *p_controller) {
        if (controller == p_controller) {
            return;
        }
        if (controller != nullptr) {
            controller->disconnect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
            controller->disconnect(
                    VehicleController::config_changed,
                    callable_mp(this, &RailVehicle3D::_on_vehicle_config_changed));
        }
        controller = p_controller;
        if (controller != nullptr) {
            controller->connect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
            controller->connect(
                    VehicleController::config_changed,
                    callable_mp(this, &RailVehicle3D::_on_vehicle_config_changed));
            _adopt_vehicle_parts();
        }
        if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
            /* A vehicle has one handle. When the controller already carries one - it does
             * whenever a VehiclePhysicsNode built it - this node renders that vehicle rather than
             * creating a second one, which would step the same controller twice and place only
             * one of the two on a track. */
            const RID vehicle_rid = controller != nullptr ? controller->get_rid() : RID();
            if (vehicle_rid.is_valid() && vehicle_rid != rid) {
                if (rid_owned && rid.is_valid()) {
                    server->vehicle_free(rid);
                }
                rid = vehicle_rid;
                rid_owned = false;
                server->vehicle_attach_rail_vehicle(rid, get_instance_id());
            }
            if (rid.is_valid()) {
                server->vehicle_attach_controller(rid, controller != nullptr ? controller->get_instance_id() : 0);
            }
        }
        if (cabin != nullptr) {
            cabin->set_train_id(controller != nullptr ? controller->get_train_id() : String());
        }
        const Dictionary state = controller != nullptr ? controller->get_state() : Dictionary();
        _on_roof_light_changed(controller != nullptr && bool(state.get("roof_light_enabled", false)));
        emit_signal(controller_changed_signal);
    }

    void RailVehicle3D::_enter_tree() {
        if (TrackManager *tracks = TrackManager::get_instance(); tracks != nullptr) {
            tracks->connect(
                    TrackManager::tracks_changed_signal,
                    callable_mp(this, &RailVehicle3D::_on_track_manager_tracks_changed));
        }
        if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
            rid = server->vehicle_create();
            rid_owned = true;
            server->vehicle_attach_rail_vehicle(rid, get_instance_id());
        }
        pending_start_track_retry = !start_track_name.is_empty();
        dirty = true;
        _bind_vehicle_node();
        /* A node pointed at a vehicle does nothing until that vehicle exists - it would only
         * place and animate itself against a vehicle that is not there yet. One without a
         * vehicle of its own has nothing to wait for. */
        set_process(controller_path.is_empty() || get_controller() != nullptr);
    }

    void RailVehicle3D::_ready() {
        _schedule_head_display_update();
        dirty = true;
        TypedArray<Node> instances = find_children("", "E3DModelInstance", true, false);
        for (int index = 0; index < instances.size(); ++index) {
            Object::cast_to<Node>(instances[index])
                    ->connect("e3d_loaded", Callable(this, "_schedule_head_display_update"));
        }
    }

    void RailVehicle3D::_exit_tree() {
        if (TrackManager *tracks = TrackManager::get_instance(); tracks != nullptr) {
            tracks->disconnect(
                    TrackManager::tracks_changed_signal,
                    callable_mp(this, &RailVehicle3D::_on_track_manager_tracks_changed));
        }
        if (model_node != nullptr) {
            model_node->disconnect("e3d_loading", Callable(this, "_on_model_node_e3d_loading"));
            model_node->disconnect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
            model_node = nullptr;
        }
        // only the handle this node created is this node's to free; an adopted one belongs to
        // the VehiclePhysicsNode that built the vehicle
        if (rid_owned && rid.is_valid()) {
            if (RailVehicleServer *server = RailVehicleServer::get_instance(); server != nullptr) {
                server->vehicle_free(rid);
            }
        }
        rid = RID();
        rid_owned = false;
        if (fiz_controller != nullptr) {
            fiz_controller->disconnect(
                        VehiclePhysicsNode::vehicle_changed_signal,
                        callable_mp(this, &RailVehicle3D::_on_vehicle_changed));
            fiz_controller = nullptr;
        }
        // letting go of the vehicle is the same operation as taking a different one, and it is
        // the only place that disconnects - a second copy of the disconnect here is what made
        // teardown report a connection that was never made
        _on_controller_changed(nullptr);
    }

    void RailVehicle3D::_notification(int p_what) {
        if (p_what == NOTIFICATION_PROCESS) {
            _process_impl(get_process_delta_time());
        }
    }

    void RailVehicle3D::process_manually(const Variant &p_delta) {
        _process_impl(double(p_delta));
    }

    void RailVehicle3D::_process_impl(double p_delta) {
        /* The bindings first: _process_dirty() ends by placing the vehicle on its start track,
         * and placing it positions the bogies - which needs their rest bases. Cached afterwards,
         * the first placement finds none and a parked vehicle never gets a second one. */
        if (animation_bindings_dirty) {
            animation_bindings_dirty = false;
            _cache_animation_bindings();
            force_detail_refresh = true;
        }
        if (dirty) {
            _process_dirty();
        }

        update_time += p_delta;
        if (update_time > 0.25) {
            update_time = 0.0;
            if (needs_head_display_update) {
                _update_head_display();
            }
            _update_model_detail();
            _update_smoke();
        }

        if (!Engine::get_singleton()->is_editor_hint()) {
            if (rid.is_valid() && !start_track_name.is_empty() && !pending_start_track_retry) {
                // the placement itself is applied by RailVehicleServer at the end of its step
                if (electric_engine != nullptr) {
                    const Dictionary state = controller->get_state();
                    _update_pantograph_raise_state(p_delta, state);
                    _update_pantograph_power(state);
                }
            } else if (controller != nullptr && start_track_name.is_empty()) {
                const double velocity = controller->get_velocity();
                const real_t distance = static_cast<real_t>(p_delta * velocity);
                set_position(get_position() + (Vector3(0.0, 0.0, -1.0) * distance));
                if (is_visible && !Math::is_zero_approx(velocity)) {
                    _update_wheel_animation_state();
                }
                if (electric_engine != nullptr) {
                    const Dictionary state = controller->get_state();
                    _update_pantograph_raise_state(p_delta, state);
                    _update_pantograph_power(state);
                }
            }
            if (controller != nullptr) {
                _sync_lights_from_controller();
                if (is_visible) {
                    _update_couplers();
                    _update_wipers();
                }
            }
        }
    }

    void RailVehicle3D::_schedule_head_display_update() {
        needs_head_display_update = true;
    }

    void RailVehicle3D::_update_head_display() {
        if (!is_inside_tree()) {
            return;
        }
        if (!head_display_node_path.is_empty()) {
            MeshInstance3D *node = node_at<MeshInstance3D>(this, head_display_node_path);
            if (node != nullptr) {
                node->set_material_override(head_display_material);
            }
        }
        needs_head_display_update = false;
    }

    void RailVehicle3D::_process_dirty() {
        dirty = false;
        if (!head_display_e3d_path.is_empty()) {
            head_display_e3d = get_node_or_null(head_display_e3d_path);
            if (head_display_e3d != nullptr) {
                head_display_e3d->connect("e3d_loaded", Callable(this, "_schedule_head_display_update"));
            }
        }

        if (!is_inside_tree()) {
            return;
        }

        _bind_vehicle_node();

        Node3D *new_model_node = model_instance_path.is_empty() ? nullptr : node_at<Node3D>(this, model_instance_path);
        if (model_node != nullptr) {
            model_node->disconnect("e3d_loading", Callable(this, "_on_model_node_e3d_loading"));
            model_node->disconnect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
        }
        model_node = new_model_node;
        if (model_node != nullptr) {
            model_node->connect("e3d_loading", Callable(this, "_on_model_node_e3d_loading"));
            model_node->connect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
        }
        _sync_model_lights();
        _update_detection_area();

        if (low_poly_cabin != nullptr) {
            low_poly_cabin->disconnect("e3d_loaded", Callable(this, "_on_low_poly_cabin_e3d_loaded"));
        }
        load_model = load_model_path.is_empty() ? nullptr : node_at<Node3D>(this, load_model_path);
        _apply_load_offset();
        low_poly_cabin = low_poly_cabin_path.is_empty() ? nullptr : node_at<Node3D>(this, low_poly_cabin_path);
        if (low_poly_cabin != nullptr) {
            low_poly_cabin->connect("e3d_loaded", Callable(this, "_on_low_poly_cabin_e3d_loaded"));
            if (bool(low_poly_cabin->call("is_e3d_loaded"))) {
                _on_low_poly_cabin_e3d_loaded();
            }
        }

        if (pending_start_track_retry) {
            _apply_start_track();
        }
    }

    void RailVehicle3D::_sync_model_lights() {
        if (model_node == nullptr || !bool(model_node->call("is_e3d_loaded"))) {
            return;
        }
        const TypedDictionary<String, bool> model_lights = model_node->get("lights_state");
        lights.merge(model_lights, false);
        const Array keys = lights.keys();
        for (int index = 0; index < keys.size(); ++index) {
            if (!model_lights.has(keys[index])) {
                lights.erase(keys[index]);
            }
        }
        lights.sort();
        model_node->set("lights_state", lights);
    }

    void RailVehicle3D::_sync_lights_from_controller() {
        if (model_node == nullptr || !bool(model_node->call("is_e3d_loaded"))) {
            return;
        }
        bool changed = false;
        const Dictionary state = controller->get_state();
        const Array keys = lights.keys();
        for (int index = 0; index < keys.size(); ++index) {
            const Variant &light_name = keys[index];
            const String light_name_string = light_name;
            for (const LightStateBinding &binding: LIGHT_STATE_BINDINGS) {
                if (light_name_string == binding.light_name) {
                    const bool new_value = state.get(binding.state_name, false);
                    if (bool(lights[light_name]) != new_value) {
                        lights[light_name] = new_value;
                        changed = true;
                    }
                    break;
                }
            }
        }
        if (changed) {
            model_node->set("lights_state", lights);
        }
    }

    /* The model is about to free and rebuild its children, so every node cached out of it is
     * about to dangle. Dropped here, at the event that announces it - RailVehicleServer's tick
     * reaches apply_track_placement() before this node's own _process would. */
    void RailVehicle3D::_on_model_node_e3d_loading() {
        front_bogie_node = nullptr;
        rear_bogie_node = nullptr;
        front_rolling_wheel_nodes.clear();
        powered_wheel_nodes.clear();
        rear_rolling_wheel_nodes.clear();
        node_rest_bases.clear();
        bogie_rest_global_bases.clear();
    }

    /* Emitted after the model has put its children in place (E3DModelInstance.reload()), so the
     * bindings are taken here rather than left to a flag the next frame would consume. */
    void RailVehicle3D::_on_model_node_e3d_loaded() {
        _sync_model_lights();
        _update_detection_area();
        animation_bindings_dirty = false;
        _cache_animation_bindings();
        force_detail_refresh = true;
    }

    void RailVehicle3D::_on_low_poly_cabin_e3d_loaded() {
        low_poly_emissive_materials.clear();
        TypedArray<Node> mesh_instances = low_poly_cabin->find_children("", "MeshInstance3D", true, false);
        for (int index = 0; index < mesh_instances.size(); ++index) {
            MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(mesh_instances[index]);
            Ref<Material> material = mesh_instance->get_material_override();
            Ref<ShaderMaterial> shader_material = material;
            if (shader_material.is_valid() && bool(shader_material->get_shader_parameter("emission_enabled"))) {
                shader_material = shader_material->duplicate();
                mesh_instance->set_material_override(shader_material);
                low_poly_emissive_materials.append(shader_material);
            }
        }
        const bool roof_light_enabled =
                controller != nullptr && bool(controller->get_state().get("roof_light_enabled", false));
        _set_low_poly_emission_energy(roof_light_enabled ? low_poly_cabin_emission_energy : 0.0);
        _update_low_poly_cabs_visibility();
    }

    // Original engine: the low-poly interior stays rendered from inside the cab (Render_interior(),
    // opengl33renderer.cpp:1123); only its occupied "cabN" submodel is hidden - or all of them
    // with jointcabs: - so the hi-fi cab doesn't overlap it (DynObj.cpp:1211-1219, 2236-2250).
    // A cab without a hi-fi model keeps every low-poly cab visible (DynObj.cpp:1214).
    void RailVehicle3D::_update_low_poly_cabs_visibility() {
        if (low_poly_cabin == nullptr) {
            return;
        }
        const int cab_number = cabin == nullptr ? 0 : static_cast<int>(cabin->get_cab_number());
        const int occupied_cab_index = cab_number < 0 ? 2 : cab_number;
        const bool hifi_cab = cabin != nullptr && cabin->get_has_cab_model();
        for (int cab_index = 0; cab_index < 3; ++cab_index) {
            const String cab_name = "cab" + itos(cab_index);
            Node3D *cab_node = Object::cast_to<Node3D>(low_poly_cabin->find_child(cab_name, true, false));
            if (cab_node != nullptr) {
                cab_node->set_visible(!hifi_cab || (!joint_cabs && cab_index != occupied_cab_index));
                continue;
            }
            // Not finding these is why the low-poly interior ends up drawn over the modelled
            // cabin, and it used to happen without a word in the log
            if (cab_index == occupied_cab_index && hifi_cab) {
                const String message =
                        vformat("RailVehicle3D '%s': no '%s' node under '%s' to hide, the low-poly interior will cover "
                                "the cabin. Children: %d, loaded: %s",
                                get_name(), cab_name, low_poly_cabin->get_name(), low_poly_cabin->get_child_count(),
                                bool(low_poly_cabin->call("is_e3d_loaded")));
                UtilityFunctions::push_warning(message);
                GameLog *game_log = GameLog::get_instance();
                if (game_log != nullptr) {
                    game_log->error(message);
                }
            }
        }
    }

    void RailVehicle3D::_on_roof_light_changed(bool p_enabled) {
        if (low_poly_emission_tween.is_valid()) {
            low_poly_emission_tween->kill();
        }
        const double target_energy = p_enabled ? low_poly_cabin_emission_energy : 0.0;
        double current_energy = target_energy;
        if (!low_poly_emissive_materials.is_empty()) {
            Ref<ShaderMaterial> material = low_poly_emissive_materials[0];
            current_energy = material->get_shader_parameter("emission_energy");
        }
        low_poly_emission_tween = create_tween();
        low_poly_emission_tween->tween_method(
                Callable(this, "_set_low_poly_emission_energy"), current_energy, target_energy,
                low_poly_cabin_emission_fade_time);
    }

    void RailVehicle3D::_set_low_poly_emission_energy(double p_value) {
        for (int index = 0; index < low_poly_emissive_materials.size(); ++index) {
            Ref<ShaderMaterial> material = low_poly_emissive_materials[index];
            material->set_shader_parameter("emission_energy", p_value);
        }
    }

    void RailVehicle3D::_update_detection_area() {
        if (Engine::get_singleton()->is_editor_hint() || model_node == nullptr ||
            !bool(model_node->call("is_e3d_loaded"))) {
            return;
        }
        const AABB aabb = model_node->call("get_aabb");
        if (aabb.size == Vector3()) {
            return;
        }

        if (detection_area == nullptr) {
            detection_area = memnew(Area3D);
            detection_area->set_name("RailVehicleDetectionArea");
            detection_area->set_monitoring(false);
            CollisionShape3D *shape_node = memnew(CollisionShape3D);
            Ref<BoxShape3D> box;
            box.instantiate();
            shape_node->set_shape(box);
            detection_area->add_child(shape_node);
            add_child(detection_area);
        }
        detection_area->set_transform(model_node->get_transform());
        CollisionShape3D *shape_node = Object::cast_to<CollisionShape3D>(detection_area->get_child(0));
        Ref<BoxShape3D> box = shape_node->get_shape();
        box->set_size(aabb.size);
        shape_node->set_position(aabb.get_center());

        if (visibility_notifier == nullptr) {
            visibility_notifier = memnew(VisibleOnScreenNotifier3D);
            visibility_notifier->set_name("RailVehicleVisibilityNotifier");
            visibility_notifier->connect("screen_entered", Callable(this, "_on_screen_entered"));
            visibility_notifier->connect("screen_exited", Callable(this, "_on_screen_exited"));
            add_child(visibility_notifier);
        }
        visibility_notifier->set_transform(model_node->get_transform());
        visibility_notifier->set_aabb(aabb);
    }

    void RailVehicle3D::_on_screen_entered() {
        is_visible = true;
        force_detail_refresh = true;
    }

    void RailVehicle3D::_on_screen_exited() {
        is_visible = false;
    }

    void RailVehicle3D::move_on_track(double p_distance) {
        if (!rid.is_valid()) {
            return;
        }
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr) {
            return;
        }
        server->vehicle_move(rid, p_distance);
        apply_track_placement();
    }

    void RailVehicle3D::_on_track_manager_tracks_changed() {
        if (pending_start_track_retry) {
            _apply_start_track();
        }
    }

    void RailVehicle3D::_apply_start_track() {
        if (start_track_name.is_empty()) {
            return;
        }
        TrackManager *tracks = TrackManager::get_instance();
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (tracks == nullptr || server == nullptr) {
            return;
        }
        const RID track_rid = tracks->track_get_rid_by_name(start_track_name);
        if (!track_rid.is_valid()) {
            return;
        }
        pending_start_track_retry = false;
        server->vehicle_set_track(
                rid, track_rid, start_track_offset, static_cast<TrackManager::Direction>(start_direction));
        apply_track_placement();
    }

    TypedArray<Node3D> RailVehicle3D::_resolve_animation_nodes(const TypedArray<NodePath> &p_paths) const {
        TypedArray<Node3D> nodes;
        for (int index = 0; index < p_paths.size(); ++index) {
            const NodePath path = p_paths[index];
            if (!path.is_empty()) {
                Node3D *node = node_at<Node3D>(const_cast<RailVehicle3D *>(this), path);
                if (node != nullptr) {
                    nodes.append(node);
                }
            }
        }
        return nodes;
    }

    void RailVehicle3D::_capture_rest_basis(Node3D *p_node) {
        if (p_node != nullptr && !node_rest_bases.has(p_node)) {
            node_rest_bases[p_node] = p_node->get_transform().basis.orthonormalized();
        }
    }

    TypedArray<Node3D> RailVehicle3D::_resolve_pantograph_arm_nodes(const TypedArray<NodePath> &p_paths) const {
        TypedArray<Node3D> nodes;
        if (p_paths.size() != 5) {
            return nodes;
        }
        for (int index = 0; index < p_paths.size(); ++index) {
            const NodePath path = p_paths[index];
            nodes.append(path.is_empty() ? nullptr : node_at<Node3D>(const_cast<RailVehicle3D *>(this), path));
        }
        return nodes;
    }

    Dictionary RailVehicle3D::_cache_pantograph_geometry(const TypedArray<Node3D> &p_nodes) const {
        if (p_nodes.size() != 5) {
            return {};
        }
        // the second arm of each pair is optional - a single-arm pantograph has none
        Node3D *lower = Object::cast_to<Node3D>(p_nodes[0]);
        Node3D *upper = Object::cast_to<Node3D>(p_nodes[2]);
        Node3D *slider = Object::cast_to<Node3D>(p_nodes[4]);
        if (lower == nullptr || upper == nullptr || slider == nullptr) {
            return {};
        }
        const Vector3 lower_to_upper =
                lower->get_global_basis().inverse().xform(upper->get_global_position() - lower->get_global_position());
        const double len_l1 = Vector2(lower_to_upper.y, lower_to_upper.z).length();
        const double angle_l0 = Math::atan2(Math::abs(lower_to_upper.z), Math::abs(lower_to_upper.y));
        double horizontal = -Math::abs(lower_to_upper.y);
        const Vector3 upper_to_slider =
                upper->get_global_basis().inverse().xform(slider->get_global_position() - upper->get_global_position());
        const double len_u1 = Vector2(upper_to_slider.y, upper_to_slider.z).length();
        const double angle_u0 = Math::atan2(Math::abs(upper_to_slider.z), Math::abs(upper_to_slider.y));
        horizontal += Math::abs(upper_to_slider.y);
        if (len_l1 <= 0.0 || len_u1 <= 0.0) {
            return {};
        }
        constexpr double HEIGHT = 0.07;
        Dictionary geometry;
        geometry["len_l1"] = len_l1;
        geometry["len_u1"] = len_u1;
        geometry["horiz"] = horizontal;
        geometry["angle_l0"] = angle_l0;
        geometry["angle_u0"] = angle_u0;
        geometry["angle_l"] = angle_l0;
        geometry["angle_u"] = angle_u0;
        geometry["height"] = HEIGHT;
        geometry["pant_wys"] = (len_l1 * Math::sin(angle_l0)) + (len_u1 * Math::sin(angle_u0)) + HEIGHT;
        return geometry;
    }

    void RailVehicle3D::_cache_animation_bindings() {
        front_bogie_node = node_at<Node3D>(this, front_bogie_path);
        rear_bogie_node = node_at<Node3D>(this, rear_bogie_path);
        front_rolling_wheel_nodes = _resolve_animation_nodes(front_rolling_wheel_paths);
        powered_wheel_nodes = _resolve_animation_nodes(powered_wheel_paths);
        rear_rolling_wheel_nodes = _resolve_animation_nodes(rear_rolling_wheel_paths);
        node_rest_bases.clear();
        bogie_rest_global_bases.clear();

        Node3D *bogies[] = {front_bogie_node, rear_bogie_node};
        for (Node3D *bogie: bogies) {
            if (bogie != nullptr) {
                _capture_rest_basis(bogie);
                bogie_rest_global_bases[bogie] = get_global_basis().inverse() * bogie->get_global_basis();
            }
        }
        TypedArray<Node3D> wheel_arrays[] = {front_rolling_wheel_nodes, powered_wheel_nodes, rear_rolling_wheel_nodes};
        for (const TypedArray<Node3D> &wheel_nodes: wheel_arrays) {
            for (int index = 0; index < wheel_nodes.size(); ++index) {
                _capture_rest_basis(Object::cast_to<Node3D>(wheel_nodes[index]));
            }
        }

        coupler_submodel_nodes.clear();
        coupler_visibility_state = -1;
        const Array coupler_names = coupler_submodel_paths.keys();
        for (int index = 0; index < coupler_names.size(); ++index) {
            if (Node3D *node = node_at<Node3D>(this, coupler_submodel_paths[coupler_names[index]]); node != nullptr) {
                coupler_submodel_nodes[coupler_names[index]] = node;
            }
        }

        wiper_arm_nodes.clear();
        wiper_applied_positions.clear();
        for (int index = 0; index < wiper_arm_paths.size(); ++index) {
            const NodePath path = wiper_arm_paths[index];
            Node3D *node = path.is_empty() ? nullptr : node_at<Node3D>(this, path);
            _capture_rest_basis(node);
            wiper_arm_nodes.append(node);
        }

        pantograph_front_arm_nodes = _resolve_pantograph_arm_nodes(pantograph_front_arm_paths);
        pantograph_rear_arm_nodes = _resolve_pantograph_arm_nodes(pantograph_rear_arm_paths);
        pantograph_front_geometry = _cache_pantograph_geometry(pantograph_front_arm_nodes);
        pantograph_rear_geometry = _cache_pantograph_geometry(pantograph_rear_arm_nodes);
        pantograph_front_converged = pantograph_front_geometry.is_empty();
        pantograph_rear_converged = pantograph_rear_geometry.is_empty();
        TypedArray<Node3D> arm_arrays[] = {pantograph_front_arm_nodes, pantograph_rear_arm_nodes};
        for (const TypedArray<Node3D> &arm_nodes: arm_arrays) {
            for (int index = 0; index < arm_nodes.size(); ++index) {
                _capture_rest_basis(Object::cast_to<Node3D>(arm_nodes[index]));
            }
        }
    }

    // Original engine: AirCoupler::GetStatus() (AirCoupler.cpp:30) - 2 with a slanted (_xon), 1 with a
    // straight (_on) connected submodel
    int RailVehicle3D::_air_coupler_status(const String &p_name) const {
        if (coupler_submodel_nodes.has(p_name + String("_xon"))) {
            return 2;
        }
        return coupler_submodel_nodes.has(p_name + String("_on")) ? 1 : 0;
    }

    // Original engine: TDynamicObject::GetPneumatic() (DynObj.cpp:395) - which hoses the model has at
    // that end: 1 left, 2 right (the "r" variant), 3 both
    int RailVehicle3D::get_pneumatic_layout(const int p_end, const bool p_brake_hose) const {
        const String name = String(p_brake_hose ? "cpneumatic" : "pneumatic") + itos(p_end + 1);
        const int left = _air_coupler_status(name);
        const int right = _air_coupler_status(name + String("r"));
        if (left > 0 && right > 0) {
            return 3;
        }
        return left > 0 ? 1 : (right > 0 ? 2 : 0);
    }

    // Original engine: TDynamicObject::SetPneumatic() (DynObj.cpp:430) - picks the hose submodel
    // matching the layout of the vehicle coupled at that end: 1 straight, 2 slanted, 3 slanted "r",
    // 4 straight "r"
    int RailVehicle3D::_pneumatic_variant(const int p_end, const bool p_brake_hose) const {
        const VehicleBuffCoupl *coupler = _coupler();
        if (coupler == nullptr) {
            return 0;
        }
        const VehicleBuffCoupl::End end = static_cast<VehicleBuffCoupl::End>(p_end);
        const int own = get_pneumatic_layout(p_end, p_brake_hose);
        int other = 0;
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (VehicleController *other_controller = controller->get_coupled_controller(p_end);
            other_controller != nullptr && server != nullptr) {
            const ObjectID other_id = ObjectID(server->vehicle_get_rail_vehicle(other_controller->get_rid()));
            if (const RailVehicle3D *other_vehicle = Object::cast_to<RailVehicle3D>(ObjectDB::get_instance(other_id));
                other_vehicle != nullptr) {
                other = other_vehicle->get_pneumatic_layout(coupler->get_connected_end(end), p_brake_hose);
            }
        }
        if (own == other) {
            switch (own) {
                case 1:
                    return 2;
                case 2:
                    return 3;
                case 3:
                    return coupler->is_coupling_owner(end) ? 1 : 4;
                default:
                    return 0;
            }
        }
        if (own == 3) {
            return other == 1 ? 4 : 1;
        }
        return own == 2 ? 4 : (own == 1 ? 1 : 0);
    }

    // Original engine: AirCoupler::Update() (AirCoupler.cpp:83)
    void RailVehicle3D::_show_air_coupler(const String &p_name, const bool p_on, const bool p_xon) {
        const bool states[] = {p_on, !(p_on || p_xon), p_xon};
        const char *suffixes[] = {"_on", "_off", "_xon"};
        for (int index = 0; index < 3; ++index) {
            if (Node3D *node = Object::cast_to<Node3D>(
                        coupler_submodel_nodes.get(p_name + String(suffixes[index]), Variant()));
                node != nullptr) {
                node->set_visible(states[index]);
            }
        }
    }

    const VehicleBuffCoupl *RailVehicle3D::_coupler() const {
        return controller != nullptr
                       ? Object::cast_to<VehicleBuffCoupl>(
                                 controller->get_component(VehicleComponentType::COMPONENT_BUFFERS))
                       : nullptr;
    }

    // Original engine: coupler and hose submodel visibility (DynObj.cpp:758-925, bnewAirCouplers branch)
    void RailVehicle3D::_update_couplers() {
        const VehicleBuffCoupl *coupler = _coupler();
        if (coupler_submodel_nodes.is_empty() || coupler == nullptr) {
            return;
        }
        int variants[2][3];
        int64_t state = 0;
        for (int end = 0; end < 2; ++end) {
            const VehicleBuffCoupl::End vehicle_end = static_cast<VehicleBuffCoupl::End>(end);
            // _on for the vehicle that draws the coupler, _xon (or _off without it) for the other
            variants[end][0] = !coupler->is_coupled(vehicle_end) ? 0 : (coupler->is_coupling_owner(vehicle_end) ? 1 : 2);
            variants[end][1] = coupler->is_brake_hose_connected(vehicle_end) ? _pneumatic_variant(end, true) : 0;
            variants[end][2] = coupler->is_main_hose_connected(vehicle_end) ? _pneumatic_variant(end, false) : 0;
            for (const int variant: variants[end]) {
                state = state * 5 + variant;
            }
        }
        if (state == coupler_visibility_state) {
            return;
        }
        coupler_visibility_state = state;
        for (int end = 0; end < 2; ++end) {
            const String number = itos(end + 1);
            _show_air_coupler(
                    "coupler" + number, variants[end][0] == 1,
                    variants[end][0] == 2 && coupler_submodel_nodes.has("coupler" + number + "_xon"));
            const char *hoses[] = {"cpneumatic", "pneumatic"};
            for (int hose = 0; hose < 2; ++hose) {
                const String name = String(hoses[hose]) + number;
                const int variant = variants[end][hose + 1];
                _show_air_coupler(name, variant == 1, variant == 2);
                _show_air_coupler(name + String("r"), variant == 4, variant == 3);
            }
        }
    }

    // TDynamicObject::UpdateWiper() (DynObj.cpp:716-731): both arms swing by the wiper angle, the
    // blade swings back by it to stay upright; every other wiper is mirrored.
    /* FIXME(#184): a wiper's position is simulation, not drawing - the node should be handed
     * where the blades are, the way it is handed the state of a light. */
    void RailVehicle3D::_update_wipers() {
        if (wiper_arm_nodes.is_empty()) {
            return;
        }
        const PackedFloat64Array positions = controller->get_state().get("wiper_positions", PackedFloat64Array());
        if (positions == wiper_applied_positions) {
            return;
        }
        wiper_applied_positions = positions;
        const double wiper_angle = Math::deg_to_rad(double(controller->get_config().get("wipers_angle", 0.0)));
        for (int wiper = 0; wiper < positions.size() && (wiper + 1) * 3 <= wiper_arm_nodes.size(); ++wiper) {
            // the state tells the way out (0..1) from the way back (1..2)
            double sweep = positions[wiper] > 1.0 ? positions[wiper] - 1.0 : positions[wiper];
            // smoothInterpolate() (utilities.h:324)
            sweep = sweep * sweep * (3.0 - 2.0 * sweep);
            const double angle = (wiper % 2 == 1 ? -wiper_angle : wiper_angle) * sweep;
            for (int element = 0; element < 3; ++element) {
                Node3D *node = Object::cast_to<Node3D>(wiper_arm_nodes[wiper * 3 + element]);
                if (node == nullptr) {
                    continue;
                }
                Transform3D transform = node->get_transform();
                transform.basis = Basis(node_rest_bases[node]) *
                                  Basis(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(element == 2 ? -angle : angle));
                node->set_transform(transform);
            }
        }
    }

    void RailVehicle3D::_apply_wheel_rotation(const TypedArray<Node3D> &p_nodes, double p_angle_degrees) {
        const double radians = Math::deg_to_rad(p_angle_degrees);
        for (int index = 0; index < p_nodes.size(); ++index) {
            Node3D *node = Object::cast_to<Node3D>(p_nodes[index]);
            const Variant rest_basis = node_rest_bases.get(node, Variant());
            if (rest_basis.get_type() == Variant::BASIS) {
                Transform3D transform = node->get_transform();
                transform.basis = Basis(rest_basis) * Basis(Vector3(1.0, 0.0, 0.0), static_cast<real_t>(radians));
                node->set_transform(transform);
            }
        }
    }

    void RailVehicle3D::_update_wheel_animation_state() {
        const VehicleWheels *wheels =
                controller != nullptr
                        ? Object::cast_to<VehicleWheels>(
                                  controller->get_component(VehicleComponentType::COMPONENT_WHEELS))
                        : nullptr;
        if (wheels == nullptr) {
            return;
        }
        // Same sign as the original's UpdateAxle() (DynObj.cpp:489) - the wheel submodels live in
        // the MaSzyna vehicle frame, which MaszynaRailVehicle3DInstancer converts as a whole.
        _apply_wheel_rotation(front_rolling_wheel_nodes, wheels->get_angle_front_deg());
        _apply_wheel_rotation(powered_wheel_nodes, wheels->get_angle_powered_deg());
        _apply_wheel_rotation(rear_rolling_wheel_nodes, wheels->get_angle_rear_deg());
    }

    /// A vehicle far from the camera is rendered from RenderingServer instances instead of a node
    /// hierarchy: nothing animates at that distance, and the hierarchy is what costs - hundreds of
    /// Node3Ds per vehicle to walk, notify and propagate a transform through, times the hundreds of
    /// vehicles a scenery runs. Its simulation is untouched, it lives in RailVehicleServer.
    ///
    /// The vehicle keeps its transform applied either way, so it stays where it belongs; only the
    /// model's instancer changes. Note the OPTIMIZED backend does not render SUBMODEL_FREE_SPOTLIGHT
    /// (see TODO.md), so a distant vehicle loses its lights.
    void RailVehicle3D::_update_model_detail() {
        const SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        if (streaming == nullptr || !streaming->has_camera() || model_node == nullptr) {
            return;
        }
        const float detail_distance = ProjectSettings::get_singleton()->get_setting(
                "maszyna/vehicles/detail_distance", DEFAULT_VEHICLE_DETAIL_DISTANCE_M);
        const double distance = get_global_position().distance_to(streaming->get_camera_position());
        const float hysteresis = MAX(VEHICLE_DETAIL_HYSTERESIS_MIN_M, detail_distance * VEHICLE_DETAIL_HYSTERESIS);
        const bool detailed = model_detailed ? distance <= detail_distance : distance <= detail_distance - hysteresis;
        if (detailed == model_detailed) {
            return;
        }
        model_detailed = detailed;
        // E3DModelInstance applies a changed instancer only in the editor, so ask it to rebuild
        model_node->set("instancer", detailed ? 1 : 0); // Instancer.NODES : Instancer.OPTIMIZED
        model_node->call("reload");
        // its e3d_loaded brings the cabN visibility and the dimmed materials back with the nodes
        if (low_poly_cabin != nullptr) {
            low_poly_cabin->set("instancer", detailed ? 1 : 0);
            low_poly_cabin->call("reload");
        }
        // the bogie and wheel nodes are gone with the hierarchy, and new ones come back with it
        animation_bindings_dirty = true;
        force_detail_refresh = true;
    }

    /// Drives the particle emitters the model carries. The rate follows the original
    /// (smoke_source::update(), particles.cpp:172-211) and the opacity its dizel_fill
    /// (particles.cpp:330), but only for a diesel: the original runs these branches for every
    /// engine type and reads the diesel-electric characteristic even on an electric. A vehicle
    /// without a diesel engine keeps the template's own rate, like a scenery chimney.
    /// Called from the 0.25 s block of _process_impl() - a plume changes slowly.
    void RailVehicle3D::_update_smoke() {
        if (model_node == nullptr || controller == nullptr || !bool(model_node->call("is_e3d_loaded"))) {
            return;
        }
        const Dictionary state = controller->get_state();
        const int engine_type = state.get("engine_type", VehicleEngine::NONE);
        if (engine_type != VehicleEngine::DIESEL && engine_type != VehicleEngine::DIESEL_ELECTRIC) {
            return;
        }

        const double revolutions = state.get("engine_rpm_count", 0.0); // rev/s, as the Mover keeps enrot
        const double max_rpm = state.get("diesel_max_rpm", 0.0);       // rev/min, the top notch of the characteristic
        const double power = state.get("engine_power", 0.0);           // kW
        const double current = state.get("engine_current", 0.0);
        const double direction = state.get("direction_absolute", 0.0);

        double intensity;
        if (bool(state.get("diesel_spinup", false))) {
            intensity = revolutions / 4.0 * 0.01;
        } else {
            // The original compares rev/min against rev/s (particles.cpp:196), which leaves the
            // deficit nearly constant and makes the rate track the engine power. Kept as it is:
            // reading both in rev/min would stop a diesel from smoking at full revs, which is
            // where it smokes most.
            const double revolutions_deficit = (max_rpm - revolutions) / 60.0;
            const double load = power * 0.005;
            if (Math::is_zero_approx(direction) || Math::is_zero_approx(current)) {
                intensity = revolutions_deficit * 0.02 * load;
            } else {
                intensity = revolutions_deficit * (Math::sqrt(Math::abs(current)) * 0.01) * 0.02 * load;
            }
        }

        // dizel_fill scales the opacity of a newly born particle in the original
        // (particles.cpp:330). Godot has no channel for that which does not also reach the
        // particles already in the air, so it scales how many are born instead - the plume thins
        // out rather than stepping down as a whole (see FINDINGS.md). The original also lets the
        // revolutions deficit go negative and subtract from the particle budget; this clamps.
        const double fill = CLAMP(double(state.get("diesel_fill", 0.0)), 0.0, 1.0);
        model_node->call("set_smoke_intensity", CLAMP(intensity, 0.0, 1.0) * fill);
    }

    void RailVehicle3D::apply_track_placement() {
        if (!rid.is_valid() || start_track_name.is_empty() || pending_start_track_retry) {
            return;
        }
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr) {
            return;
        }
        const Transform3D body_transform = server->vehicle_get_transform(rid);
        const bool moved = body_transform != last_body_transform;
        last_body_transform = body_transform;

        /* The body's transform is the server's answer and nothing else. It used to be written
         * twice here - the server's, then one this node composed from the bogies - and the two
         * differ on anything but straight track, so a parked vehicle on a curve flicked between
         * them whenever something raised force_detail_refresh. The composition moved to the
         * server, which owns the placement it is made of. */
        set_global_transform(body_transform);

        if (!is_visible || (!moved && !force_detail_refresh)) {
            return;
        }
        force_detail_refresh = false;
        if (front_bogie_node == nullptr && rear_bogie_node == nullptr) {
            _update_wheel_animation_state();
            return;
        }
        if (front_bogie_node == nullptr || rear_bogie_node == nullptr) {
            if (!bogie_configuration_warned) {
                bogie_configuration_warned = true;
                UtilityFunctions::push_warning(
                        "RailVehicle3D '", get_name(), "': front and rear bogie paths must be set together.");
            }
            _update_wheel_animation_state();
            return;
        }

        bogie_configuration_warned = false;
        // the running gear is the wheels' business: they know the pivot spacing and where each
        // bogie sits. This node only puts the nodes there.
        VehicleWheels *wheels =
                controller != nullptr
                        ? Object::cast_to<VehicleWheels>(
                                  controller->get_component(VehicleComponentType::COMPONENT_WHEELS))
                        : nullptr;
        if (wheels == nullptr) {
            _update_wheel_animation_state();
            return;
        }
        const Transform3D front_transform = wheels->get_bogie_transform(VehicleWheels::BOGIE_FRONT);
        const Transform3D rear_transform = wheels->get_bogie_transform(VehicleWheels::BOGIE_REAR);
        Vector3 body_forward = front_transform.origin - rear_transform.origin;
        if (body_forward.is_zero_approx()) {
            _update_wheel_animation_state();
            return;
        }
        body_forward.normalize();
        const double body_yaw = Math::atan2(-body_forward.x, body_forward.z);
        Node3D *bogie_nodes[] = {front_bogie_node, rear_bogie_node};
        Transform3D bogie_transforms[] = {front_transform, rear_transform};
        for (int index = 0; index < 2; ++index) {
            const Vector3 bogie_forward = -bogie_transforms[index].basis.get_column(2).normalized();
            const double bogie_yaw = Math::atan2(-bogie_forward.x, bogie_forward.z);
            const Variant rest_basis = bogie_rest_global_bases.get(bogie_nodes[index], Variant());
            if (rest_basis.get_type() == Variant::BASIS) {
                const double yaw_delta = -(bogie_yaw - body_yaw);
                bogie_nodes[index]->set_global_basis(
                        get_global_basis() * Basis(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(yaw_delta)) *
                        Basis(rest_basis));
            }
        }
        _update_wheel_animation_state();
    }

    RailVehicle3D::PantographFrame RailVehicle3D::_pantograph_frame() const {
        PantographFrame frame;
        frame.transform = get_global_transform();
        frame.forward = -frame.transform.basis.get_column(2);
        frame.up = frame.transform.basis.get_column(1);
        frame.left = -frame.transform.basis.get_column(0);
        return frame;
    }

    /* Where the vehicle is, in the terms a scenery is written in: a warning that only carries
     * world coordinates cannot be looked up in the .scn that produced the wiring. */
    String RailVehicle3D::_track_position_text() const {
        RailVehicleServer *server = RailVehicleServer::get_instance();
        TrackManager *tracks = TrackManager::get_instance();
        if (server == nullptr || tracks == nullptr) {
            return String("unknown track");
        }
        const Dictionary placement = server->vehicle_get_track_position(rid);
        const RID track = placement.get("track_rid", RID());
        if (!track.is_valid()) {
            return String("no track");
        }
        const String name = tracks->track_get_name(track);
        return vformat(
                "%s at %.2f m", name.is_empty() ? String("(unnamed track)") : name,
                double(placement.get("along", 0.0)));
    }

    /* The third way a raised pantograph reads no voltage, and the only one that is not about the
     * wire: the arm has not reached it (PantDiff >= 0.01, DynObj.cpp:3866), so the vehicle is fed
     * 0 V while a perfectly good span is overhead. Reported on the transition, like the other two,
     * because from the cab all three look the same. */
    void RailVehicle3D::_report_contact_gap(const int p_index, const bool p_is_active, const bool p_converged) {
        Dictionary cache = pantograph_wire_cache[p_index];
        const bool was_touching = cache.get("touching", false);
        if (p_is_active && was_touching && !p_converged) {
            UtilityFunctions::push_warning(vformat(
                    "Lost contact: %s pantograph %d is not reaching the wire - %s", get_name(), p_index,
                    _track_position_text()));
        }
        cache["touching"] = p_is_active && p_converged;
        pantograph_wire_cache[p_index] = cache;
    }

    /* FIXME(#184): this belongs in RailVehicleServer's step, not in the node that draws the
     * vehicle. Nothing here needs a node - the server already owns the placement and
     * vehicle_get_transform(rid) - and it decides what the simulation is fed, which is the one
     * thing a rendering layer must not do. Moving it needs the collector offsets below to reach
     * the vehicle first; the original keeps them in TAnimPant::vPos. */
    void RailVehicle3D::_update_pantograph_power(const Dictionary &p_state) {
        if (Engine::get_singleton()->is_editor_hint() || electric_engine == nullptr || controller == nullptr) {
            return;
        }
        const PantographFrame frame = _pantograph_frame();
        const Dictionary &state = p_state;
        const double assumed_voltage =
                MAX(Math::abs(double(state.get("current_collector/pantograph_first_voltage", 0.0))),
                    Math::abs(double(state.get("current_collector/pantograph_second_voltage", 0.0))));
        const bool front_active =
                bool(state.get("current_collector/pantograph_first_active", false)) && pantograph_front_converged;
        const bool rear_active =
                bool(state.get("current_collector/pantograph_second_active", false)) && pantograph_rear_converged;
        const int active_count = int(front_active) + int(rear_active);
        const double current = active_count > 0 ? double(state.get("current0", 0.0)) / active_count : 0.0;
        _report_contact_gap(
                2, bool(state.get("current_collector/pantograph_first_active", false)),
                pantograph_front_converged);
        _report_contact_gap(
                3, bool(state.get("current_collector/pantograph_second_active", false)),
                pantograph_rear_converged);
        const double front_voltage =
                front_active ? _pantograph_wire_voltage(2, pantograph_front_offset, frame, assumed_voltage, current)
                             : 0.0;
        electric_engine->set_pantograph_wire_voltage(VehicleElectricEngine::PANTOGRAPH_FIRST, front_voltage);
        electric_engine->set_pantograph_wire_voltage(
                VehicleElectricEngine::PANTOGRAPH_SECOND,
                rear_active ? _pantograph_wire_voltage(3, pantograph_rear_offset, frame, assumed_voltage, current)
                            : 0.0);
    }

    /// FIXME(#184): moves to RailVehicleServer with _update_pantograph_power().
    double RailVehicle3D::_pantograph_wire_voltage(
            const int p_index, const Vector3 &p_offset, const PantographFrame &p_frame, const double p_assumed_voltage,
            const double p_current) {
        const Vector3 contact_point = p_frame.transform.xform(p_offset);
        const Dictionary wire =
                _find_pantograph_wire(p_index, contact_point, p_frame.up, p_frame.forward, p_frame.left);
        const RID wire_rid = wire["rid"];
        if (!wire_rid.is_valid()) {
            return 0.0;
        }
        TractionPowerServer *traction_power_server = TractionPowerServer::get_instance();
        if (traction_power_server == nullptr) {
            return 0.0;
        }
        const double voltage = traction_power_server->wire_get_voltage(wire_rid, p_assumed_voltage, p_current);
        /* A span that is overhead but carries nothing is a different defect from a hole in the
         * wiring - it means the network behind it has no source, or the resistance never reached
         * it - and the two are indistinguishable from the cab, where both read as a dead line. */
        Dictionary cache = pantograph_wire_cache[p_index];
        const bool had_voltage = cache.get("powered", false);
        if (had_voltage && Math::is_zero_approx(voltage)) {
            UtilityFunctions::push_warning(vformat(
                    "Dead traction: %s has a wire under pantograph %d carrying no voltage - %s, %v",
                    get_name(), p_index, _track_position_text(), contact_point));
        }
        cache["powered"] = !Math::is_zero_approx(voltage);
        pantograph_wire_cache[p_index] = cache;
        return voltage;
    }

    /* FIXME(#184): the arm geometry is the vehicle's own state (TAnimPant, DynObj.h:106) and
     * belongs beside the Mover; only _apply_pantograph_animation() below is drawing. */
    void RailVehicle3D::_update_pantograph_raise_state(const double p_delta, const Dictionary &p_state) {
        if (Engine::get_singleton()->is_editor_hint() || controller == nullptr || electric_engine == nullptr) {
            return;
        }
        const Dictionary &state = p_state;
        pantograph_front_converged = _update_pantograph_arm(
                0, pantograph_front_geometry, pantograph_front_arm_nodes,
                state.get("current_collector/pantograph_first_active", false), p_delta, state);
        if (is_visible && !pantograph_front_geometry.is_empty()) {
            _apply_pantograph_animation(pantograph_front_arm_nodes, pantograph_front_geometry);
        }
        pantograph_rear_converged = _update_pantograph_arm(
                1, pantograph_rear_geometry, pantograph_rear_arm_nodes,
                state.get("current_collector/pantograph_second_active", false), p_delta, state);
        if (is_visible && !pantograph_rear_geometry.is_empty()) {
            _apply_pantograph_animation(pantograph_rear_arm_nodes, pantograph_rear_geometry);
        }
    }

    bool RailVehicle3D::_update_pantograph_arm(
            const int p_index, Dictionary p_geometry, const TypedArray<Node3D> &p_arm_nodes, const bool p_is_active,
            const double p_delta, const Dictionary &p_state) {
        if (p_geometry.is_empty()) {
            return true;
        }
        const Dictionary &state = p_state;
        const double pressure = state.get("current_collector/pantograph_tank_pressure", 0.0);
        const bool power_available =
                bool(state.get("power24_available", false)) || bool(state.get("power110_available", false));
        const bool is_ezt =
                (controller->get_train_type() & VehicleController::TRAIN_TYPE_EZT) == VehicleController::TRAIN_TYPE_EZT;
        const double pressure_threshold = is_ezt ? 2.45 : 3.45;
        double speed_factor = 0.0;
        if (pressure > pressure_threshold && power_available) {
            speed_factor = MAX(0.0, 0.015 * pressure * p_delta);
        }
        double pant_diff = Math_INF;
        if (p_is_active) {
            // a lowered pantograph comes down regardless of the wire (DynObj.cpp:3775), no search needed
            const PantographFrame frame = _pantograph_frame();
            Node3D *lower_arm = Object::cast_to<Node3D>(p_arm_nodes[0]);
            const Dictionary wire = _find_pantograph_wire(
                    p_index, lower_arm->get_global_position(), frame.up, frame.forward, frame.left);
            pant_diff = double(wire["height"]) - double(p_geometry["pant_wys"]);
        }
        double angle = p_geometry["angle_l"];
        if (speed_factor > 0.0 && p_is_active) {
            if (pant_diff > 0.001) {
                angle += MIN(speed_factor, 0.55 * pant_diff);
            } else if (pant_diff < -0.001) {
                angle += 0.4 * pant_diff;
            }
        } else {
            if (angle > double(p_geometry["angle_l0"])) {
                angle -= 0.15 * p_delta;
            }
            if (angle < double(p_geometry["angle_l0"])) {
                angle = p_geometry["angle_l0"];
            }
        }
        if (!Math::is_equal_approx(angle, double(p_geometry["angle_l"]))) {
            const double upper_angle = Math::acos(
                    ((double(p_geometry["len_l1"]) * Math::cos(angle)) + double(p_geometry["horiz"])) /
                    double(p_geometry["len_u1"]));
            if (angle + upper_angle < Math_PI) {
                p_geometry["angle_l"] = angle;
                p_geometry["angle_u"] = upper_angle;
                p_geometry["pant_wys"] = (double(p_geometry["len_l1"]) * Math::sin(angle)) +
                                         (double(p_geometry["len_u1"]) * Math::sin(upper_angle)) +
                                         double(p_geometry["height"]);
            }
        }
        return p_is_active && pant_diff < 0.01;
    }

    /* FIXME(#184): moves to RailVehicleServer with _update_pantograph_power(), and
     * pantograph_wire_cache - which span each pantograph is on - is the vehicle's state, not the
     * node's. */
    Dictionary RailVehicle3D::_find_pantograph_wire(
            int p_index, const Vector3 &p_contact_point, const Vector3 &p_up, const Vector3 &p_forward,
            const Vector3 &p_left) {
        TractionPowerServer *traction_power_server = TractionPowerServer::get_instance();
        if (traction_power_server == nullptr) {
            /* "No wire in reach", the same answer the search gives when it finds none - the raise
             * simulation reads this height and an absent key would read as 0.0, which is
             * "the wire is right here" and folds the pantograph instead of extending it. */
            Dictionary missing;
            missing["rid"] = RID();
            missing["height"] = INFINITY;
            return missing;
        }
        Dictionary cache = pantograph_wire_cache[p_index];
        /* The wire found last frame is kept and followed: running off the end of a span is not a
         * loss of contact, the neighbouring span is reached along the chain in the same frame
         * (DynObj.cpp:8742-8770). Its height is recomputed every frame - a cached height made the
         * wire step up and down while driving and dropped the contact with it. */
        const RID wire_rid = cache.get("rid", RID());
        if (wire_rid.is_valid()) {
            const Dictionary followed = traction_power_server->wire_follow_above(
                    wire_rid, p_contact_point, p_up, p_forward, p_left, pantograph_slider_half_width,
                    PANTOGRAPH_HORN_WIDTH);
            if (RID(followed["rid"]).is_valid()) {
                cache["rid"] = followed["rid"];
                pantograph_wire_cache[p_index] = cache;
                return followed;
            }
        }
        // the chain ran out, so search the region like update_traction() does (DynObj.cpp:8799)
        const Dictionary result = traction_power_server->wire_find_above_with_height(
                p_contact_point, p_up, p_forward, p_left, pantograph_slider_half_width, PANTOGRAPH_HORN_WIDTH);
        /* A pantograph that had a wire and now has none is what the vehicle reads as a loss of
         * line voltage, and it trips the main switch. The original reports the same class of
         * event with the place it happened (scene.cpp:112, "Bad traction"), which is the only way
         * to tell a hole in the scenery's wiring from a defect in this search. */
        if (wire_rid.is_valid() && !RID(result["rid"]).is_valid()) {
            UtilityFunctions::push_warning(vformat(
                    "Bad traction: %s lost the wire under pantograph %d - %s, %v", get_name(), p_index,
                    _track_position_text(), p_contact_point));
        }
        cache["rid"] = result["rid"];
        pantograph_wire_cache[p_index] = cache;
        return result;
    }

    void RailVehicle3D::_apply_pantograph_animation(const TypedArray<Node3D> &p_nodes, const Dictionary &p_geometry) {
        const double a_deg = Math::rad_to_deg(double(p_geometry["angle_l"]) - double(p_geometry["angle_l0"]));
        const double b_deg = Math::rad_to_deg(double(p_geometry["angle_u"]) - double(p_geometry["angle_u0"]));
        const double c_deg = a_deg + b_deg;
        TypedArray<Node3D> one_node;
        one_node.append(p_nodes[0]);
        _apply_wheel_rotation(one_node, -a_deg);
        one_node[0] = p_nodes[1];
        _apply_wheel_rotation(one_node, a_deg);
        one_node[0] = p_nodes[2];
        _apply_wheel_rotation(one_node, c_deg);
        one_node[0] = p_nodes[3];
        _apply_wheel_rotation(one_node, -c_deg);
        one_node[0] = p_nodes[4];
        _apply_wheel_rotation(one_node, -b_deg);
    }

#define DEFINE_PATH_PROPERTY(name, dirty_flag)                                                                         \
    void RailVehicle3D::set_##name(const NodePath &p_value) {                                                          \
        if (name != p_value) {                                                                                         \
            name = p_value;                                                                                            \
            dirty_flag = true;                                                                                         \
        }                                                                                                              \
    }                                                                                                                  \
    NodePath RailVehicle3D::get_##name() const {                                                                       \
        return name;                                                                                                   \
    }
#define DEFINE_ARRAY_PROPERTY(name)                                                                                    \
    void RailVehicle3D::set_##name(const TypedArray<NodePath> &p_value) {                                              \
        if (name != p_value) {                                                                                         \
            name = p_value;                                                                                            \
            animation_bindings_dirty = true;                                                                           \
        }                                                                                                              \
    }                                                                                                                  \
    TypedArray<NodePath> RailVehicle3D::get_##name() const {                                                           \
        return name;                                                                                                   \
    }

    DEFINE_PATH_PROPERTY(model_instance_path, dirty)
    DEFINE_PATH_PROPERTY(controller_path, dirty)
    DEFINE_PATH_PROPERTY(front_bogie_path, animation_bindings_dirty)
    DEFINE_PATH_PROPERTY(rear_bogie_path, animation_bindings_dirty)
    DEFINE_ARRAY_PROPERTY(front_rolling_wheel_paths)
    DEFINE_ARRAY_PROPERTY(powered_wheel_paths)
    DEFINE_ARRAY_PROPERTY(rear_rolling_wheel_paths)
    DEFINE_ARRAY_PROPERTY(pantograph_front_arm_paths)
    DEFINE_ARRAY_PROPERTY(pantograph_rear_arm_paths)
    DEFINE_ARRAY_PROPERTY(wiper_arm_paths)

#undef DEFINE_ARRAY_PROPERTY

    void RailVehicle3D::set_coupler_submodel_paths(const Dictionary &p_value) {
        if (coupler_submodel_paths != p_value) {
            coupler_submodel_paths = p_value;
            animation_bindings_dirty = true;
        }
    }

    Dictionary RailVehicle3D::get_coupler_submodel_paths() const {
        return coupler_submodel_paths;
    }
#undef DEFINE_PATH_PROPERTY

    void RailVehicle3D::set_lights(const TypedDictionary<String, bool> &p_value) {
        if (lights != p_value) {
            lights = p_value;
            _sync_model_lights();
        }
    }
    TypedDictionary<String, bool> RailVehicle3D::get_lights() const {
        return lights;
    }
    void RailVehicle3D::set_pantograph_front_offset(const Vector3 &p_value) {
        pantograph_front_offset = p_value;
    }
    Vector3 RailVehicle3D::get_pantograph_front_offset() const {
        return pantograph_front_offset;
    }
    void RailVehicle3D::set_pantograph_rear_offset(const Vector3 &p_value) {
        pantograph_rear_offset = p_value;
    }
    Vector3 RailVehicle3D::get_pantograph_rear_offset() const {
        return pantograph_rear_offset;
    }
    void RailVehicle3D::set_pantograph_collector_width(double p_value) {
        pantograph_collector_width = p_value;
    }
    double RailVehicle3D::get_pantograph_collector_width() const {
        return pantograph_collector_width;
    }
    void RailVehicle3D::set_start_track_name(const String &p_value) {
        if (start_track_name != p_value) {
            start_track_name = p_value;
            pending_start_track_retry = !start_track_name.is_empty();
            dirty = true;
        }
    }
    String RailVehicle3D::get_start_track_name() const {
        return start_track_name;
    }
    void RailVehicle3D::set_start_track_offset(double p_value) {
        if (!Math::is_equal_approx(start_track_offset, p_value)) {
            start_track_offset = p_value;
            pending_start_track_retry = !start_track_name.is_empty();
            dirty = true;
        }
    }
    double RailVehicle3D::get_start_track_offset() const {
        return start_track_offset;
    }
    void RailVehicle3D::set_start_direction(int p_value) {
        if (start_direction != p_value) {
            start_direction = p_value;
            pending_start_track_retry = !start_track_name.is_empty();
            dirty = true;
        }
    }
    int RailVehicle3D::get_start_direction() const {
        return start_direction;
    }
    void RailVehicle3D::set_cabin_scene(const Ref<PackedScene> &p_value) {
        cabin_scene = p_value;
    }
    Ref<PackedScene> RailVehicle3D::get_cabin_scene() const {
        return cabin_scene;
    }
    void RailVehicle3D::set_cabin_rotate_180deg(bool p_value) {
        cabin_rotate_180deg = p_value;
    }
    bool RailVehicle3D::get_cabin_rotate_180deg() const {
        return cabin_rotate_180deg;
    }
    void RailVehicle3D::set_joint_cabs(bool p_value) {
        joint_cabs = p_value;
    }
    bool RailVehicle3D::get_joint_cabs() const {
        return joint_cabs;
    }
    void RailVehicle3D::set_load_model_path(const NodePath &p_value) {
        if (load_model_path != p_value) {
            load_model_path = p_value;
            dirty = true;
        }
    }
    NodePath RailVehicle3D::get_load_model_path() const {
        return load_model_path;
    }
    void RailVehicle3D::set_low_poly_cabin_path(const NodePath &p_value) {
        if (low_poly_cabin_path != p_value) {
            low_poly_cabin_path = p_value;
            dirty = true;
        }
    }
    NodePath RailVehicle3D::get_low_poly_cabin_path() const {
        return low_poly_cabin_path;
    }
    void RailVehicle3D::set_low_poly_cabin_emission_energy(double p_value) {
        low_poly_cabin_emission_energy = p_value;
    }
    double RailVehicle3D::get_low_poly_cabin_emission_energy() const {
        return low_poly_cabin_emission_energy;
    }
    void RailVehicle3D::set_low_poly_cabin_emission_fade_time(double p_value) {
        low_poly_cabin_emission_fade_time = p_value;
    }
    double RailVehicle3D::get_low_poly_cabin_emission_fade_time() const {
        return low_poly_cabin_emission_fade_time;
    }
    void RailVehicle3D::set_head_display_e3d_path(const NodePath &p_value) {
        if (head_display_e3d_path != p_value) {
            head_display_e3d_path = p_value;
            head_display_e3d = nullptr;
            dirty = true;
        }
    }
    NodePath RailVehicle3D::get_head_display_e3d_path() const {
        return head_display_e3d_path;
    }
    void RailVehicle3D::set_head_display_material(const Ref<Material> &p_value) {
        if (head_display_material != p_value) {
            head_display_material = p_value;
            needs_head_display_update = true;
        }
    }
    Ref<Material> RailVehicle3D::get_head_display_material() const {
        return head_display_material;
    }
    void RailVehicle3D::set_head_display_node_path(const NodePath &p_value) {
        if (head_display_node_path != p_value) {
            head_display_node_path = p_value;
            needs_head_display_update = true;
        }
    }
    NodePath RailVehicle3D::get_head_display_node_path() const {
        return head_display_node_path;
    }
} // namespace godot
