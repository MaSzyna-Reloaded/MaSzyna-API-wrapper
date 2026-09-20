#include "../scenery/SceneryStreamingServer.hpp"
#include "RailVehicle3D.hpp"

#include "../engines/TrainElectricEngine.hpp"
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
        BIND_RAIL_NODE_PATH(controller_path, "TrainController,FIZTrainController");
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
        ClassDB::bind_method(D_METHOD("move_on_track", "distance"), &RailVehicle3D::move_on_track);
        ClassDB::bind_method(D_METHOD("_process", "delta"), &RailVehicle3D::process_manually);
        ClassDB::bind_method(D_METHOD("_process_dirty"), &RailVehicle3D::_process_dirty);
        ClassDB::bind_method(D_METHOD("_jump_into_cabin", "cabin", "player"), &RailVehicle3D::_jump_into_cabin);
        ClassDB::bind_method(D_METHOD("_show_cabin_after_frames"), &RailVehicle3D::_show_cabin_after_frames);
        ClassDB::bind_method(
                D_METHOD("_apply_cabin_camera_configuration"), &RailVehicle3D::_apply_cabin_camera_configuration);
        ClassDB::bind_method(D_METHOD("_on_controller_changed", "controller"), &RailVehicle3D::_on_controller_changed);
        ClassDB::bind_method(D_METHOD("_schedule_head_display_update"), &RailVehicle3D::_schedule_head_display_update);
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
    }

    Object *RailVehicle3D::_singleton(const StringName &p_name) const {
        return get_tree()->get_root()->get_node_or_null(NodePath(p_name));
    }

    void RailVehicle3D::enter_cabin(Node *p_player) {
        if (cabin_scene.is_null()) {
            UtilityFunctions::push_warning(get_name(), " has no cabin_scene; cabin entry not yet supported");
            return;
        }

        camera = Object::cast_to<Node3D>(p_player->call("get_camera"));
        Node3D *new_cabin = Object::cast_to<Node3D>(cabin_scene->instantiate());
        if (new_cabin == nullptr || !new_cabin->has_signal("cabin_ready")) {
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

        if (!controller_path.is_empty()) {
            TrainController *resolved_controller = _resolve_controller(controller_path);
            if (resolved_controller != nullptr) {
                cabin->set("controller_path", resolved_controller->get_path());
            }
        }

        cabin->set_visible(false);
        cabin->connect(
                "cabin_ready", Callable(this, "_jump_into_cabin").bind(cabin, p_player), Object::CONNECT_ONE_SHOT);
        cabin->connect("camera_configuration_changed", Callable(this, "_apply_cabin_camera_configuration"));
        cabin->connect("camera_configuration_changed", Callable(this, "_update_low_poly_cabs_visibility"));
        cabin->set_transform(Transform3D());
        if (cabin_rotate_180deg) {
            cabin->rotate_y(static_cast<real_t>(Math::deg_to_rad(180.0)));
        }
        add_child(cabin);

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
        cabin->disconnect("camera_configuration_changed", Callable(this, "_apply_cabin_camera_configuration"));
        cabin->disconnect("camera_configuration_changed", Callable(this, "_update_low_poly_cabs_visibility"));
        cabin->get_parent()->remove_child(cabin);
        cabin->queue_free();
        cabin = nullptr;
        _update_low_poly_cabs_visibility();
    }

    void RailVehicle3D::_apply_cabin_camera_configuration() {
        if (cabin == nullptr || camera == nullptr || camera->get_parent() != cabin) {
            return;
        }
        camera->set("bound_enabled", cabin->get("camera_bound_enabled"));
        Vector3 bound_min = cabin->get("camera_bound_min");
        Vector3 bound_max = cabin->get("camera_bound_max");
        bound_min.y += 0.5;
        bound_max.y += 1.8;
        camera->set("bound_min", bound_min);
        camera->set("bound_max", bound_max);
        camera->set_global_transform(cabin->call("get_camera_transform"));
        // Original engine looks along VectorFront * CabOccupied (drivermode.cpp:1071), so cab 2
        // faces the opposite way.
        const bool rear_cab = static_cast<int>(cabin->get("cab_number")) < 0;
        if (cabin_rotate_180deg != rear_cab) {
            camera->set_global_basis(get_global_basis());
        } else {
            camera->set_global_basis(
                    get_global_basis().rotated(Vector3(0.0, 1.0, 0.0), static_cast<real_t>(Math::deg_to_rad(180.0))));
        }
    }

    TrainController *RailVehicle3D::_resolve_controller(const NodePath &p_node_path) const {
        Node *node = get_node_or_null(p_node_path);
        if (TrainController *direct_controller = Object::cast_to<TrainController>(node); direct_controller != nullptr) {
            return direct_controller;
        }
        if (node != nullptr && node->has_method("get_controller")) {
            return Object::cast_to<TrainController>(node->call("get_controller"));
        }
        return nullptr;
    }

    TrainController *RailVehicle3D::get_controller() const {
        return controller_path.is_empty() ? nullptr : _resolve_controller(controller_path);
    }

    void RailVehicle3D::_on_controller_changed(TrainController *p_controller) {
        if (controller == p_controller) {
            return;
        }
        if (controller != nullptr) {
            controller->disconnect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
        }
        controller = p_controller;
        electric_engine = nullptr;
        for (int index = 0; index < pantograph_wire_cache.size(); ++index) {
            pantograph_wire_cache[index] = Dictionary();
        }
        if (controller != nullptr) {
            controller->connect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
            TypedArray<Node> electric_engines = controller->find_children("*", "TrainElectricEngine", true, false);
            if (!electric_engines.is_empty()) {
                electric_engine = Object::cast_to<Node>(electric_engines[0]);
            }
        }
        if (rid.is_valid()) {
            _singleton("RailVehiclePhysicsServer")
                    ->call("vehicle_bind_controller", rid, controller != nullptr ? controller->get_rid() : RID());
        }
        if (cabin != nullptr) {
            cabin->call("set_train_controller", controller);
        }
        const Dictionary state = controller != nullptr ? controller->get_state() : Dictionary();
        _on_roof_light_changed(controller != nullptr && bool(state.get("roof_light_enabled", false)));
    }

    void RailVehicle3D::_enter_tree() {
        _singleton("TrackManager")->connect("tracks_changed", Callable(this, "_on_track_manager_tracks_changed"));
        rid = _singleton("RailVehiclePhysicsServer")->call("vehicle_create");
        pending_start_track_retry = !start_track_name.is_empty();
        dirty = true;
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
        _singleton("TrackManager")->disconnect("tracks_changed", Callable(this, "_on_track_manager_tracks_changed"));
        if (model_node != nullptr) {
            model_node->disconnect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
            model_node = nullptr;
        }
        if (rid.is_valid()) {
            _singleton("RailVehiclePhysicsServer")->call("vehicle_free", rid);
            rid = RID();
        }
        if (fiz_controller != nullptr) {
            fiz_controller->disconnect("controller_changed", Callable(this, "_on_controller_changed"));
            fiz_controller = nullptr;
        }
        if (controller != nullptr) {
            controller->disconnect("roof_light_changed", Callable(this, "_on_roof_light_changed"));
            controller = nullptr;
        }
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
        if (dirty) {
            _process_dirty();
        }
        if (animation_bindings_dirty) {
            animation_bindings_dirty = false;
            _cache_animation_bindings();
            force_detail_refresh = true;
            _update_track_transform();
        }

        update_time += p_delta;
        if (update_time > 0.25) {
            update_time = 0.0;
            if (needs_head_display_update) {
                _update_head_display();
            }
            _update_model_detail();
        }

        if (!Engine::get_singleton()->is_editor_hint()) {
            if (rid.is_valid() && !start_track_name.is_empty() && !pending_start_track_retry) {
                // only the velocity is wanted here, and asking for the whole state would rebuild
                // it from the mover for every vehicle of every frame (see get_state())
                const double velocity = controller != nullptr ? controller->get_velocity() : 0.0;
                // the vehicle is moved on its track by RailVehiclePhysicsServer's global step
                if (!Math::is_zero_approx(velocity)) {
                    _update_track_transform();
                }
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

        Node *controller_node = controller_path.is_empty() ? nullptr : get_node_or_null(controller_path);
        Node *new_fiz_controller = nullptr;
        if (controller_node != nullptr && Object::cast_to<TrainController>(controller_node) == nullptr &&
            controller_node->has_signal("controller_changed")) {
            new_fiz_controller = controller_node;
        }
        if (new_fiz_controller == nullptr && !controller_path.is_empty()) {
            const NodePath parent_path = NodePath(String(controller_path).get_base_dir());
            Node *parent_node = parent_path.is_empty() ? nullptr : get_node_or_null(parent_path);
            if (parent_node != nullptr && parent_node->has_signal("controller_changed")) {
                new_fiz_controller = parent_node;
            }
        }
        if (fiz_controller != new_fiz_controller) {
            if (fiz_controller != nullptr) {
                fiz_controller->disconnect("controller_changed", Callable(this, "_on_controller_changed"));
            }
            fiz_controller = new_fiz_controller;
            if (fiz_controller != nullptr) {
                fiz_controller->connect("controller_changed", Callable(this, "_on_controller_changed"));
            }
        }
        _on_controller_changed(get_controller());

        Node3D *new_model_node = model_instance_path.is_empty() ? nullptr : node_at<Node3D>(this, model_instance_path);
        if (model_node != nullptr) {
            model_node->disconnect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
        }
        model_node = new_model_node;
        if (model_node != nullptr) {
            model_node->connect("e3d_loaded", Callable(this, "_on_model_node_e3d_loaded"));
        }
        _sync_model_lights();
        _update_detection_area();

        if (low_poly_cabin != nullptr) {
            low_poly_cabin->disconnect("e3d_loaded", Callable(this, "_on_low_poly_cabin_e3d_loaded"));
        }
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

    void RailVehicle3D::_on_model_node_e3d_loaded() {
        _sync_model_lights();
        _update_detection_area();
        animation_bindings_dirty = true;
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
        const int cab_number = cabin == nullptr ? 0 : static_cast<int>(cabin->get("cab_number"));
        const int occupied_cab_index = cab_number < 0 ? 2 : cab_number;
        const bool hifi_cab = cabin != nullptr && bool(cabin->get("has_cab_model"));
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
        _singleton("RailVehiclePhysicsServer")->call("vehicle_move", rid, p_distance);
        _update_track_transform();
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
        Object *track_manager = _singleton("TrackManager");
        const RID track_rid = track_manager->call("track_get_rid_by_name", start_track_name);
        if (!track_rid.is_valid()) {
            return;
        }
        pending_start_track_retry = false;
        _singleton("RailVehiclePhysicsServer")
                ->call("vehicle_set_track", rid, track_rid, start_track_offset, start_direction);
        _update_track_transform();
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
        for (int index = 0; index < p_nodes.size(); ++index) {
            if (Object::cast_to<Node3D>(p_nodes[index]) == nullptr) {
                return {};
            }
        }
        Node3D *lower = Object::cast_to<Node3D>(p_nodes[0]);
        Node3D *upper = Object::cast_to<Node3D>(p_nodes[2]);
        Node3D *slider = Object::cast_to<Node3D>(p_nodes[4]);
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
        const int own = get_pneumatic_layout(p_end, p_brake_hose);
        int other = 0;
        if (TrainController *other_controller = controller->get_coupled_controller(p_end);
            other_controller != nullptr) {
            Node *node = other_controller->get_parent();
            while (node != nullptr && Object::cast_to<RailVehicle3D>(node) == nullptr) {
                node = node->get_parent();
            }
            if (const RailVehicle3D *other_vehicle = Object::cast_to<RailVehicle3D>(node); other_vehicle != nullptr) {
                other = other_vehicle->get_pneumatic_layout(
                        controller->get_mover()->Couplers[p_end].ConnectedNr, p_brake_hose);
            }
        }
        if (own == other) {
            switch (own) {
                case 1:
                    return 2;
                case 2:
                    return 3;
                case 3:
                    return controller->get_mover()->Couplers[p_end].Render ? 1 : 4;
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

    // Original engine: coupler and hose submodel visibility (DynObj.cpp:758-925, bnewAirCouplers branch)
    void RailVehicle3D::_update_couplers() {
        if (coupler_submodel_nodes.is_empty() || controller->get_mover() == nullptr) {
            return;
        }
        const TMoverParameters *mover = controller->get_mover();
        int variants[2][3];
        int64_t state = 0;
        for (int end = 0; end < 2; ++end) {
            const TCoupling &coupler = mover->Couplers[end];
            const bool coupled = (coupler.CouplingFlag & coupling::coupler) != 0;
            // _on for the coupling owner (Render), _xon (or _off without it) for the other vehicle
            variants[end][0] = !coupled ? 0 : (coupler.Render ? 1 : 2);
            variants[end][1] = (coupler.CouplingFlag & coupling::brakehose) != 0 ? _pneumatic_variant(end, true) : 0;
            variants[end][2] = (coupler.CouplingFlag & coupling::mainhose) != 0 ? _pneumatic_variant(end, false) : 0;
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
        if (controller == nullptr) {
            return;
        }
        const Dictionary state = controller->get_state();
        // Same sign as the original's UpdateAxle() (DynObj.cpp:489) - the wheel submodels live in
        // the MaSzyna vehicle frame, which MaszynaRailVehicle3DInstancer converts as a whole.
        _apply_wheel_rotation(front_rolling_wheel_nodes, double(state.get("wheel_angle_front_deg", 0.0)));
        _apply_wheel_rotation(powered_wheel_nodes, double(state.get("wheel_angle_powered_deg", 0.0)));
        _apply_wheel_rotation(rear_rolling_wheel_nodes, double(state.get("wheel_angle_rear_deg", 0.0)));
    }

    /// A vehicle far from the camera is rendered from RenderingServer instances instead of a node
    /// hierarchy: nothing animates at that distance, and the hierarchy is what costs - hundreds of
    /// Node3Ds per vehicle to walk, notify and propagate a transform through, times the hundreds of
    /// vehicles a scenery runs. Its simulation is untouched, it lives in RailVehiclePhysicsServer.
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
                "maszyna/rendering/vehicle_detail_distance", DEFAULT_VEHICLE_DETAIL_DISTANCE_M);
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

    void RailVehicle3D::_update_track_transform() {
        if (!rid.is_valid() || start_track_name.is_empty() || pending_start_track_retry) {
            return;
        }
        Object *physics_server = _singleton("RailVehiclePhysicsServer");
        const Transform3D center_transform = physics_server->call("vehicle_get_transform", rid);
        const bool moved = center_transform != last_center_transform;
        last_center_transform = center_transform;
        set_global_transform(center_transform);

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
        const double pivot_spacing =
                controller != nullptr ? double(controller->get_config().get("bogie_pivot_spacing", 0.0)) : 0.0;
        if (pivot_spacing <= 0.0) {
            _update_wheel_animation_state();
            return;
        }
        // RailVehiclePhysicsServer's track-offset distance is rear-relative (see
        // process_movement()'s own comment on this), so a positive distance here actually
        // samples toward the vehicle's rear, not its front - swapped from what the "front"/
        // "rear" naming below implies. Confirmed live: this flipped the whole vehicle 180
        // degrees the instant it started moving (test_rail_vehicle_idle_orientation_regression.gd).
        const Transform3D front_transform =
                physics_server->call("vehicle_get_transform_at_distance", rid, pivot_spacing * -0.5);
        const Transform3D rear_transform =
                physics_server->call("vehicle_get_transform_at_distance", rid, pivot_spacing * 0.5);
        Vector3 body_forward = front_transform.origin - rear_transform.origin;
        if (body_forward.is_zero_approx()) {
            _update_wheel_animation_state();
            return;
        }
        body_forward.normalize();
        const Vector3 average_up =
                (front_transform.basis.get_column(1) + rear_transform.basis.get_column(1)).normalized();
        const Vector3 z_axis = -body_forward;
        const Vector3 x_axis = average_up.cross(z_axis).normalized();
        const Vector3 y_axis = z_axis.cross(x_axis).normalized();
        set_global_transform(Transform3D(
                Basis(x_axis, y_axis, z_axis).orthonormalized(),
                (front_transform.origin + rear_transform.origin) * 0.5));

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
        const double front_voltage =
                front_active ? _pantograph_wire_voltage(2, pantograph_front_offset, frame, assumed_voltage, current)
                             : 0.0;
        electric_engine->call("set_pantograph_wire_voltage", TrainElectricEngine::PANTOGRAPH_FIRST, front_voltage);
        electric_engine->call(
                "set_pantograph_wire_voltage", TrainElectricEngine::PANTOGRAPH_SECOND,
                rear_active ? _pantograph_wire_voltage(3, pantograph_rear_offset, frame, assumed_voltage, current)
                            : 0.0);
    }

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
        return _singleton("TractionPowerServer")->call("wire_get_voltage", wire_rid, p_assumed_voltage, p_current);
    }

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
                (controller->get_train_type() & TrainController::TRAIN_TYPE_EZT) == TrainController::TRAIN_TYPE_EZT;
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

    Dictionary RailVehicle3D::_find_pantograph_wire(
            int p_index, const Vector3 &p_contact_point, const Vector3 &p_up, const Vector3 &p_forward,
            const Vector3 &p_left) {
        Object *traction_power_server = _singleton("TractionPowerServer");
        Dictionary cache = pantograph_wire_cache[p_index];
        // Original engine: the found wire is kept and its height recomputed every frame (DynObj.cpp:8255-8284),
        // a new search only once the pantograph left that span - a cached height made the wire height change
        // in steps while driving, dropping the contact (and the voltage) whenever it stepped up
        const RID wire_rid = cache.get("rid", RID());
        if (wire_rid.is_valid()) {
            const double height = traction_power_server->call(
                    "wire_get_height_above", wire_rid, p_contact_point, p_up, p_forward, p_left,
                    pantograph_collector_width);
            if (Math::is_finite(height)) {
                Dictionary result;
                result["rid"] = wire_rid;
                result["height"] = height;
                return result;
            }
        }
        // without a wire, search the region every frame like update_traction() (DynObj.cpp:8292)
        const Dictionary result = traction_power_server->call(
                "wire_find_above_with_height", p_contact_point, p_up, p_forward, p_left, pantograph_collector_width);
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
