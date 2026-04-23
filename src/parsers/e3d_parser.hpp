#pragma once
#include "models/e3d/E3DModel.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>

#include <vector>

namespace godot {
    class E3DParser : public Node {
            GDCLASS(E3DParser, Node)

        public:
            Ref<E3DModel> parse(const Ref<FileAccess> &p_file) const;

        protected:
            static void _bind_methods();

        private:
            const int64_t max_31_b = 1LL << 31;
            const int64_t max_32_b = 1LL << 32;

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
                    uint32_t index_count;
                    uint32_t first_index_idx;
                    uint32_t transparent;
                    PackedVector3Array vertices;
                    PackedVector3Array normals;
                    PackedVector2Array uvs;
                    PackedInt32Array indices;
                    PackedFloat64Array tangents;
                    Transform3D matrix;
            };

            static ChunkHeader _read_chunk_header(const Ref<FileAccess> &p_file);
            int u32s(uint32_t p_value) const;
            static PackedVector3Array
            _calculate_normals(const PackedVector3Array &p_vertices, const PackedInt32Array &p_indices);
            SubModelData _read_submodel(const Ref<FileAccess> &p_file, int p_chunk_size) const;
            std::vector<SubModelData> _parse_file(const Ref<FileAccess> &p_file) const;
            static TypedArray<String> _buffer_to_strings(const PackedByteArray &p_buffer);
            static Transform3D _read_matrix(const Ref<FileAccess> &p_file);
            static Ref<E3DSubModel> _create_submodel(SubModelData &p_submodel);
    };
} // namespace godot
