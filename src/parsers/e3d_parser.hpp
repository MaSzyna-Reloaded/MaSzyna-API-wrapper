#pragma once
#include "e3d/E3DModel.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>

#include <array>
#include <string_view>
#include <vector>

namespace godot {
    class E3DParser : public Object {
            GDCLASS(E3DParser, Object)

        public:
            static E3DParser *get_instance() {
                return dynamic_cast<E3DParser *>(Engine::get_singleton()->get_singleton("E3DParser"));
            }

            Ref<E3DModel> parse(const Ref<FileAccess> &p_file) const;

        protected:
            static void _bind_methods();

        private:
            const int64_t max_31_b = 1LL << 31;
            const int64_t max_32_b = 1LL << 32;
            static constexpr std::array<std::string_view, 19> NON_LIGHTS_TO_EXCLUDE = {
                    "coupler1",      "coupler2",    "cctrl1",       "cctrl2",      "cpass1",
                    "cpass2",        "cpneumatic1", "cpneumatic1r", "cpneumatic2", "cpneumatic2r",
                    "external_only", "mechanik1",   "mechanik2",    "pneumatic1",  "pneumatic1r",
                    "pneumatic2",    "pneumatic2r", "shutters1",    "shutters2",
            };

            struct ChunkHeader {
                    String id;
                    uint32_t len;
                    uint32_t data_len;
            };

            struct Indices {
                    uint32_t i1;
                    uint32_t i2;
                    uint32_t i3;
            };

            struct Vertices {
                    Vector3 v1;
                    Vector3 v2;
                    Vector3 v3;
            };

            struct Edges {
                    Vector3 e1;
                    Vector3 e2;
            };

            struct SubModelData {
                    int next_idx;
                    int first_child_idx;
                    uint32_t type;
                    int name_idx;
                    String name;
                    int anim;
                    uint32_t flags;
                    int matrix_idx;
                    String material;
                    int vertex_count;
                    int first_vertex_idx;
                    int material_idx;
                    bool is_material_colored;
                    float lights_on_threshold;
                    float visibility_light_threshold;
                    Color diffuse_color;
                    Color selfillum_color;
                    float gl_lines_size;
                    float lod_max_distance;
                    float lod_min_distance;
                    float light_range;
                    float light_attenuation;
                    float light_angle;
                    float near_attenuation_start;
                    float near_attenuation_end;
                    bool use_near_attenuation;
                    int far_attenuation_decay;
                    float cos_hotspot_angle;
                    float cos_view_angle;
                    uint32_t index_count;
                    uint32_t first_index_idx;
                    uint32_t transparent;
                    PackedVector3Array vertices;
                    PackedVector3Array normals;
                    PackedVector2Array uvs;
                    PackedInt32Array indices;
                    PackedFloat64Array tangents;
                    Transform3D matrix;
                    float light_energy;
            };

            ChunkHeader _read_chunk_header(const Ref<FileAccess> &p_file) const;
            int u32s(uint32_t p_value) const;
            PackedVector3Array
            _calculate_normals(const PackedVector3Array &p_vertices, const PackedInt32Array &p_indices) const;
            SubModelData _read_submodel(const Ref<FileAccess> &p_file, int p_chunk_size) const;
            std::vector<SubModelData> _parse_file(const Ref<FileAccess> &p_file) const;
            std::vector<String> _buffer_to_strings(const PackedByteArray &p_buffer) const;
            Transform3D _read_matrix(const Ref<FileAccess> &p_file) const;
            Ref<E3DSubModel> _create_submodel(SubModelData &p_submodel) const;
            void _register_lights(
                    const Ref<E3DModel> &p_model, const std::vector<Ref<E3DSubModel>> &p_submodels,
                    const std::vector<SubModelData> &p_meta, const std::vector<int> &p_parent_indices) const;
            NodePath _build_submodel_path(
                    const std::vector<SubModelData> &p_meta, const std::vector<int> &p_parent_indices,
                    int p_index) const;
    };
} // namespace godot
