#include "RailVehicle3D.hpp"
#include "RailVehicleController.hpp"
#include "cabin/Cabin3D.hpp"
#include "scenery/SceneryHUDMouseServer.hpp"
#include "scenery/SceneryStreamingServer.hpp"
#include "vehicles/base/VehiclePhysicsNode.hpp"
#include "vehicles/rail/RailVehicleBuffCoupl.hpp"
#include "vehicles/rail/RailVehicleLoad.hpp"
#include "vehicles/rail/RailVehicleWheels.hpp"

#include "logging/GameLog.hpp"
#include "tracks/TrackServer.hpp"
#include "vehicles/base/VehicleServer.hpp"
#include "vehicles/rail/RailVehicleDieselEngine.hpp"
#include "vehicles/rail/RailVehicleDoors.hpp"
#include "vehicles/rail/RailVehicleElectricEngine.hpp"
#include "vehicles/rail/RailVehicleLighting.hpp"
#include "vehicles/rail/RailVehicleServer.hpp"
#include "vehicles/rail/RailVehicleWipers.hpp"

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
    template<typename T>
    Ref<T> RailVehicle3D::_component(const VehicleComponentType::Type p_type) const {
        const VehicleServer *server = VehicleServer::get_instance();
        return server != nullptr && rid.is_valid() ? Ref<T>(server->vehicle_component_get(rid, p_type)) : Ref<T>();
    }

    const char *RailVehicle3D::controller_changed_signal = "controller_changed";

    namespace {
        /* Which submodel of the model shows which of the vehicle's lamps. The lamp is read off
         * the lighting component, so a renamed or removed one is a build error rather than a
         * light that silently stops working. */
        /// Whether the headlights' dimming reaches a light: the headlamps only (DynObj.cpp:1212-1320)
        enum LightDimming { LIGHT_DIMMING_NONE, LIGHT_DIMMING_HEADLIGHT };

        struct LightStateBinding {
                const char *light_name;
                bool (RailVehicleLighting::*is_enabled)() const;
                LightDimming dimming;
        };

        constexpr std::array<LightStateBinding, 10> LIGHT_STATE_BINDINGS = {{
                {"headlamp11", &RailVehicleLighting::get_front_headlight_upper_enabled, LIGHT_DIMMING_HEADLIGHT},
                {"headlamp12", &RailVehicleLighting::get_front_headlight_right_enabled, LIGHT_DIMMING_HEADLIGHT},
                {"headlamp13", &RailVehicleLighting::get_front_headlight_left_enabled, LIGHT_DIMMING_HEADLIGHT},
                {"headlamp21", &RailVehicleLighting::get_rear_headlight_upper_enabled, LIGHT_DIMMING_HEADLIGHT},
                {"headlamp22", &RailVehicleLighting::get_rear_headlight_right_enabled, LIGHT_DIMMING_HEADLIGHT},
                {"headlamp23", &RailVehicleLighting::get_rear_headlight_left_enabled, LIGHT_DIMMING_HEADLIGHT},
                {"endsignal12", &RailVehicleLighting::get_front_redmarker_right_enabled, LIGHT_DIMMING_NONE},
                {"endsignal13", &RailVehicleLighting::get_front_redmarker_left_enabled, LIGHT_DIMMING_NONE},
                {"endsignal22", &RailVehicleLighting::get_rear_redmarker_right_enabled, LIGHT_DIMMING_NONE},
                {"endsignal23", &RailVehicleLighting::get_rear_redmarker_left_enabled, LIGHT_DIMMING_NONE},
        }};
    } // namespace

    template<typename T>
    static T *node_at(Node *p_owner, const NodePath &p_path) {
        return Object::cast_to<T>(p_owner->get_node_or_null(p_path));
    }

    RailVehicle3D::RailVehicle3D() {
        set_process(true);
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
        BIND_RAIL_PROPERTY(pantograph_collector_width, Variant::FLOAT);
        BIND_RAIL_NODE_PATH_ARRAY(pantograph_front_arm_paths);
        BIND_RAIL_NODE_PATH_ARRAY(pantograph_rear_arm_paths);
        BIND_RAIL_NODE_PATH_ARRAY(wiper_arm_paths);
        BIND_RAIL_NODE_PATH_ARRAY(mirror_paths);
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

        ClassDB::bind_method(D_METHOD("show_cabin"), &RailVehicle3D::show_cabin);
        ClassDB::bind_method(D_METHOD("hide_cabin"), &RailVehicle3D::hide_cabin);
        ClassDB::bind_method(D_METHOD("get_cabin"), &RailVehicle3D::get_cabin);
        ClassDB::bind_method(D_METHOD("get_controller"), &RailVehicle3D::get_controller);
        ClassDB::bind_method(D_METHOD("get_rid"), &RailVehicle3D::get_rid);
        ClassDB::bind_method(D_METHOD("move_on_track", "distance"), &RailVehicle3D::move_on_track);
        ClassDB::bind_method(D_METHOD("_process", "delta"), &RailVehicle3D::_process_impl);
        ClassDB::bind_method(D_METHOD("_process_dirty"), &RailVehicle3D::_process_dirty);
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
                D_METHOD("_on_track_server_tracks_changed"), &RailVehicle3D::_on_track_server_tracks_changed);

