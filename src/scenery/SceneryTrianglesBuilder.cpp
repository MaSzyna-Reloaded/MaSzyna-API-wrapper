#include "scenery/SceneryTrianglesBuilder.hpp"

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {
    void SceneryTrianglesBuilder::_bind_methods() {
        ClassDB::bind_static_method(
                "SceneryTrianglesBuilder", D_METHOD("build_chunks", "triangles", "chunk_size_m"),
                &SceneryTrianglesBuilder::build_chunks);
    }

    static String make_chunk_key(const String &p_texture, const int p_chunk_x, const int p_chunk_z) {
        return p_texture + String("|") + String::num_int64(p_chunk_x) + String("|") + String::num_int64(p_chunk_z);
    }

    static Vector3 get_chunk_origin(const int p_chunk_x, const int p_chunk_z, const double p_chunk_size_m) {
        return Vector3(
                static_cast<real_t>((static_cast<double>(p_chunk_x) + 0.5) * p_chunk_size_m), 0.0,
                static_cast<real_t>((static_cast<double>(p_chunk_z) + 0.5) * p_chunk_size_m));
    }

    Array SceneryTrianglesBuilder::build_chunks(const Array &p_triangles, const double p_chunk_size_m) {
        Dictionary chunks_by_key;

        for (int i = 0; i < p_triangles.size(); i++) {
            const Array triangle = p_triangles[i];
            if (triangle.size() < 4) {
                continue;
            }

            const String texture = triangle[0];
            const PackedVector3Array vertices = triangle[1];
            const PackedVector3Array normals = triangle[2];
            const PackedVector2Array uvs = triangle[3];

            if (vertices.is_empty()) {
                continue;
            }

            real_t min_x = vertices[0].x;
            real_t max_x = vertices[0].x;
            real_t min_z = vertices[0].z;
            real_t max_z = vertices[0].z;

            for (int vertex_index = 1; vertex_index < vertices.size(); vertex_index++) {
                const Vector3 vertex = vertices[vertex_index];
                min_x = MIN(min_x, vertex.x);
                max_x = MAX(max_x, vertex.x);
                min_z = MIN(min_z, vertex.z);
                max_z = MAX(max_z, vertex.z);
            }

            const int min_chunk_x = Math::floor(min_x / p_chunk_size_m);
            const int max_chunk_x = Math::floor(max_x / p_chunk_size_m);
            const int min_chunk_z = Math::floor(min_z / p_chunk_size_m);
            const int max_chunk_z = Math::floor(max_z / p_chunk_size_m);

            for (int chunk_x = min_chunk_x; chunk_x <= max_chunk_x; chunk_x++) {
                for (int chunk_z = min_chunk_z; chunk_z <= max_chunk_z; chunk_z++) {
                    const String chunk_key = make_chunk_key(texture, chunk_x, chunk_z);
                    Dictionary chunk;

                    if (chunks_by_key.has(chunk_key)) {
                        chunk = chunks_by_key[chunk_key];
                    } else {
                        chunk["texture"] = texture;
                        chunk["chunk_x"] = chunk_x;
                        chunk["chunk_z"] = chunk_z;
                        chunk["origin"] = get_chunk_origin(chunk_x, chunk_z, p_chunk_size_m);
                        chunk["vertices"] = PackedVector3Array();
                        chunk["normals"] = PackedVector3Array();
                        chunk["uvs"] = PackedVector2Array();
                    }

                    const Vector3 chunk_origin = chunk["origin"];
                    PackedVector3Array chunk_vertices = chunk["vertices"];
                    PackedVector3Array chunk_normals = chunk["normals"];
                    PackedVector2Array chunk_uvs = chunk["uvs"];

                    for (int vertex_index = 0; vertex_index < vertices.size(); vertex_index++) {
                        chunk_vertices.append(vertices[vertex_index] - chunk_origin);
                    }

                    chunk_normals.append_array(normals);
                    chunk_uvs.append_array(uvs);

                    chunk["vertices"] = chunk_vertices;
                    chunk["normals"] = chunk_normals;
                    chunk["uvs"] = chunk_uvs;
                    chunks_by_key[chunk_key] = chunk;
                }
            }
        }

        return chunks_by_key.values();
    }
} // namespace godot
