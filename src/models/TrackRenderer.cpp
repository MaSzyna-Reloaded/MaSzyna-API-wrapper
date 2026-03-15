#include "TrackRenderer.hpp"
#include "models/MaterialManager.hpp"
#include "models/e3d/E3DModel.hpp"
#include "models/e3d/E3DSubModel.hpp"
#include "models/e3d/instance/E3DModelInstance.hpp"
#include <cmath>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/base_material3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

namespace godot {

    void TrackRenderer::set_ballast_enabled(const bool p_enabled) {
        ballast_enabled = p_enabled;
        _update_render_instances();
    }

    void TrackRenderer::set_collision_enabled(const bool p_enabled) {
        collision_enabled = p_enabled;
        _update_render_instances();
    }

    void TrackRenderer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("update_track_data", "points", "quats", "length"), &TrackRenderer::update_track_data);
        ClassDB::bind_method(
                D_METHOD("update_rail_mesh", "length", "rail_spacing", "curve_precision"),
                &TrackRenderer::update_rail_mesh);
        ClassDB::bind_method(
                D_METHOD("update_ballast_mesh", "length", "curve_precision"),
                &TrackRenderer::update_ballast_mesh);
        ClassDB::bind_method(D_METHOD("set_rail_material", "material"), &TrackRenderer::set_rail_material);
        ClassDB::bind_method(D_METHOD("get_rail_material"), &TrackRenderer::get_rail_material);
        ClassDB::bind_method(D_METHOD("set_sleeper_mesh", "mesh"), &TrackRenderer::set_sleeper_mesh);
        ClassDB::bind_method(D_METHOD("get_sleeper_mesh"), &TrackRenderer::get_sleeper_mesh);
        ClassDB::bind_method(D_METHOD("set_sleeper_material", "material"), &TrackRenderer::set_sleeper_material);
        ClassDB::bind_method(D_METHOD("get_sleeper_material"), &TrackRenderer::get_sleeper_material);
        ClassDB::bind_method(D_METHOD("set_ballast_material", "material"), &TrackRenderer::set_ballast_material);
        ClassDB::bind_method(D_METHOD("get_ballast_material"), &TrackRenderer::get_ballast_material);
        ClassDB::bind_method(D_METHOD("set_ballast_texture", "path"), &TrackRenderer::set_ballast_texture);

        ClassDB::bind_method(D_METHOD("set_sleeper_e3d", "instance"), &TrackRenderer::set_sleeper_e3d);
        ClassDB::bind_method(D_METHOD("_on_sleeper_e3d_loaded"), &TrackRenderer::_on_sleeper_e3d_loaded);
        ClassDB::bind_method(D_METHOD("get_instance_rid"), &TrackRenderer::get_instance_rid);
        ClassDB::bind_method(D_METHOD("get_rail_mesh_instance"), &TrackRenderer::get_rail_mesh_instance);
        ClassDB::bind_method(
                D_METHOD("get_sleeper_multimesh_instance"), &TrackRenderer::get_sleeper_multimesh_instance);
        ClassDB::bind_method(
                D_METHOD("get_ballast_mesh_instance"), &TrackRenderer::get_ballast_mesh_instance);

        ClassDB::bind_method(D_METHOD("set_sleeper_height", "height"), &TrackRenderer::set_sleeper_height);
        ClassDB::bind_method(D_METHOD("get_sleeper_height"), &TrackRenderer::get_sleeper_height);
        ClassDB::bind_method(D_METHOD("set_sleeper_spacing", "spacing"), &TrackRenderer::set_sleeper_spacing);
        ClassDB::bind_method(D_METHOD("get_sleeper_spacing"), &TrackRenderer::get_sleeper_spacing);

        ClassDB::bind_method(D_METHOD("set_rail_spacing", "spacing"), &TrackRenderer::set_rail_spacing);
        ClassDB::bind_method(D_METHOD("get_rail_spacing"), &TrackRenderer::get_rail_spacing);
        ClassDB::bind_method(D_METHOD("set_ballast_height", "height"), &TrackRenderer::set_ballast_height);
        ClassDB::bind_method(D_METHOD("get_ballast_height"), &TrackRenderer::get_ballast_height);
        ClassDB::bind_method(D_METHOD("set_ballast_offset", "offset"), &TrackRenderer::set_ballast_offset);
        ClassDB::bind_method(D_METHOD("get_ballast_offset"), &TrackRenderer::get_ballast_offset);
        ClassDB::bind_method(D_METHOD("set_ballast_uv_scale", "scale"), &TrackRenderer::set_ballast_uv_scale);
        ClassDB::bind_method(D_METHOD("get_ballast_uv_scale"), &TrackRenderer::get_ballast_uv_scale);
        ClassDB::bind_method(D_METHOD("set_ballast_width_tiling", "tiling"), &TrackRenderer::set_ballast_width_tiling);
        ClassDB::bind_method(D_METHOD("get_ballast_width_tiling"), &TrackRenderer::get_ballast_width_tiling);
        ClassDB::bind_method(D_METHOD("set_ballast_length_tiling", "tiling"), &TrackRenderer::set_ballast_length_tiling);
        ClassDB::bind_method(D_METHOD("get_ballast_length_tiling"), &TrackRenderer::get_ballast_length_tiling);
        ClassDB::bind_method(D_METHOD("set_ballast_enabled", "enabled"), &TrackRenderer::set_ballast_enabled);
        ClassDB::bind_method(D_METHOD("is_ballast_enabled"), &TrackRenderer::is_ballast_enabled);
        ClassDB::bind_method(D_METHOD("set_collision_enabled", "enabled"), &TrackRenderer::set_collision_enabled);
        ClassDB::bind_method(D_METHOD("is_collision_enabled"), &TrackRenderer::is_collision_enabled);

        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "rail_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
                "set_rail_material", "get_rail_material");
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "sleeper_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
                "set_sleeper_material", "get_sleeper_material");
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "ballast_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
                "set_ballast_material", "get_ballast_material");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sleeper_height"), "set_sleeper_height", "get_sleeper_height");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sleeper_spacing"), "set_sleeper_spacing", "get_sleeper_spacing");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rail_spacing"), "set_rail_spacing", "get_rail_spacing");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ballast_height"), "set_ballast_height", "get_ballast_height");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ballast_offset"), "set_ballast_offset", "get_ballast_offset");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ballast_uv_scale"), "set_ballast_uv_scale", "get_ballast_uv_scale");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ballast_width_tiling"), "set_ballast_width_tiling", "get_ballast_width_tiling");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ballast_length_tiling"), "set_ballast_length_tiling", "get_ballast_length_tiling");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ballast_enabled"), "set_ballast_enabled", "is_ballast_enabled");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_enabled"), "set_collision_enabled", "is_collision_enabled");
    }

    TrackRenderer::TrackRenderer() : shape_owner_id(create_shape_owner(this)) {
        sleeper_height = -0.25f;
        sleeper_spacing = 1.0f;
        rail_spacing = 1.6f;
        ballast_height = 0.4f;
        ballast_offset = -0.05f;
        ballast_uv_scale = 1.0f;
        ballast_width_tiling = 1.0f;
        ballast_length_tiling = 1.0f;
        ballast_enabled = true;


        RenderingServer *rs = RenderingServer::get_singleton();

        // Create rail instance
        rail_mesh_instance = rs->instance_create();

        // Create ballast instance
        ballast_mesh_instance = rs->instance_create();

        // Create sleeper instance
        sleeper_multimesh_instance = rs->instance_create();
        sleeper_multimesh = rs->multimesh_create();
        rs->instance_set_base(sleeper_multimesh_instance, sleeper_multimesh);

        // Create physics
        physics_shape.instantiate();

        shape_owner_add_shape(shape_owner_id, physics_shape);


        set_collision_layer(1);
        set_collision_mask(1);

        set_notify_transform(true);
    }

    void TrackRenderer::_notification(const int p_what) {
        switch (p_what) {
            case NOTIFICATION_TRANSFORM_CHANGED:
            case NOTIFICATION_ENTER_WORLD:
            case NOTIFICATION_VISIBILITY_CHANGED:
                _update_render_instances();
                break;
            default:;
        }
    }

    TrackRenderer::~TrackRenderer() {
        if (sleeper_e3d_node != nullptr) {
            sleeper_e3d_node->disconnect(E3DModelInstance::E3D_LOADED_SIGNAL, Callable(this, "_on_sleeper_e3d_loaded"));
            sleeper_e3d_node = nullptr;
        }

        RenderingServer *rs = RenderingServer::get_singleton();
        if (rail_mesh_instance.is_valid()) {
            rs->free_rid(rail_mesh_instance);
        }
        if (rail_mesh.is_valid()) {
            rs->free_rid(rail_mesh);
        }
        if (ballast_mesh_instance.is_valid()) {
            rs->free_rid(ballast_mesh_instance);
        }
        if (ballast_mesh.is_valid()) {
            rs->free_rid(ballast_mesh);
        }
        if (sleeper_multimesh_instance.is_valid()) {
            rs->free_rid(sleeper_multimesh_instance);
        }
        if (sleeper_multimesh.is_valid()) {
            rs->free_rid(sleeper_multimesh);
        }
    }

    void TrackRenderer::_ready() {
        if (Engine::get_singleton()->is_editor_hint()) {
            set_process(true);
        }

        _update_render_instances();
    }

    void TrackRenderer::_exit_tree() {
        RenderingServer *rs = RenderingServer::get_singleton();
        rs->instance_set_scenario(rail_mesh_instance, RID());
        rs->instance_set_scenario(sleeper_multimesh_instance, RID());
        rs->instance_set_scenario(ballast_mesh_instance, RID());
    }

    void TrackRenderer::_update_render_instances() {
        if (!is_inside_tree()) {
            return;
        }

        RenderingServer *rs = RenderingServer::get_singleton();
        const Ref<World3D> world = get_world_3d();
        if (world.is_null()) {
            return;
        }
        const RID scenario = world->get_scenario();

        const bool visible = is_visible_in_tree();
        rs->instance_set_scenario(rail_mesh_instance, visible ? scenario : RID());
        rs->instance_set_scenario(sleeper_multimesh_instance, visible ? scenario : RID());
        rs->instance_set_scenario(ballast_mesh_instance, (visible && ballast_enabled) ? scenario : RID());

        const Transform3D gt = get_global_transform();
        rs->instance_set_transform(rail_mesh_instance, gt);
        rs->instance_set_transform(sleeper_multimesh_instance, gt);
        rs->instance_set_transform(ballast_mesh_instance, gt);

        // StaticBody3D properties will automatically follow global_transform and visibility
        shape_owner_set_disabled(shape_owner_id, !(visible && collision_enabled));
    }

    void TrackRenderer::update_track_data(
            const PackedVector4Array &p_points, const PackedVector4Array &p_quats, float p_length) {
        path_points = p_points;
        path_quats = p_quats;
        path_length = p_length;

        RenderingServer *rs = RenderingServer::get_singleton();

        float safe_length = p_length > 0.001f ? p_length : 0.001f;

        // Update AABB for culling
        if (p_points.size() > 0) {
            AABB aabb(Vector3(p_points[0].x, p_points[0].y, p_points[0].z), Vector3(0, 0, 0));
            for (int i = 1; i < p_points.size(); i++) {
                aabb.expand_to(Vector3(p_points[i].x, p_points[i].y, p_points[i].z));
            }
            aabb.grow_by(5.0);
            rs->instance_set_custom_aabb(rail_mesh_instance, aabb);
            rs->instance_set_custom_aabb(sleeper_multimesh_instance, aabb);
            rs->instance_set_custom_aabb(ballast_mesh_instance, aabb);
        }

        // 1. Update Rails Visual
        rs->instance_geometry_set_shader_parameter(rail_mesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(
                rail_mesh_instance, "path_point_count", p_points.size());

        if (rail_material.is_valid()) {
            rail_material->set_shader_parameter("path_points", p_points);
            rail_material->set_shader_parameter("path_quats", p_quats);
        }

        // 2. Update Ballast Visual
        rs->instance_geometry_set_shader_parameter(ballast_mesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(
                ballast_mesh_instance, "path_point_count", p_points.size());

        if (ballast_material.is_valid()) {
            ballast_material->set_shader_parameter("path_points", p_points);
            ballast_material->set_shader_parameter("path_quats", p_quats);
        }

        // 3. Update Sleepers Visual
        rs->instance_geometry_set_shader_parameter(sleeper_multimesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(
                sleeper_multimesh_instance, "path_point_count", p_points.size());
        rs->instance_geometry_set_shader_parameter(sleeper_multimesh_instance, "sleeper_height", sleeper_height);
        float safe_spacing = sleeper_spacing > 0.001f ? sleeper_spacing : 0.001f;
        rs->instance_geometry_set_shader_parameter(sleeper_multimesh_instance, "sleeper_spacing", safe_spacing);

        if (sleeper_material.is_valid()) {
            sleeper_material->set_shader_parameter("path_points", p_points);
            sleeper_material->set_shader_parameter("path_quats", p_quats);
        }

        if (sleeper_multimesh.is_valid() && sleeper_mesh.is_valid()) {
            int count = static_cast<int>(std::floor(safe_length / safe_spacing));
            if (count > 0) {
                rs->multimesh_allocate_data(sleeper_multimesh, count, RenderingServer::MULTIMESH_TRANSFORM_3D);
                rs->multimesh_set_visible_instances(sleeper_multimesh, -1);
                for (int i = 0; i < count; i++) {
                    rs->multimesh_instance_set_transform(sleeper_multimesh, i, Transform3D());
                }
            } else {
                rs->multimesh_allocate_data(sleeper_multimesh, 0, RenderingServer::MULTIMESH_TRANSFORM_3D);
            }
        }

        // 3. Update Collision Shape (Low Poly) - Always generate if points present
        if (p_points.size() > 1) {
            PackedVector3Array collision_faces;
            float rail_top_height = 0.16f;
            float rail_bottom_depth = 0.10f; // Extra depth below sleepers
            float total_rail_height = sleeper_height + rail_top_height;
            float bottom_y = sleeper_height - rail_bottom_depth;
            float half_spacing = rail_spacing * 0.5f;

            for (int i = 0; i < p_points.size() - 1; i++) {
                Vector3 p1_pos = Vector3(p_points[i].x, p_points[i].y, p_points[i].z);
                Vector3 p2_pos = Vector3(p_points[i + 1].x, p_points[i + 1].y, p_points[i + 1].z);
                Quaternion q1(p_quats[i].x, p_quats[i].y, p_quats[i].z, p_quats[i].w);
                Quaternion q2(p_quats[i + 1].x, p_quats[i + 1].y, p_quats[i + 1].z, p_quats[i + 1].w);

                Basis b1(q1);
                Basis b2(q2);

                for (int side: {-1, 1}) {
                    float rail_half_width = 0.04f;
                    float x_off = static_cast<float>(side) * half_spacing;

                    // Top face points
                    Vector3 t1 = p1_pos + b1.xform(Vector3(x_off - rail_half_width, total_rail_height, 0));
                    Vector3 t2 = p1_pos + b1.xform(Vector3(x_off + rail_half_width, total_rail_height, 0));
                    Vector3 t3 = p2_pos + b2.xform(Vector3(x_off + rail_half_width, total_rail_height, 0));
                    Vector3 t4 = p2_pos + b2.xform(Vector3(x_off - rail_half_width, total_rail_height, 0));

                    // Bottom face points
                    Vector3 b1_p = p1_pos + b1.xform(Vector3(x_off - rail_half_width, bottom_y, 0));
                    Vector3 b2_p = p1_pos + b1.xform(Vector3(x_off + rail_half_width, bottom_y, 0));
                    Vector3 b3_p = p2_pos + b2.xform(Vector3(x_off + rail_half_width, bottom_y, 0));
                    Vector3 b4_p = p2_pos + b2.xform(Vector3(x_off - rail_half_width, bottom_y, 0));

                    // Triangles for all faces
                    // Top (Counter-clockwise from above)
                    collision_faces.push_back(t1);
                    collision_faces.push_back(t2);
                    collision_faces.push_back(t3);
                    collision_faces.push_back(t1);
                    collision_faces.push_back(t3);
                    collision_faces.push_back(t4);
                    // Left (looking forward)
                    collision_faces.push_back(b1_p);
                    collision_faces.push_back(t1);
                    collision_faces.push_back(t4);
                    collision_faces.push_back(b1_p);
                    collision_faces.push_back(t4);
                    collision_faces.push_back(b4_p);
                    // Right (looking forward)
                    collision_faces.push_back(b2_p);
                    collision_faces.push_back(b3_p);
                    collision_faces.push_back(t3);
                    collision_faces.push_back(b2_p);
                    collision_faces.push_back(t3);
                    collision_faces.push_back(t2);
                    // Bottom (looking from below)
                    collision_faces.push_back(b1_p);
                    collision_faces.push_back(b4_p);
                    collision_faces.push_back(b3_p);
                    collision_faces.push_back(b1_p);
                    collision_faces.push_back(b3_p);
                    collision_faces.push_back(b2_p);

                    if (i == 0) {
                        // Front cap
                        collision_faces.push_back(b1_p);
                        collision_faces.push_back(b2_p);
                        collision_faces.push_back(t2);
                        collision_faces.push_back(b1_p);
                        collision_faces.push_back(t2);
                        collision_faces.push_back(t1);
                    }
                    if (i == p_points.size() - 2) {
                        // Back cap
                        collision_faces.push_back(b4_p);
                        collision_faces.push_back(t4);
                        collision_faces.push_back(t3);
                        collision_faces.push_back(b4_p);
                        collision_faces.push_back(t3);
                        collision_faces.push_back(b3_p);
                    }
                }
            }

            physics_shape->set_faces(collision_faces);
            physics_shape->set_backface_collision_enabled(true);
        }

        _update_render_instances();
    }

    void TrackRenderer::set_rail_material(const Ref<ShaderMaterial> &p_material) {
        rail_material = p_material;
        RenderingServer::get_singleton()->instance_geometry_set_material_override(
                rail_mesh_instance, rail_material.is_valid() ? rail_material->get_rid() : RID());
        update_track_data(path_points, path_quats, path_length);
    }

    void TrackRenderer::set_sleeper_e3d(Object *p_instance) {
        E3DModelInstance *inst = cast_to<E3DModelInstance>(p_instance);
        if (sleeper_e3d_node == inst) {
            return;
        }

        if (sleeper_e3d_node != nullptr) {
            sleeper_e3d_node->disconnect(E3DModelInstance::E3D_LOADED_SIGNAL, Callable(this, "_on_sleeper_e3d_loaded"));
        }

        sleeper_e3d_node = inst;

        if (sleeper_e3d_node != nullptr) {
            sleeper_e3d_node->connect(E3DModelInstance::E3D_LOADED_SIGNAL, Callable(this, "_on_sleeper_e3d_loaded"));
            _on_sleeper_e3d_loaded();
        }
    }

    void TrackRenderer::_on_sleeper_e3d_loaded() {
        const E3DModelInstance *inst = cast_to<E3DModelInstance>(sleeper_e3d_node);
        if (inst == nullptr) {
            return;
        }

        const Ref<E3DModel> model = inst->get_model();
        if (model.is_null()) {
            return;
        }

        TypedArray<E3DSubModel> submodels = model->get_submodels();
        if (submodels.is_empty()) {
            return;
        }

        // For now, we just take the first submodel that has a mesh
        // This is a simplification but aligns with how it was done in GDScript
        Ref<Mesh> found_mesh;
        Ref<Material> found_mat;
        String found_material_name;
        Color found_diffuse = Color(1, 1, 1);
        bool found_colored = false;
        bool found_transparent = false;
        bool found_dynamic = false;
        int found_dynamic_index = 0;

        std::vector<Ref<E3DSubModel>> stack;
        for (const auto & submodel : submodels) {
            stack.push_back(submodel);
        }

        while (!stack.empty()) {
            const Ref<E3DSubModel> sm = stack.back();
            stack.pop_back();

            if (sm->get_mesh().is_valid()) {
                found_mesh = sm->get_mesh();
                found_material_name = sm->get_material_name();
                found_diffuse = sm->get_diffuse_color();
                found_colored = sm->get_material_colored();
                found_transparent = sm->get_material_transparent();
                found_dynamic = sm->get_dynamic_material();
                found_dynamic_index = sm->get_dynamic_material_index();
                break;
            }

            TypedArray<E3DSubModel> children = sm->get_submodels();
            for (const auto & i : children) {
                stack.emplace_back(i);
            }
        }

        if (found_mesh.is_valid()) {
            set_sleeper_mesh(found_mesh);

            // Try to resolve material
            if (sleeper_material.is_valid()) {
                MaterialManager *mm =
                        cast_to<MaterialManager>(Engine::get_singleton()->get_singleton("MaterialManager"));
                if (mm != nullptr) {
                    const String data_path = inst->get_data_path();
                    // Strip "res://" or leading slash if present, similar to E3DNodesInstancer
                    const String unprefixed_model_path = String("/").join(data_path.split("/").slice(1));

                    if (found_dynamic) {
                        Array skins = inst->get_skins();
                        if (skins.size() > found_dynamic_index) {
                            found_material_name = skins[found_dynamic_index];
                        }
                    }

                    MaterialManager::Transparency transparency =
                            found_transparent ? MaterialManager::Alpha : MaterialManager::Disabled;

                    Ref<StandardMaterial3D> mat;
                    if (found_colored) {
                        // In C++ we don't easily have access to the helper that loads
                        // res://addons/libmaszyna/e3d/colored.material But we can just use a default StandardMaterial3D
                        // if it's just colored
                        mat.instantiate();
                        mat->set_albedo(found_diffuse);
                    } else {
                        mat = mm->get_material(
                                unprefixed_model_path, found_material_name, transparency, false, found_diffuse);
                    }

                    if (mat.is_valid()) {
                        sleeper_material->set_shader_parameter(
                                "albedo_tex", mat->get_texture(BaseMaterial3D::TEXTURE_ALBEDO));
                        sleeper_material->set_shader_parameter("albedo", mat->get_albedo());
                    }
                }
            }
        }
    }

    Ref<ShaderMaterial> TrackRenderer::get_rail_material() const {
        return rail_material;
    }

    void TrackRenderer::set_sleeper_mesh(const Ref<Mesh> &p_mesh) {
        sleeper_mesh = p_mesh;
        RenderingServer *rs = RenderingServer::get_singleton();
        if (sleeper_multimesh.is_valid()) {
            rs->multimesh_set_mesh(sleeper_multimesh, p_mesh.is_valid() ? p_mesh->get_rid() : RID());
            rs->multimesh_set_visible_instances(sleeper_multimesh, -1);
        }
        update_track_data(path_points, path_quats, path_length);
    }

    Ref<Mesh> TrackRenderer::get_sleeper_mesh() const {
        return sleeper_mesh;
    }

    void TrackRenderer::set_sleeper_material(const Ref<ShaderMaterial> &p_material) {
        sleeper_material = p_material;
        RenderingServer::get_singleton()->instance_geometry_set_material_override(
                sleeper_multimesh_instance, sleeper_material.is_valid() ? sleeper_material->get_rid() : RID());
        update_track_data(path_points, path_quats, path_length);
    }

    Ref<ShaderMaterial> TrackRenderer::get_sleeper_material() const {
        return sleeper_material;
    }

    void TrackRenderer::set_ballast_material(const Ref<ShaderMaterial> &p_material) {
        ballast_material = p_material;
        RenderingServer::get_singleton()->instance_geometry_set_material_override(
                ballast_mesh_instance, ballast_material.is_valid() ? ballast_material->get_rid() : RID());
        update_track_data(path_points, path_quats, path_length);
    }

    Ref<ShaderMaterial> TrackRenderer::get_ballast_material() const {
        return ballast_material;
    }

    void TrackRenderer::set_ballast_uv_scale(const float p_scale) {
        ballast_uv_scale = p_scale;
        if (ballast_material.is_valid()) {
            ballast_material->set_shader_parameter("uv_scale", ballast_uv_scale);
        }
    }

    void TrackRenderer::set_ballast_texture(const String &p_path) {
        if (ballast_material.is_null()) {
            return;
        }

        MaterialManager *mm = cast_to<MaterialManager>(Engine::get_singleton()->get_singleton("MaterialManager"));
        if (mm == nullptr) {
            UtilityFunctions::push_warning("[TrackRenderer] Failed to get MaterialManager singleton");
            return;
        }

        // Texture path is relative to game_dir (MaterialManager handles this)
        const Ref<ImageTexture> tex = mm->get_texture(p_path);
        if (!tex.is_valid()) {
            UtilityFunctions::push_warning("[TrackRenderer] Ballast texture is not supported");
            return;
        }

        if (tex.is_valid()) {
            UtilityFunctions::push_warning("[TrackRenderer] Ballast texture is supported");
            ballast_material->set_shader_parameter("albedo_tex", tex);
            ballast_material->set_shader_parameter("albedo", Color(1, 1, 1, 1));
        }
    }

    void TrackRenderer::update_ballast_mesh(const float p_length, const float p_curve_precision) {
        if (p_length < 0.1f) {
            return;
        }

        // Cache mechanism
        float old_length = 0.0f;
        float old_height = 0.0f;
        float old_offset = 0.0f;
        float old_w_tiling = 0.0f;
        float old_l_tiling = 0.0f;
        if (internal_ballast_mesh.is_valid()) {
            old_length = internal_ballast_mesh->get_meta("length", 0.0f);
            old_height = internal_ballast_mesh->get_meta("ballast_height", 0.0f);
            old_offset = internal_ballast_mesh->get_meta("ballast_offset", 0.0f);
            old_w_tiling = internal_ballast_mesh->get_meta("ballast_width_tiling", 0.0f);
            old_l_tiling = internal_ballast_mesh->get_meta("ballast_length_tiling", 0.0f);
        }

        bool should_generate = internal_ballast_mesh.is_null();
        if (!should_generate) {
            if (std::abs(old_length - p_length) > 1.0f ||
                std::abs(old_height - ballast_height) > 0.001f ||
                std::abs(old_offset - ballast_offset) > 0.001f ||
                std::abs(old_w_tiling - ballast_width_tiling) > 0.001f ||
                std::abs(old_l_tiling - ballast_length_tiling) > 0.001f) {
                should_generate = true;
            }
        }

        if (!should_generate) {
            return;
        }

        const int steps = std::max(2, static_cast<int>(std::floor(p_length / p_curve_precision)));

        // Simple trapezoid for ballast
        PackedVector2Array ballast_poly;
        ballast_poly.push_back(Vector2(-1.25f, 0.0f)); // Top left
        ballast_poly.push_back(Vector2(1.25f, 0.0f));  // Top right
        ballast_poly.push_back(Vector2(2.0f, -ballast_height));  // Bottom right
        ballast_poly.push_back(Vector2(-2.0f, -ballast_height)); // Bottom left

        PackedVector3Array vertices;
        PackedVector3Array normals;
        PackedVector2Array uvs;
        PackedInt32Array indices;

        const int poly_size = static_cast<int>(ballast_poly.size());

        for (int i = 0; i <= steps; i++) {
            const float z = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;
            const float v_u = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;

            for (int j = 0; j < poly_size; j++) {
                const Vector2 p = ballast_poly[j];
                vertices.push_back(Vector3(p.x, p.y + ballast_offset, z));

                // Improved normals for trapezoid
                Vector3 n(0, 1, 0);
                if (j == 1 || j == 2) { // Right slope
                    n = Vector3(0.47f, 0.88f, 0.0f);
                } else if (j == 3 || j == 0) { // Left slope
                    n = Vector3(-0.47f, 0.88f, 0.0f);
                }
                normals.push_back(n);

                uvs.push_back(Vector2(p.x * ballast_width_tiling, v_u * ballast_length_tiling));
            }
        }

        for (int i = 0; i < static_cast<int>(steps); i++) {
            for (int j = 0; j < poly_size; j++) {
                const int next_j = (j + 1) % poly_size;
                const int curr = (i * poly_size) + j;
                const int next = (i * poly_size) + next_j;
                const int step_curr = ((i + 1) * poly_size) + j;
                const int step_next = ((i + 1) * poly_size) + next_j;

                indices.push_back(curr);
                indices.push_back(step_curr);
                indices.push_back(next);
                indices.push_back(next);
                indices.push_back(step_curr);
                indices.push_back(step_next);
            }
        }

        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays.set(Mesh::ARRAY_VERTEX, vertices);
        arrays.set(Mesh::ARRAY_NORMAL, normals);
        arrays.set(Mesh::ARRAY_TEX_UV, uvs);
        arrays.set(Mesh::ARRAY_INDEX, indices);

        Ref<ArrayMesh> new_mesh;
        new_mesh.instantiate();
        new_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        internal_ballast_mesh = new_mesh;
        internal_ballast_mesh->set_meta("length", p_length);
        internal_ballast_mesh->set_meta("ballast_height", ballast_height);
        internal_ballast_mesh->set_meta("ballast_offset", ballast_offset);
        internal_ballast_mesh->set_meta("ballast_width_tiling", ballast_width_tiling);
        internal_ballast_mesh->set_meta("ballast_length_tiling", ballast_length_tiling);
        ballast_mesh = internal_ballast_mesh->get_rid();
        RenderingServer::get_singleton()->instance_set_base(ballast_mesh_instance, ballast_mesh);
    }


    void
    TrackRenderer::update_rail_mesh(const float p_length, const float p_rail_spacing, const float p_curve_precision) {
        if (p_length < 0.1f) {
            return;
        }

        // Cache mechanism
        float old_length = 0.0f;
        float old_spacing = 0.0f;
        if (internal_rail_mesh.is_valid()) {
            old_length = internal_rail_mesh->get_meta("length", 0.0f);
            old_spacing = internal_rail_mesh->get_meta("rail_spacing", 0.0f);
        }

        bool should_generate = internal_rail_mesh.is_null();
        if (!should_generate) {
            if (std::abs(old_length - p_length) > 1.0f || std::abs(old_spacing - p_rail_spacing) > 0.001f) {
                should_generate = true;
            }
        }

        if (!should_generate) {
            return;
        }

        if (p_length < 0.1f) {
            return;
        }

        const int steps = std::max(2, static_cast<int>(std::floor(p_length / p_curve_precision)));

        PackedVector2Array rail_poly;
        rail_poly.push_back(Vector2(-0.075f, 0.0f));
        rail_poly.push_back(Vector2(0.075f, 0.0f));
        rail_poly.push_back(Vector2(0.075f, 0.015f));
        rail_poly.push_back(Vector2(0.02f, 0.03f));
        rail_poly.push_back(Vector2(0.02f, 0.12f));
        rail_poly.push_back(Vector2(0.04f, 0.13f));
        rail_poly.push_back(Vector2(0.04f, 0.16f));
        rail_poly.push_back(Vector2(-0.04f, 0.16f));
        rail_poly.push_back(Vector2(-0.04f, 0.13f));
        rail_poly.push_back(Vector2(-0.02f, 0.12f));
        rail_poly.push_back(Vector2(-0.02f, 0.03f));
        rail_poly.push_back(Vector2(-0.075f, 0.015f));

        PackedVector3Array vertices;
        PackedVector3Array normals;
        PackedVector2Array uvs;
        PackedInt32Array indices;

        const float offset = p_rail_spacing * 0.5f;
        const int poly_size = static_cast<int>(rail_poly.size());

        for (int i = 0; i <= steps; i++) {
            const float z = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;
            const float v_u = (static_cast<float>(i) / static_cast<float>(steps)) * p_length;

            for (int s = 0; s < 2; s++) {
                constexpr int sides[2] = {-1, 1};
                const int side = sides[s];
                for (int j = 0; j < poly_size; j++) {
                    const Vector2 p = rail_poly[j];
                    const float x_pos = (static_cast<float>(side) * offset) + (p.x * static_cast<float>(side));
                    vertices.push_back(Vector3(x_pos, p.y, z));
                    normals.push_back(Vector3(p.x, p.y - 0.08f, 0.0f).normalized());
                    uvs.push_back(Vector2(static_cast<float>(j) / static_cast<float>(poly_size - 1), v_u));
                }
            }
        }

        const int v_per_step = poly_size * 2;
        for (int i = 0; i < static_cast<int>(steps); i++) {
            for (int side_idx = 0; side_idx < 2; side_idx++) {
                const int side_off = side_idx * poly_size;
                for (int j = 0; j < poly_size; j++) {
                    const int next_j = (j + 1) % poly_size;
                    const int curr = (i * v_per_step) + side_off + j;
                    const int next = (i * v_per_step) + side_off + next_j;
                    const int step_curr = ((i + 1) * v_per_step) + side_off + j;
                    const int step_next = ((i + 1) * v_per_step) + side_off + next_j;

                    if (side_idx == 1) {
                        indices.push_back(curr);
                        indices.push_back(next);
                        indices.push_back(step_curr);
                        indices.push_back(next);
                        indices.push_back(step_next);
                        indices.push_back(step_curr);
                    } else {
                        indices.push_back(curr);
                        indices.push_back(step_curr);
                        indices.push_back(next);
                        indices.push_back(next);
                        indices.push_back(step_curr);
                        indices.push_back(step_next);
                    }
                }
            }
        }

        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays.set(Mesh::ARRAY_VERTEX, vertices);
        arrays.set(Mesh::ARRAY_NORMAL, normals);
        arrays.set(Mesh::ARRAY_TEX_UV, uvs);
        arrays.set(Mesh::ARRAY_INDEX, indices);

        Ref<ArrayMesh> new_mesh;
        new_mesh.instantiate();
        new_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        internal_rail_mesh = new_mesh;
        internal_rail_mesh->set_meta("length", p_length);
        internal_rail_mesh->set_meta("rail_spacing", p_rail_spacing);
        rail_mesh = internal_rail_mesh->get_rid();
        RenderingServer::get_singleton()->instance_set_base(rail_mesh_instance, rail_mesh);
    }
} // namespace godot