#undef BIND_RAIL_NODE_PATH_ARRAY
#undef BIND_RAIL_NODE_PATH
#undef BIND_RAIL_PROPERTY
        /* The vehicle this node renders has changed - it has one now, it has a different one, or
         * it has none. Whoever needs the vehicle reacts to this instead of looking for it again
         * later: a consumer wired up before the vehicle is built would otherwise have to poll. */
        ADD_SIGNAL(MethodInfo(controller_changed_signal));
    }

    /// The cab interior, built from cabin_scene into the vehicle - only a view: the cab logic is
    /// the vehicle's (CabinSystem). A cab is built within add_child() (Cabin3D's cabin_ready comes
    /// from its NOTIFICATION_READY), so get_cabin() returns it built once this returns.
    void RailVehicle3D::show_cabin() {
        Cabin3D *cabin = _object<Cabin3D>(cabin_id);
        if (cabin != nullptr) {
            return;
        }
        if (cabin_scene.is_null()) {
            UtilityFunctions::push_warning(get_name(), " has no cabin_scene; cabin entry not yet supported");
            return;
        }
        Cabin3D *new_cabin = Object::cast_to<Cabin3D>(cabin_scene->instantiate());
        if (new_cabin == nullptr) {
            UtilityFunctions::push_error("Root node of cabin scene must be a Cabin3D");
            return;
        }
        cabin = new_cabin;
        cabin_id = _id_of(cabin);
        cabin->connect(
                Cabin3D::camera_configuration_changed_signal,
                callable_mp(this, &RailVehicle3D::_update_low_poly_cabs_visibility));
        cabin->set_transform(Transform3D());
        if (cabin_rotate_180deg) {
            cabin->rotate_y(static_cast<real_t>(Math::deg_to_rad(180.0)));
        }
        add_child(cabin);
        /* A cabin holds the handle of the vehicle it sits in and takes everything else from
         * CabinSystem. Told once it is in the tree, because building its interior puts nodes
         * there, and told here rather than at the next controller change, which for an existing
         * vehicle never comes. */
        cabin->set_vehicle_rid(rid);
        _update_low_poly_cabs_visibility();
    }

    /// The cab interior freed; a camera put into it is to be taken out first
    void RailVehicle3D::hide_cabin() {
        Cabin3D *cabin = _object<Cabin3D>(cabin_id);
        if (cabin == nullptr) {
            return;
        }
        cabin->disconnect(
                Cabin3D::camera_configuration_changed_signal,
                callable_mp(this, &RailVehicle3D::_update_low_poly_cabs_visibility));
        cabin->get_parent()->remove_child(cabin);
        cabin->queue_free();
        cabin_id = ObjectID();
        _update_low_poly_cabs_visibility();
    }

    Cabin3D *RailVehicle3D::get_cabin() const {
        Cabin3D *cabin = _object<Cabin3D>(cabin_id);
        return cabin;
    }

    RailVehicleController *RailVehicle3D::_resolve_controller(const NodePath &p_node_path) const {
        // a vehicle in the tree is a VehiclePhysicsNode; the controller is the object it owns - a
        // railway one, as this node draws a rail vehicle
        VehiclePhysicsNode *physics = Object::cast_to<VehiclePhysicsNode>(get_node_or_null(p_node_path));
        return physics == nullptr ? nullptr : Object::cast_to<RailVehicleController>(physics->get_controller().ptr());
    }

    RID RailVehicle3D::get_rid() const {
        return rid;
    }

    Ref<RailVehicleController> RailVehicle3D::get_controller() const {
        return Ref<RailVehicleController>(controller_path.is_empty() ? nullptr : _resolve_controller(controller_path));
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
        Node3D *load_model = _object<Node3D>(load_model_id);
        const RailVehicleController *vehicle = _object<RailVehicleController>(controller_id);
        const Ref<RailVehicleLoad> load = _component<RailVehicleLoad>(VehicleComponentType::COMPONENT_LOAD);
        if (load_model == nullptr || vehicle == nullptr || load.is_null()) {
            return;
        }
        const TypedArray<String> accepted = load->get_accepted_loads();
        const TypedArray<float> offsets = load->get_minimum_load_offsets();
        const String cargo = vehicle->get_load_name().to_lower();
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
        const double fill = max_load > 0.0 ? CLAMP(vehicle->get_load_amount() / max_load, 0.0, 1.0) : 0.0;
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
        VehiclePhysicsNode *fiz_controller = _object<VehiclePhysicsNode>(fiz_controller_id);
        RailVehicleController *vehicle =
                fiz_controller != nullptr
                        ? Object::cast_to<RailVehicleController>(fiz_controller->get_controller().ptr())
                        : nullptr;
        _on_controller_changed(vehicle);
        dirty = true;
        set_process(true);
    }

    /* Resolving the vehicle node and subscribing to it. Done on entering the tree, before any
     * processing: with the subscription made in _process() instead, a node that waits for its
     * vehicle would never hear about it. */
    void RailVehicle3D::_bind_vehicle_node() {
        VehiclePhysicsNode *fiz_controller = _object<VehiclePhysicsNode>(fiz_controller_id);
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
            fiz_controller_id = _id_of(fiz_controller);
            if (fiz_controller != nullptr) {
                fiz_controller->connect(
                        VehiclePhysicsNode::vehicle_changed_signal,
                        callable_mp(this, &RailVehicle3D::_on_vehicle_changed));
            }
        }
        _on_controller_changed(get_controller().ptr());
    }

    void RailVehicle3D::_on_controller_changed(RailVehicleController *p_controller) {
        Cabin3D *cabin = _object<Cabin3D>(cabin_id);
        Node3D *model_node = _object<Node3D>(model_node_id);
        RailVehicleController *controller = _object<RailVehicleController>(controller_id);
        if (controller == p_controller) {
            return;
        }
        if (controller != nullptr) {
            controller->disconnect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
            controller->disconnect(
                    VehicleController::config_changed, callable_mp(this, &RailVehicle3D::_on_vehicle_config_changed));
        }
        controller = p_controller;
        controller_id = _id_of(controller);
        if (controller != nullptr) {
            controller->connect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
            controller->connect(
                    VehicleController::config_changed, callable_mp(this, &RailVehicle3D::_on_vehicle_config_changed));
        }
        VehicleServer *vehicle_server = VehicleServer::get_instance();
        if (RailVehicleServer *server = RailVehicleServer::get_instance();
            server != nullptr && vehicle_server != nullptr) {
            /* A vehicle has one handle. When the controller already carries one - it does
             * whenever a VehiclePhysicsNode built it - this node renders that vehicle rather than
             * creating a second one, which would step the same controller twice and place only
             * one of the two on a track. */
            const RID vehicle_rid = controller != nullptr ? controller->get_rid() : RID();
            if (vehicle_rid.is_valid() && vehicle_rid != rid) {
                if (rid_owned && rid.is_valid()) {
                    vehicle_server->vehicle_free(rid);
                }
                rid = vehicle_rid;
                rid_owned = false;
                server->vehicle_attach(rid);
                server->vehicle_attach_rail_vehicle(rid, get_instance_id());
                // the pantographs belong to the handle: the model's arms go with it to the new one
                _publish_pantograph_geometry(RailVehicleElectricEngine::PANTOGRAPH_FIRST, pantograph_front_arm_nodes);
                _publish_pantograph_geometry(RailVehicleElectricEngine::PANTOGRAPH_SECOND, pantograph_rear_arm_nodes);
                if (model_node != nullptr) {
                    // the model node is a GDScript E3DModelInstance, unknown at build time
                    _register_pickable(model_node->call("get_e3d_instance"));
                }
            }
            if (rid.is_valid()) {
                vehicle_server->vehicle_attach_controller(
                        rid, controller != nullptr ? controller->get_instance_id() : 0);
            }
        }
        if (cabin != nullptr) {
            cabin->set_vehicle_rid(rid);
        }
        const Ref<RailVehicleLighting> lighting =
                _component<RailVehicleLighting>(VehicleComponentType::COMPONENT_LIGHTING);
        _on_roof_light_changed(lighting.is_valid() && lighting->get_roof_light_enabled());
        emit_signal(controller_changed_signal);
    }

    void RailVehicle3D::_enter_tree() {
        if (TrackServer *tracks = TrackServer::get_instance(); tracks != nullptr) {
            tracks->connect(
                    TrackServer::tracks_changed_signal,
                    callable_mp(this, &RailVehicle3D::_on_track_server_tracks_changed));
        }
        VehicleServer *vehicle_server = VehicleServer::get_instance();
        if (RailVehicleServer *server = RailVehicleServer::get_instance();
            server != nullptr && vehicle_server != nullptr) {
            rid = vehicle_server->vehicle_create();
            rid_owned = true;
            server->vehicle_attach(rid);
            server->vehicle_attach_rail_vehicle(rid, get_instance_id());
        }
        pending_start_track_retry = !start_track_name.is_empty();
        dirty = true;
        _bind_vehicle_node();
        /* A node pointed at a vehicle does nothing until that vehicle exists - it would only
         * place and animate itself against a vehicle that is not there yet. One without a
         * vehicle of its own has nothing to wait for. */
        set_process(controller_path.is_empty() || get_controller().is_valid());
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
        VehiclePhysicsNode *fiz_controller = _object<VehiclePhysicsNode>(fiz_controller_id);
        Node3D *model_node = _object<Node3D>(model_node_id);
        if (TrackServer *tracks = TrackServer::get_instance(); tracks != nullptr) {
            tracks->disconnect(
                    TrackServer::tracks_changed_signal,
                    callable_mp(this, &RailVehicle3D::_on_track_server_tracks_changed));
        }
        if (model_node != nullptr) {
            model_node->disconnect("e3d_loading", Callable(this, "_on_model_node_e3d_loading"));
            model_node->disconnect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
            model_node->disconnect("e3d_instance_created", callable_mp(this, &RailVehicle3D::_register_pickable));
            model_node_id = ObjectID();
        }
        _register_pickable(RID());
        // only the handle this node created is this node's to free; an adopted one belongs to
        // the VehiclePhysicsNode that built the vehicle
        if (rid_owned && rid.is_valid()) {
            if (VehicleServer *server = VehicleServer::get_instance(); server != nullptr) {
                server->vehicle_free(rid);
            }
        }
        rid = RID();
        rid_owned = false;
        if (fiz_controller != nullptr) {
            fiz_controller->disconnect(
                    VehiclePhysicsNode::vehicle_changed_signal, callable_mp(this, &RailVehicle3D::_on_vehicle_changed));
            fiz_controller_id = ObjectID();
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
                // the placement itself is applied by RailVehicleServer at the end of its step, and
                // the pantographs raised there
                if (is_visible) {
                    _update_pantograph_animation();
                }
            } else if (rid.is_valid() && start_track_name.is_empty()) {
                const VehicleServer *server = VehicleServer::get_instance();
                const double velocity = server != nullptr ? server->vehicle_get_velocity(rid) : 0.0;
                const real_t distance = static_cast<real_t>(p_delta * velocity);
                set_position(get_position() + (Vector3(0.0, 0.0, -1.0) * distance));
                if (is_visible && !Math::is_zero_approx(velocity)) {
                    _update_wheel_animation_state();
                }
                if (is_visible) {
                    _update_pantograph_animation();
                }
            }
            if (rid.is_valid()) {
                _sync_lights_from_controller();
                if (is_visible) {
                    _update_couplers();
                    _update_wipers();
                    _update_mirrors();
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
        Node3D *load_model = _object<Node3D>(load_model_id);
        Node *head_display_e3d = _object<Node>(head_display_e3d_id);
        Node3D *model_node = _object<Node3D>(model_node_id);
        Node3D *low_poly_cabin = _object<Node3D>(low_poly_cabin_id);
        dirty = false;
        if (!head_display_e3d_path.is_empty()) {
            head_display_e3d = get_node_or_null(head_display_e3d_path);
            head_display_e3d_id = _id_of(head_display_e3d);
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
            model_node->disconnect("e3d_instance_created", callable_mp(this, &RailVehicle3D::_register_pickable));
        }
        model_node = new_model_node;
        model_node_id = _id_of(model_node);
        if (model_node != nullptr) {
            model_node->connect("e3d_loading", Callable(this, "_on_model_node_e3d_loading"));
            model_node->connect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
            model_node->connect("e3d_instance_created", callable_mp(this, &RailVehicle3D::_register_pickable));
            // the model node is a GDScript E3DModelInstance, unknown at build time
            _register_pickable(model_node->call("get_e3d_instance"));
        }
        _sync_model_lights();
        _update_detection_area();
        _update_smoke();

        if (low_poly_cabin != nullptr) {
            low_poly_cabin->disconnect("e3d_loaded", Callable(this, "_on_low_poly_cabin_e3d_loaded"));
        }
        load_model = load_model_path.is_empty() ? nullptr : node_at<Node3D>(this, load_model_path);
        load_model_id = _id_of(load_model);
        _apply_load_offset();
        low_poly_cabin = low_poly_cabin_path.is_empty() ? nullptr : node_at<Node3D>(this, low_poly_cabin_path);
        low_poly_cabin_id = _id_of(low_poly_cabin);
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
        Node3D *model_node = _object<Node3D>(model_node_id);
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
        Node3D *model_node = _object<Node3D>(model_node_id);
        if (model_node == nullptr || !bool(model_node->call("is_e3d_loaded"))) {
            return;
        }
        const Ref<RailVehicleLighting> lighting =
                _component<RailVehicleLighting>(VehicleComponentType::COMPONENT_LIGHTING);
        if (lighting.is_null()) {
            return;
        }
        bool changed = false;
        const Array keys = lights.keys();
        for (int index = 0; index < keys.size(); ++index) {
            const Variant &light_name = keys[index];
            const String light_name_string = light_name;
            for (const LightStateBinding &binding: LIGHT_STATE_BINDINGS) {
                if (light_name_string == binding.light_name) {
                    const bool new_value = (lighting.ptr()->*binding.is_enabled)();
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
        // Headlights dimmed (DynamicObject->DimHeadlights): E3DModelInstance is a GDScript node,
        // hence the property by name, as for lights_state above
        if (const bool dimmed = lighting->get_headlights_dimmed(); dimmed != headlights_dimmed) {
            headlights_dimmed = dimmed;
            Dictionary lights_dimmed;
            for (const LightStateBinding &binding: LIGHT_STATE_BINDINGS) {
                if (binding.dimming == LIGHT_DIMMING_HEADLIGHT) {
                    lights_dimmed[binding.light_name] = dimmed;
                }
            }
            model_node->set("lights_dimmed_multiplier", lighting->get_head_light_dimmed_multiplier());
            model_node->set("lights_dimmed", lights_dimmed);
        }
    }

    /* The model is about to free and rebuild its children (a reload, or leaving the tree), so
     * every node cached out of it goes, and is taken again when the model has loaded. */
    void RailVehicle3D::_on_model_node_e3d_loading() {
        front_bogie_node_id = ObjectID();
        rear_bogie_node_id = ObjectID();
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
        _update_smoke();
    }

    /// The model is clicked in free camera (SceneryHUDMouseServer) while it is detailed: a new
    /// instance comes with every detail switch, and a far vehicle is left out of picking
    void RailVehicle3D::_register_pickable(const RID &p_instance) {
        SceneryHUDMouseServer *mouse = SceneryHUDMouseServer::get_instance();
        if (Engine::get_singleton()->is_editor_hint() || mouse == nullptr) {
            return;
        }
        mouse->pickable_free(pickable);
        pickable = RID();
        const VehicleServer *server = VehicleServer::get_instance();
        if (!model_detailed || !p_instance.is_valid() || !rid.is_valid() || server == nullptr) {
            return;
        }
        pickable = mouse->vehicle_pickable_create(p_instance, server->vehicle_get_name(rid), rid);
    }

    void RailVehicle3D::_on_low_poly_cabin_e3d_loaded() {
        Node3D *low_poly_cabin = _object<Node3D>(low_poly_cabin_id);
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
        const Ref<RailVehicleLighting> lighting =
                _component<RailVehicleLighting>(VehicleComponentType::COMPONENT_LIGHTING);
        const bool roof_light_enabled = lighting.is_valid() && lighting->get_roof_light_enabled();
        _set_low_poly_emission_energy(roof_light_enabled ? low_poly_cabin_emission_energy : 0.0);
        _update_low_poly_cabs_visibility();
    }

    // Original engine: the low-poly interior stays rendered from inside the cab (Render_interior(),
    // opengl33renderer.cpp:1123); only its occupied "cabN" submodel is hidden - or all of them
    // with jointcabs: - so the hi-fi cab doesn't overlap it (DynObj.cpp:1211-1219, 2236-2250).
    // A cab without a hi-fi model keeps every low-poly cab visible (DynObj.cpp:1214).
    void RailVehicle3D::_update_low_poly_cabs_visibility() {
        Cabin3D *cabin = _object<Cabin3D>(cabin_id);
        Node3D *low_poly_cabin = _object<Node3D>(low_poly_cabin_id);
        if (low_poly_cabin == nullptr) {
            return;
        }
        const int cab_number = cabin == nullptr ? 0 : cabin->get_cab_number();
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
        Node3D *model_node = _object<Node3D>(model_node_id);
        Area3D *detection_area = _object<Area3D>(detection_area_id);
        VisibleOnScreenNotifier3D *visibility_notifier = _object<VisibleOnScreenNotifier3D>(visibility_notifier_id);
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
            detection_area_id = _id_of(detection_area);
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
            visibility_notifier_id = _id_of(visibility_notifier);
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

    void RailVehicle3D::_on_track_server_tracks_changed() {
        if (pending_start_track_retry) {
            _apply_start_track();
        }
    }

    void RailVehicle3D::_apply_start_track() {
        if (start_track_name.is_empty()) {
            return;
        }
        TrackServer *tracks = TrackServer::get_instance();
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
                rid, track_rid, start_track_offset, static_cast<TrackServer::Direction>(start_direction));
        apply_track_placement();
    }

    Vector<ObjectID> RailVehicle3D::_resolve_animation_nodes(const TypedArray<NodePath> &p_paths) const {
        Vector<ObjectID> nodes;
        for (int index = 0; index < p_paths.size(); ++index) {
            const NodePath path = p_paths[index];
            if (!path.is_empty()) {
                Node3D *node = node_at<Node3D>(const_cast<RailVehicle3D *>(this), path);
                if (node != nullptr) {
                    nodes.push_back(_id_of(node));
                }
            }
        }
        return nodes;
    }

    void RailVehicle3D::_capture_rest_basis(Node3D *p_node) {
        if (p_node != nullptr && !node_rest_bases.has(_id_of(p_node))) {
            node_rest_bases[_id_of(p_node)] = p_node->get_transform().basis.orthonormalized();
        }
    }

    Vector<ObjectID> RailVehicle3D::_resolve_pantograph_arm_nodes(const TypedArray<NodePath> &p_paths) const {
        Vector<ObjectID> nodes;
        if (p_paths.size() != 5) {
            return nodes;
        }
        for (int index = 0; index < p_paths.size(); ++index) {
            const NodePath path = p_paths[index];
            nodes.push_back(
                    path.is_empty() ? ObjectID() : _id_of(node_at<Node3D>(const_cast<RailVehicle3D *>(this), path)));
        }
        return nodes;
    }

    void RailVehicle3D::_publish_pantograph_geometry(
            const RailVehicleElectricEngine::PantographSelector p_pantograph, const Vector<ObjectID> &p_nodes) const {
        RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr || !rid.is_valid() || p_nodes.size() != 5) {
            return;
        }
        // the second arm of each pair is optional - a single-arm pantograph has none
        const Node3D *lower = _object<Node3D>(p_nodes[0]);
        const Node3D *upper = _object<Node3D>(p_nodes[2]);
        const Node3D *slider = _object<Node3D>(p_nodes[4]);
        if (lower == nullptr || upper == nullptr || slider == nullptr) {
            return;
        }
        const Vector3 lower_to_upper =
                lower->get_global_basis().inverse().xform(upper->get_global_position() - lower->get_global_position());
        const Vector3 upper_to_slider =
                upper->get_global_basis().inverse().xform(slider->get_global_position() - upper->get_global_position());
        const double lower_length = Vector2(lower_to_upper.y, lower_to_upper.z).length();
        const double upper_length = Vector2(upper_to_slider.y, upper_to_slider.z).length();
        if (lower_length <= 0.0 || upper_length <= 0.0) {
            return;
        }
        /* Where this pantograph sits on the vehicle, in the vehicle's own space - the original
         * reads the same thing off the submodel's matrix (TAnimPant::vPos, DynObj.cpp:5508-5549:
         * sideways, up, and along the length). Without it both pantographs of a vehicle sampled
         * the wire at the same point, the vehicle's origin. */
        server->vehicle_set_pantograph_geometry(
                rid, p_pantograph, get_global_transform().affine_inverse().xform(lower->get_global_position()),
                lower_length, upper_length, Math::abs(upper_to_slider.y) - Math::abs(lower_to_upper.y),
                Math::atan2(Math::abs(lower_to_upper.z), Math::abs(lower_to_upper.y)),
                Math::atan2(Math::abs(upper_to_slider.z), Math::abs(upper_to_slider.y)), pantograph_collector_width);
    }

    void RailVehicle3D::_cache_animation_bindings() {
        Node3D *front_bogie_node = node_at<Node3D>(this, front_bogie_path);
        front_bogie_node_id = _id_of(front_bogie_node);
        Node3D *rear_bogie_node = node_at<Node3D>(this, rear_bogie_path);
        rear_bogie_node_id = _id_of(rear_bogie_node);
        front_rolling_wheel_nodes = _resolve_animation_nodes(front_rolling_wheel_paths);
        powered_wheel_nodes = _resolve_animation_nodes(powered_wheel_paths);
        rear_rolling_wheel_nodes = _resolve_animation_nodes(rear_rolling_wheel_paths);
        node_rest_bases.clear();
        bogie_rest_global_bases.clear();

        Node3D *bogies[] = {front_bogie_node, rear_bogie_node};
        for (Node3D *bogie: bogies) {
            if (bogie != nullptr) {
                _capture_rest_basis(bogie);
                bogie_rest_global_bases[_id_of(bogie)] = get_global_basis().inverse() * bogie->get_global_basis();
            }
        }
        for (const Vector<ObjectID> *wheel_nodes:
             {&front_rolling_wheel_nodes, &powered_wheel_nodes, &rear_rolling_wheel_nodes}) {
            for (const ObjectID &wheel: *wheel_nodes) {
                _capture_rest_basis(_object<Node3D>(wheel));
            }
        }

        coupler_submodel_nodes.clear();
        coupler_visibility_state = -1;
        const Array coupler_names = coupler_submodel_paths.keys();
        for (int index = 0; index < coupler_names.size(); ++index) {
            if (Node3D *node = node_at<Node3D>(this, coupler_submodel_paths[coupler_names[index]]); node != nullptr) {
                coupler_submodel_nodes[coupler_names[index]] = _id_of(node);
            }
        }

        wiper_arm_nodes.clear();
        wiper_applied_positions.clear();
        for (int index = 0; index < wiper_arm_paths.size(); ++index) {
            const NodePath path = wiper_arm_paths[index];
            Node3D *node = path.is_empty() ? nullptr : node_at<Node3D>(this, path);
            _capture_rest_basis(node);
            wiper_arm_nodes.push_back(_id_of(node));
        }

        mirror_nodes.clear();
        mirror_applied_left = -1.0;
        // resolved here: the bindings are cached before _process_dirty() takes model_node
        const Node *model = model_instance_path.is_empty() ? nullptr : node_at<Node>(this, model_instance_path);
        for (int index = 0; index < mirror_paths.size(); ++index) {
            const NodePath path = mirror_paths[index];
            Node3D *node = path.is_empty() ? nullptr : node_at<Node3D>(this, path);
            if (node == nullptr) {
                continue;
            }
            // the original's offset() is the position in the model, through every parent (Model3d.cpp:1584)
            Transform3D model_transform = node->get_transform();
            for (Node3D *parent = Object::cast_to<Node3D>(node->get_parent()); parent != nullptr && parent != model;
                 parent = Object::cast_to<Node3D>(parent->get_parent())) {
                model_transform = parent->get_transform() * model_transform;
            }
            _capture_rest_basis(node);
            mirror_nodes.push_back({_id_of(node), index % 2 == 1, model_transform.origin.z > 0.0});
        }

        pantograph_front_arm_nodes = _resolve_pantograph_arm_nodes(pantograph_front_arm_paths);
        pantograph_rear_arm_nodes = _resolve_pantograph_arm_nodes(pantograph_rear_arm_paths);
        _publish_pantograph_geometry(RailVehicleElectricEngine::PANTOGRAPH_FIRST, pantograph_front_arm_nodes);
        _publish_pantograph_geometry(RailVehicleElectricEngine::PANTOGRAPH_SECOND, pantograph_rear_arm_nodes);
        for (const Vector<ObjectID> *arm_nodes: {&pantograph_front_arm_nodes, &pantograph_rear_arm_nodes}) {
            for (const ObjectID &arm: *arm_nodes) {
                _capture_rest_basis(_object<Node3D>(arm));
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
        if (left > 0) {
            return 1;
        }
        return right > 0 ? 2 : 0;
    }

    // Original engine: TDynamicObject::SetPneumatic() (DynObj.cpp:430) - picks the hose submodel
    // matching the layout of the vehicle coupled at that end: 1 straight, 2 slanted, 3 slanted "r",
    // 4 straight "r"
    int RailVehicle3D::_pneumatic_variant(const int p_end, const bool p_brake_hose) const {
        const Ref<RailVehicleBuffCoupl> coupler = _coupler();
        if (coupler.is_null()) {
            return 0;
        }
        const RailVehicleBuffCoupl::End end = static_cast<RailVehicleBuffCoupl::End>(p_end);
        const int own = get_pneumatic_layout(p_end, p_brake_hose);
        int other = 0;
        RailVehicleServer *server = RailVehicleServer::get_instance();
        // the vehicles coupled beyond p_end, from the farthest one back through this one: the
        // neighbour is the one just before it
        const TypedArray<RID> coupled =
                server != nullptr
                        ? server->vehicle_get_coupled(rid, p_end, RailVehicleController::COUPLING_ELEMENT_COUPLER)
                        : TypedArray<RID>();
        if (const int64_t own_index = coupled.find(rid); own_index > 0) {
            const ObjectID other_id = ObjectID(server->vehicle_get_rail_vehicle(coupled[own_index - 1]));
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
        if (own == 2) {
            return 4;
        }
        return own == 1 ? 1 : 0;
    }

    // Original engine: AirCoupler::Update() (AirCoupler.cpp:83)
    void RailVehicle3D::_show_air_coupler(const String &p_name, const bool p_on, const bool p_xon) {
        const bool states[] = {p_on, !(p_on || p_xon), p_xon};
        const char *suffixes[] = {"_on", "_off", "_xon"};
        for (int index = 0; index < 3; ++index) {
            const ObjectID *id = coupler_submodel_nodes.getptr(p_name + String(suffixes[index]));
            if (Node3D *node = id != nullptr ? _object<Node3D>(*id) : nullptr; node != nullptr) {
                node->set_visible(states[index]);
            }
        }
    }

    Ref<RailVehicleBuffCoupl> RailVehicle3D::_coupler() const {
        const RailVehicleServer *server = RailVehicleServer::get_instance();
        return server != nullptr && rid.is_valid() ? Ref<RailVehicleBuffCoupl>(server->vehicle_component_get(
                                                             rid, RailVehicleComponentType::COMPONENT_BUFFERS))
                                                   : Ref<RailVehicleBuffCoupl>();
    }

    // Original engine: coupler and hose submodel visibility (DynObj.cpp:758-925, bnewAirCouplers branch)
    void RailVehicle3D::_update_couplers() {
        const Ref<RailVehicleBuffCoupl> coupler = _coupler();
        if (coupler_submodel_nodes.is_empty() || coupler.is_null()) {
            return;
        }
        int variants[2][3];
        int64_t state = 0;
        for (int end = 0; end < 2; ++end) {
            const RailVehicleBuffCoupl::End vehicle_end = static_cast<RailVehicleBuffCoupl::End>(end);
            // _on for the vehicle that draws the coupler, _xon (or _off without it) for the other
            if (!coupler->is_coupled(vehicle_end)) {
                variants[end][0] = 0;
            } else if (coupler->is_coupling_owner(vehicle_end)) {
                variants[end][0] = 1;
            } else {
                variants[end][0] = 2;
            }
            variants[end][1] = coupler->is_brake_hose_connected(vehicle_end) ? _pneumatic_variant(end, true) : 0;
            variants[end][2] = coupler->is_main_hose_connected(vehicle_end) ? _pneumatic_variant(end, false) : 0;
            for (const int variant: variants[end]) {
                state = (state * 5) + variant;
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
        const Ref<RailVehicleWipers> wipers = _component<RailVehicleWipers>(VehicleComponentType::COMPONENT_WIPERS);
        if (wiper_arm_nodes.is_empty() || wipers.is_null()) {
            return;
        }
        const PackedFloat64Array positions = wipers->get_sweep_positions();
        if (positions == wiper_applied_positions) {
            return;
        }
        wiper_applied_positions = positions;
        const double wiper_angle = Math::deg_to_rad(wipers->get_angle());
        for (int wiper = 0; wiper < positions.size() && (static_cast<int64_t>(wiper) + 1) * 3 <= wiper_arm_nodes.size();
             ++wiper) {
            // the state tells the way out (0..1) from the way back (1..2)
            double sweep = positions[wiper] > 1.0 ? positions[wiper] - 1.0 : positions[wiper];
            // smoothInterpolate() (utilities.h:324)
            sweep = sweep * sweep * (3.0 - (2.0 * sweep));
            const double angle = (wiper % 2 == 1 ? -wiper_angle : wiper_angle) * sweep;
            for (int element = 0; element < 3; ++element) {
                const ObjectID id = wiper_arm_nodes[(wiper * 3) + element];
                Node3D *node = _object<Node3D>(id);
                if (node == nullptr) {
                    continue;
                }
                Transform3D transform = node->get_transform();
                transform.basis = node_rest_bases[id] *
                                  Basis(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(element == 2 ? -angle : angle));
                node->set_transform(transform);
            }
        }
    }

    // TDynamicObject::UpdateMirror() (DynObj.cpp:748-766): the mirrors at the end of the occupied
    // cab turn out about their vertical axis by MirrorMaxShift, as far as their side is unfolded.
    void RailVehicle3D::_update_mirrors() {
        const Ref<RailVehicleDoors> doors = _component<RailVehicleDoors>(VehicleComponentType::COMPONENT_DOORS);
        const RailVehicleController *vehicle = _object<RailVehicleController>(controller_id);
        if (mirror_nodes.empty() || doors.is_null() || vehicle == nullptr) {
            return;
        }
        const double left = doors->get_mirror_left_position();
        const double right = doors->get_mirror_right_position();
        const int occupied_cab = vehicle->get_occupied_cab();
        if (left == mirror_applied_left && right == mirror_applied_right && occupied_cab == mirror_applied_cab) {
            return;
        }
        mirror_applied_left = left;
        mirror_applied_right = right;
        mirror_applied_cab = occupied_cab;
        const double max_shift = Math::deg_to_rad(doors->get_mirror_max_shift());
        for (const MirrorNode &mirror: mirror_nodes) {
            const bool active = mirror.front ? occupied_cab > 0 : occupied_cab < 0;
            const double angle = active ? max_shift * (mirror.right ? right : left) : 0.0;
            Node3D *node = _object<Node3D>(mirror.node);
            if (node == nullptr) {
                continue;
            }
            Transform3D transform = node->get_transform();
            transform.basis = node_rest_bases[mirror.node] * Basis(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(angle));
            node->set_transform(transform);
        }
    }

    void RailVehicle3D::_apply_wheel_rotation(const Vector<ObjectID> &p_nodes, double p_angle_degrees) {
        for (const ObjectID &id: p_nodes) {
            _apply_node_rotation(id, p_angle_degrees);
        }
    }

    /* A node turned about its own x axis from the pose it was captured in */
    void RailVehicle3D::_apply_node_rotation(const ObjectID &p_node, const double p_angle_degrees) {
        Node3D *node = _object<Node3D>(p_node);
        const Basis *rest_basis = node_rest_bases.getptr(p_node);
        if (node == nullptr || rest_basis == nullptr) {
            return;
        }
        Transform3D transform = node->get_transform();
        transform.basis =
                *rest_basis * Basis(Vector3(1.0, 0.0, 0.0), static_cast<real_t>(Math::deg_to_rad(p_angle_degrees)));
        node->set_transform(transform);
    }

    void RailVehicle3D::_update_wheel_animation_state() {
        const Ref<RailVehicleWheels> wheels = _component<RailVehicleWheels>(VehicleComponentType::COMPONENT_WHEELS);
        if (wheels.is_null()) {
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
        Node3D *model_node = _object<Node3D>(model_node_id);
        Node3D *low_poly_cabin = _object<Node3D>(low_poly_cabin_id);
        const SceneryStreamingServer *streaming = SceneryStreamingServer::get_instance();
        if (streaming == nullptr || !streaming->streaming_has_camera() || model_node == nullptr) {
            return;
        }
        const float detail_distance = ProjectSettings::get_singleton()->get_setting(
                "maszyna/vehicles/detail_distance", DEFAULT_VEHICLE_DETAIL_DISTANCE_M);
        const double distance = get_global_position().distance_to(streaming->streaming_get_camera_position());
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
    /// without a diesel engine gets the template's own rate, like a scenery chimney - the server
    /// starts a vehicle's emitters silent, so even that has to be said.
    /// Called from the 0.25 s block of _process_impl() - a plume changes slowly - and whenever the
    /// model is (re)built, so new emitters never spawn at a rate the engine did not give them.
    void RailVehicle3D::_update_smoke() {
        Node3D *model_node = _object<Node3D>(model_node_id);
        // without a vehicle the engine type is unknown
        if (model_node == nullptr || !rid.is_valid() || !bool(model_node->call("is_e3d_loaded"))) {
            return;
        }
        const Ref<RailVehicleEngine> engine = _component<RailVehicleEngine>(VehicleComponentType::COMPONENT_ENGINE);
        const Ref<RailVehicleDieselEngine> diesel_engine = engine;
        const Ref<RailVehicleElectricEngine> electric_engine = engine;
        const RailVehicleController *vehicle = _object<RailVehicleController>(controller_id);
        if (vehicle == nullptr) {
            return;
        }
        const int engine_type = engine.is_valid() ? engine->get_type() : RailVehicleEngine::NONE;
        if (engine_type != RailVehicleEngine::DIESEL && engine_type != RailVehicleEngine::DIESEL_ELECTRIC) {
            model_node->call("set_smoke_intensity", 1.0);
            return;
        }

        const double revolutions = engine->get_rpm_count(); // rev/s, as the Mover keeps enrot
        const double max_rpm = diesel_engine.is_valid() ? diesel_engine->get_max_rpm() : 0.0;
        const double power = engine->get_power(); // kW
        /* The motor's current, which only an engine that has motors has - it used to be read as
         * "engine_current", a key no component publishes, so this term was always zero. */
        const double current = electric_engine.is_valid() ? electric_engine->get_motor_current() : 0.0;
        const double direction = vehicle->get_direction_absolute();

        double intensity;
        if (diesel_engine.is_valid() && diesel_engine->get_spinup()) {
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
        const double fill = CLAMP(diesel_engine.is_valid() ? diesel_engine->get_fill() : 0.0, 0.0, 1.0);
        model_node->call("set_smoke_intensity", CLAMP(intensity, 0.0, 1.0) * fill);
    }

    void RailVehicle3D::apply_track_placement() {
        Node3D *front_bogie_node = _object<Node3D>(front_bogie_node_id);
        Node3D *rear_bogie_node = _object<Node3D>(rear_bogie_node_id);
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
        const Ref<RailVehicleWheels> wheels = _component<RailVehicleWheels>(VehicleComponentType::COMPONENT_WHEELS);
        if (wheels.is_null()) {
            _update_wheel_animation_state();
            return;
        }
        const Transform3D front_transform = wheels->get_bogie_transform(RailVehicleWheels::BOGIE_FRONT);
        const Transform3D rear_transform = wheels->get_bogie_transform(RailVehicleWheels::BOGIE_REAR);
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
            const Basis *rest_basis = bogie_rest_global_bases.getptr(_id_of(bogie_nodes[index]));
            if (rest_basis != nullptr) {
                const double yaw_delta = -(bogie_yaw - body_yaw);
                bogie_nodes[index]->set_global_basis(
                        get_global_basis() * Basis(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(yaw_delta)) *
                        *rest_basis);
            }
        }
        _update_wheel_animation_state();
    }

    void RailVehicle3D::_update_pantograph_animation() {
        const RailVehicleServer *server = RailVehicleServer::get_instance();
        if (server == nullptr || !rid.is_valid()) {
            return;
        }
        if (pantograph_front_arm_nodes.size() == 5) {
            _apply_pantograph_animation(
                    pantograph_front_arm_nodes,
                    server->vehicle_get_pantograph_raise(rid, RailVehicleElectricEngine::PANTOGRAPH_FIRST));
        }
        if (pantograph_rear_arm_nodes.size() == 5) {
            _apply_pantograph_animation(
                    pantograph_rear_arm_nodes,
                    server->vehicle_get_pantograph_raise(rid, RailVehicleElectricEngine::PANTOGRAPH_SECOND));
        }
    }

    void RailVehicle3D::_apply_pantograph_animation(const Vector<ObjectID> &p_nodes, const Vector2 &p_raise) {
        const double a_deg = Math::rad_to_deg(double(p_raise.x));
        const double b_deg = Math::rad_to_deg(double(p_raise.y));
        const double c_deg = a_deg + b_deg;
        // lower arm, its pair, upper arm, its pair, slider
        const double angles[] = {-a_deg, a_deg, c_deg, -c_deg, -b_deg};
        for (int64_t index = 0; index < p_nodes.size(); ++index) {
            _apply_node_rotation(p_nodes[index], angles[index]);
        }
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
    DEFINE_ARRAY_PROPERTY(mirror_paths)

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
            head_display_e3d_id = ObjectID();
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
