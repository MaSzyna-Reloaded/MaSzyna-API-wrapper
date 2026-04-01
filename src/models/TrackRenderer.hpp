#ifndef TRACK_RENDERER_HPP
#define TRACK_RENDERER_HPP

#include <godot_cpp/classes/static_body3d.hpp>
#include <godot_cpp/classes/concave_polygon_shape3d.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace godot {

class TrackRenderer : public StaticBody3D {
    GDCLASS(TrackRenderer, StaticBody3D)

private:
    RID scenario;
    
    // Rail rendering
    RID rail_mesh_instance;
    RID rail_mesh;
    Ref<ArrayMesh> internal_rail_mesh;
    Ref<ShaderMaterial> rail_material;
    
    // Sleeper rendering
    RID sleeper_multimesh_instance;
    RID sleeper_multimesh;
    Ref<Mesh> sleeper_mesh;
    Ref<ShaderMaterial> sleeper_material;

    PackedVector4Array path_points;
    PackedVector4Array path_quats;
    float path_length = 0.0f;

    float sleeper_height = 0.0f;
    float sleeper_spacing = 0.6f;
    float rail_spacing = 1.6f;
    bool collision_enabled = true;

    uint32_t shape_owner_id = 0;
    Ref<ConcavePolygonShape3D> physics_shape;

    void _update_render_instances();

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    TrackRenderer();
    ~TrackRenderer() override;

    void _ready() override;
    void _exit_tree() override;

    void update_track_data(const PackedVector4Array &p_points, const PackedVector4Array &p_quats, float p_length);
    void update_rail_mesh(float length, float rail_spacing, float curve_precision);
    
    void set_rail_material(const Ref<ShaderMaterial> &p_material);
    Ref<ShaderMaterial> get_rail_material() const;
    
    void set_sleeper_mesh(const Ref<Mesh> &p_mesh);
    Ref<Mesh> get_sleeper_mesh() const;

    void set_sleeper_material(const Ref<ShaderMaterial> &p_material);
    Ref<ShaderMaterial> get_sleeper_material() const;

    void set_sleeper_height(float p_height) {
        sleeper_height = p_height;
        update_track_data(path_points, path_quats, path_length);
    }
    float get_sleeper_height() const { return sleeper_height; }

    void set_sleeper_spacing(float p_spacing) {
        sleeper_spacing = p_spacing;
        update_track_data(path_points, path_quats, path_length);
    }
    float get_sleeper_spacing() const { return sleeper_spacing; }

    void set_rail_spacing(float p_spacing) {
        rail_spacing = p_spacing;
        update_track_data(path_points, path_quats, path_length);
    }
    float get_rail_spacing() const { return rail_spacing; }

    void set_collision_enabled(bool p_enabled);
    bool is_collision_enabled() const { return collision_enabled; }

    RID get_instance_rid() const { return rail_mesh_instance; }
    RID get_rail_mesh_instance() const { return rail_mesh_instance; }
    RID get_sleeper_multimesh_instance() const { return sleeper_multimesh_instance; }
};

} // namespace godot

#endif // TRACK_RENDERER_HPP
