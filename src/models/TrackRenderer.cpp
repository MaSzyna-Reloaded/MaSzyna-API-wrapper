#include "TrackRenderer.hpp"
#include <cmath>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

    void TrackRenderer::set_collision_enabled(bool p_enabled) {
        collision_enabled = p_enabled;
        _update_render_instances();
    }

    void TrackRenderer::_bind_methods() {
        ClassDB::bind_method(
                D_METHOD("update_track_data", "points", "quats", "length"), &TrackRenderer::update_track_data);
        ClassDB::bind_method(D_METHOD("update_rail_mesh", "length", "rail_spacing", "curve_precision"), &TrackRenderer::update_rail_mesh);
        ClassDB::bind_method(D_METHOD("set_rail_material", "material"), &TrackRenderer::set_rail_material);
        ClassDB::bind_method(D_METHOD("get_rail_material"), &TrackRenderer::get_rail_material);
        ClassDB::bind_method(D_METHOD("set_sleeper_mesh", "mesh"), &TrackRenderer::set_sleeper_mesh);
        ClassDB::bind_method(D_METHOD("get_sleeper_mesh"), &TrackRenderer::get_sleeper_mesh);
        ClassDB::bind_method(D_METHOD("set_sleeper_material", "material"), &TrackRenderer::set_sleeper_material);
        ClassDB::bind_method(D_METHOD("get_sleeper_material"), &TrackRenderer::get_sleeper_material);
        ClassDB::bind_method(D_METHOD("get_instance_rid"), &TrackRenderer::get_instance_rid);
        ClassDB::bind_method(D_METHOD("get_rail_mesh_instance"), &TrackRenderer::get_rail_mesh_instance);
        ClassDB::bind_method(
                D_METHOD("get_sleeper_multimesh_instance"), &TrackRenderer::get_sleeper_multimesh_instance);

        ClassDB::bind_method(D_METHOD("set_sleeper_height", "height"), &TrackRenderer::set_sleeper_height);
        ClassDB::bind_method(D_METHOD("get_sleeper_height"), &TrackRenderer::get_sleeper_height);
        ClassDB::bind_method(D_METHOD("set_sleeper_spacing", "spacing"), &TrackRenderer::set_sleeper_spacing);
        ClassDB::bind_method(D_METHOD("get_sleeper_spacing"), &TrackRenderer::get_sleeper_spacing);

        ClassDB::bind_method(D_METHOD("set_rail_spacing", "spacing"), &TrackRenderer::set_rail_spacing);
        ClassDB::bind_method(D_METHOD("get_rail_spacing"), &TrackRenderer::get_rail_spacing);
        ClassDB::bind_method(D_METHOD("set_collision_enabled", "enabled"), &TrackRenderer::set_collision_enabled);
        ClassDB::bind_method(D_METHOD("is_collision_enabled"), &TrackRenderer::is_collision_enabled);

        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "rail_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
                "set_rail_material", "get_rail_material");
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "sleeper_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_sleeper_mesh",
                "get_sleeper_mesh");
        ADD_PROPERTY(
                PropertyInfo(Variant::OBJECT, "sleeper_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"),
                "set_sleeper_material", "get_sleeper_material");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sleeper_height"), "set_sleeper_height", "get_sleeper_height");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sleeper_spacing"), "set_sleeper_spacing", "get_sleeper_spacing");
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rail_spacing"), "set_rail_spacing", "get_rail_spacing");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_enabled"), "set_collision_enabled", "is_collision_enabled");
    }

    TrackRenderer::TrackRenderer() {
        RenderingServer *rs = RenderingServer::get_singleton();

        // Create rail instance
        rail_mesh_instance = rs->instance_create();

        // Create sleeper instance
        sleeper_multimesh_instance = rs->instance_create();
        sleeper_multimesh = rs->multimesh_create();
        rs->instance_set_base(sleeper_multimesh_instance, sleeper_multimesh);

        // Create physics
        physics_shape.instantiate();
        shape_owner_id = shape_owner_create(this);
        shape_owner_add_shape(shape_owner_id, physics_shape);

        collision_enabled = true;
        set_collision_layer(1);
        set_collision_mask(1);

        set_notify_transform(true);
    }

    void TrackRenderer::_notification(int p_what) {
        switch (p_what) {
            case NOTIFICATION_TRANSFORM_CHANGED:
            case NOTIFICATION_ENTER_WORLD:
            case NOTIFICATION_VISIBILITY_CHANGED:
                _update_render_instances();
                break;
        }
    }

    TrackRenderer::~TrackRenderer() {
        RenderingServer *rs = RenderingServer::get_singleton();
        if (rail_mesh_instance.is_valid()) {
            rs->free_rid(rail_mesh_instance);
        }
        if (rail_mesh.is_valid()) {
            rs->free_rid(rail_mesh);
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
    }

    void TrackRenderer::_update_render_instances() {
        if (!is_inside_tree()) {
            return;
        }

        RenderingServer *rs = RenderingServer::get_singleton();
        Ref<World3D> world = get_world_3d();
        if (world.is_null()) {
            return;
        }
        RID scenario = world->get_scenario();

        const bool visible = is_visible_in_tree();
        rs->instance_set_scenario(rail_mesh_instance, visible ? scenario : RID());
        rs->instance_set_scenario(sleeper_multimesh_instance, visible ? scenario : RID());

        const Transform3D gt = get_global_transform();
        rs->instance_set_transform(rail_mesh_instance, gt);
        rs->instance_set_transform(sleeper_multimesh_instance, gt);

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
        }

        // 1. Update Rails Visual
        rs->instance_geometry_set_shader_parameter(rail_mesh_instance, "path_length", safe_length);
        rs->instance_geometry_set_shader_parameter(rail_mesh_instance, "path_point_count", static_cast<int>(p_points.size()));
        rs->instance_geometry_set_shader_parameter(rail_mesh_instance, "sleeper_height", sleeper_height);

        if (rail_material.is_valid()) {
            rail_material->set_shader_parameter("path_points", p_points);
            rail_material->set_shader_parameter("path_quats", p_quats);
        }

        // 2. Update Sleepers Visual
        if (sleeper_multimesh.is_valid() && sleeper_mesh.is_valid()) {
            int count = static_cast<int>(std::floor(safe_length / sleeper_spacing));
            if (count > 0) {
                rs->multimesh_allocate_data(sleeper_multimesh, count, RenderingServer::MULTIMESH_TRANSFORM_3D);
                for (int i = 0; i < count; i++) {
                    rs->multimesh_instance_set_transform(sleeper_multimesh, i, Transform3D());
                }
            }

            rs->instance_geometry_set_shader_parameter(sleeper_multimesh_instance, "path_length", safe_length);
            rs->instance_geometry_set_shader_parameter(
                    sleeper_multimesh_instance, "path_point_count", static_cast<int>(p_points.size()));
            rs->instance_geometry_set_shader_parameter(sleeper_multimesh_instance, "sleeper_height", sleeper_height);
            rs->instance_geometry_set_shader_parameter(sleeper_multimesh_instance, "sleeper_spacing", sleeper_spacing);

            if (sleeper_material.is_valid()) {
                sleeper_material->set_shader_parameter("path_points", p_points);
                sleeper_material->set_shader_parameter("path_quats", p_quats);
            }
        }

        // 3. Update Collision Shape (Low Poly) - Always generate if points present
        if (p_points.size() > 1) {
            PackedVector3Array collision_faces;
            float rail_half_width = 0.04f;
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
            physics_shape->set_backface_collision(true);
        }

        _update_render_instances();
    }

    void TrackRenderer::set_rail_material(const Ref<ShaderMaterial> &p_material) {
        rail_material = p_material;
        if (rail_material.is_valid()) {
            RenderingServer::get_singleton()->instance_geometry_set_material_override(
                    rail_mesh_instance, rail_material->get_rid());
        }
    }

    Ref<ShaderMaterial> TrackRenderer::get_rail_material() const {
        return rail_material;
    }

    void TrackRenderer::set_sleeper_mesh(const Ref<Mesh> &p_mesh) {
        sleeper_mesh = p_mesh;
        if (sleeper_mesh.is_valid()) {
            RenderingServer::get_singleton()->multimesh_set_mesh(sleeper_multimesh, sleeper_mesh->get_rid());
        }
    }

    Ref<Mesh> TrackRenderer::get_sleeper_mesh() const {
        return sleeper_mesh;
    }

    void TrackRenderer::set_sleeper_material(const Ref<ShaderMaterial> &p_material) {
        sleeper_material = p_material;
        if (sleeper_material.is_valid()) {
            RenderingServer::get_singleton()->instance_geometry_set_material_override(
                    sleeper_multimesh_instance, sleeper_material->get_rid());
        }
    }

    Ref<ShaderMaterial> TrackRenderer::get_sleeper_material() const {
        return sleeper_material;
    }


    void TrackRenderer::update_rail_mesh(float length, float rail_spacing, float curve_precision) {
        if (length < 0.1f) return;

        // Cache mechanism
        float old_length = 0.0f;
        float old_spacing = 0.0f;
        if (internal_rail_mesh.is_valid()) {
            old_length = internal_rail_mesh->get_meta("length", 0.0f);
            old_spacing = internal_rail_mesh->get_meta("rail_spacing", 0.0f);
        }

        bool should_generate = internal_rail_mesh.is_null();
        if (!should_generate) {
            if (std::abs(old_length - length) > 1.0f || std::abs(old_spacing - rail_spacing) > 0.001f) {
                should_generate = true;
            }
        }

        if (!should_generate) return;

        if (length < 0.1f) return;

        int steps = std::max(2, static_cast<int>(std::floor(length / curve_precision)));

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

        float offset = rail_spacing * 0.5f;
        int poly_size = rail_poly.size();

        for (int i = 0; i <= steps; i++) {
            float z = (static_cast<float>(i) / steps) * length;
            float v_u = (static_cast<float>(i) / steps) * length;

            int sides[2] = {-1, 1};
            for (int s = 0; s < 2; s++) {
                int side = sides[s];
                for (int j = 0; j < poly_size; j++) {
                    Vector2 p = rail_poly[j];
                    float x_pos = (side * offset) + (p.x * side);
                    vertices.push_back(Vector3(x_pos, p.y, z));
                    normals.push_back(Vector3(p.x, p.y - 0.08f, 0.0f).normalized());
                    uvs.push_back(Vector2(static_cast<float>(j) / (poly_size - 1), v_u));
                }
            }
        }

        int v_per_step = poly_size * 2;
        for (int i = 0; i < steps; i++) {
            for (int side_idx = 0; side_idx < 2; side_idx++) {
                int side_off = side_idx * poly_size;
                for (int j = 0; j < poly_size; j++) {
                    int next_j = (j + 1) % poly_size;
                    int curr = i * v_per_step + side_off + j;
                    int next = i * v_per_step + side_off + next_j;
                    int step_curr = (i + 1) * v_per_step + side_off + j;
                    int step_next = (i + 1) * v_per_step + side_off + next_j;

                    if (side_idx == 1) {
                        indices.push_back(curr); indices.push_back(next); indices.push_back(step_curr);
                        indices.push_back(next); indices.push_back(step_next); indices.push_back(step_curr);
                    } else {
                        indices.push_back(curr); indices.push_back(step_curr); indices.push_back(next);
                        indices.push_back(next); indices.push_back(step_curr); indices.push_back(step_next);
                    }
                }
            }
        }

        Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays[Mesh::ARRAY_VERTEX] = vertices;
        arrays[Mesh::ARRAY_NORMAL] = normals;
        arrays[Mesh::ARRAY_TEX_UV] = uvs;
        arrays[Mesh::ARRAY_INDEX] = indices;

        Ref<ArrayMesh> new_mesh;
        new_mesh.instantiate();
        new_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        internal_rail_mesh = new_mesh;
        internal_rail_mesh->set_meta("length", length);
        internal_rail_mesh->set_meta("rail_spacing", rail_spacing);
        rail_mesh = internal_rail_mesh->get_rid();
        RenderingServer::get_singleton()->instance_set_base(rail_mesh_instance, rail_mesh);
    }
} // namespace godot
