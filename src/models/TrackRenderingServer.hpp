#ifndef TRACK_RENDERING_SERVER_HPP
#define TRACK_RENDERING_SERVER_HPP

#include "macros.hpp"
#include "models/e3d/E3DModelManager.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

namespace godot {

    class TrackRenderingServer : public Object {
            GDCLASS(TrackRenderingServer, Object)

        private:
            struct TrackState {
                    RID scenario;

                    RID rail_mesh_instance;
                    RID rail_mesh;
                    Ref<ArrayMesh> internal_rail_mesh;
                    Ref<ShaderMaterial> rail_material;

                    RID sleeper_multimesh_instance;
                    RID sleeper_multimesh;
                    Ref<Mesh> sleeper_mesh;
                    Ref<ShaderMaterial> sleeper_material;

                    RID ballast_mesh_instance;
                    RID ballast_mesh;
                    Ref<ArrayMesh> internal_ballast_mesh;
                    Ref<ShaderMaterial> ballast_material;

                    PackedVector4Array path_points;
                    PackedVector4Array path_quats;
                    float path_length = 0.0f;
                    String sleeper_model_name;
                    String sleeper_model_skin;
                    String ballast_texture_path;

                    float sleeper_height = -0.25f;
                    float sleeper_spacing = 1.0f;
                    float rail_spacing = 1.6f;
                    float rail_height = 0.16f;
                    float ballast_height = 0.4f;
                    float ballast_offset = -0.05f;
                    float ballast_uv_scale = 1.0f;
                    float ballast_width_tiling = 1.0f;
                    float ballast_length_tiling = 1.0f;

                    bool ballast_enabled = true;
                    bool visible = true;
                    Transform3D transform;
            };

            static TrackRenderingServer *singleton;

            HashMap<int64_t, TrackState> tracks;
            int64_t next_track_id = 1;
            E3DModelManager *model_manager = nullptr;
            TrackState *get_track_state(int64_t p_track_id);
            const TrackState *get_track_state(int64_t p_track_id) const;
            void _free_track_rids(TrackState &p_state);
            void _update_render_instances(TrackState &p_state);
            void _clear_sleeper_material(TrackState &p_state);
            void _reload_sleeper_model(int64_t p_track_id, TrackState &p_state);
            Ref<ShaderMaterial> _create_shader_material(const String &p_shader_path) const;
            void _apply_material_override(const RID &p_instance, const Ref<ShaderMaterial> &p_material) const;

        protected:
            static void _bind_methods();

        public:
            TrackRenderingServer();
            ~TrackRenderingServer() override;

            static TrackRenderingServer *get_singleton();

            int64_t create_track();
            void free_track(int64_t p_track_id);

            void set_track_transform(int64_t p_track_id, const Transform3D &p_transform);
            Transform3D get_track_transform(int64_t p_track_id) const;

            void set_track_visible(int64_t p_track_id, bool p_visible);
            bool is_track_visible(int64_t p_track_id) const;

            void set_track_scenario(int64_t p_track_id, RID p_scenario);
            RID get_track_scenario(int64_t p_track_id) const;

            void update_track_data(
                    int64_t p_track_id, const PackedVector4Array &p_points, const PackedVector4Array &p_quats,
                    float p_length);
            void update_rail_mesh(
                    int64_t p_track_id, float p_length, float p_rail_spacing, float p_rail_height,
                    float p_curve_precision);
            void update_ballast_mesh(int64_t p_track_id, float p_length, float p_curve_precision);

            void set_sleeper_model_name(int64_t p_track_id, const String &p_sleeper_model_name);
            void set_sleeper_model_skin(int64_t p_track_id, const String &p_sleeper_model_skin);

            void set_sleeper_mesh(int64_t p_track_id, const Ref<Mesh> &p_mesh);
            Ref<Mesh> get_sleeper_mesh(int64_t p_track_id) const;

            void set_ballast_texture(int64_t p_track_id, const String &p_path);

            void set_sleeper_height(int64_t p_track_id, float p_height);
            float get_sleeper_height(int64_t p_track_id) const;

            void set_sleeper_spacing(int64_t p_track_id, float p_spacing);
            float get_sleeper_spacing(int64_t p_track_id) const;

            void set_rail_spacing(int64_t p_track_id, float p_spacing);
            float get_rail_spacing(int64_t p_track_id) const;

            void set_rail_height(int64_t p_track_id, float p_height);
            float get_rail_height(int64_t p_track_id) const;

            void set_ballast_height(int64_t p_track_id, float p_height);
            float get_ballast_height(int64_t p_track_id) const;

            void set_ballast_offset(int64_t p_track_id, float p_offset);
            float get_ballast_offset(int64_t p_track_id) const;

            void set_ballast_uv_scale(int64_t p_track_id, float p_scale);
            float get_ballast_uv_scale(int64_t p_track_id) const;

            void set_ballast_width_tiling(int64_t p_track_id, float p_tiling);
            float get_ballast_width_tiling(int64_t p_track_id) const;

            void set_ballast_length_tiling(int64_t p_track_id, float p_tiling);
            float get_ballast_length_tiling(int64_t p_track_id) const;

            void set_ballast_enabled(int64_t p_track_id, bool p_enabled);
            bool get_ballast_enabled(int64_t p_track_id) const;

            RID get_instance_rid(int64_t p_track_id) const;
            RID get_rail_mesh_instance(int64_t p_track_id) const;
            RID get_sleeper_multimesh_instance(int64_t p_track_id) const;
            RID get_ballast_mesh_instance(int64_t p_track_id) const;
    };

} // namespace godot

#endif // TRACK_RENDERING_SERVER_HPP
