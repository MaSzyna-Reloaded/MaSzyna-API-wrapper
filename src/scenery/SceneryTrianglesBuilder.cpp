#include "scenery/SceneryTrianglesBuilder.hpp"

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <vector>

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
        struct Chunk {
            String texture;
            int x;
            int z;
            Vector3 origin;
            PackedVector3Array vertices;
            PackedVector3Array normals;
            PackedVector2Array uvs;
        };
        Dictionary chunk_indices;
        std::vector<Chunk> chunks;

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

            // Store each triangle once, in the cell containing its centroid.
            for (int base = 0; base + 2 < vertices.size(); base += 3) {
                const Vector3 centroid = (vertices[base] + vertices[base + 1] + vertices[base + 2]) / 3.0;
                const int chunk_x = static_cast<int>(Math::floor(centroid.x / p_chunk_size_m));
                const int chunk_z = static_cast<int>(Math::floor(centroid.z / p_chunk_size_m));
                const String chunk_key = make_chunk_key(texture, chunk_x, chunk_z);
                if (!chunk_indices.has(chunk_key)) {
                    chunk_indices[chunk_key] = static_cast<int64_t>(chunks.size());
                    chunks.push_back({texture, chunk_x, chunk_z, get_chunk_origin(chunk_x, chunk_z, p_chunk_size_m),
                                      PackedVector3Array(), PackedVector3Array(), PackedVector2Array()});
                }
                Chunk &chunk = chunks[static_cast<int64_t>(chunk_indices[chunk_key])];
                // Keep a single owner of each buffer while appending, avoiding copy-on-write.
                for (int vertex_offset = 0; vertex_offset < 3; vertex_offset++) {
                    chunk.vertices.append(vertices[base + vertex_offset] - chunk.origin);
                    chunk.normals.append(normals[base + vertex_offset]);
                    chunk.uvs.append(uvs[base + vertex_offset]);
                }
            }
        }

        Array result;
        for (const Chunk &chunk : chunks) {
            Dictionary entry;
            entry["texture"] = chunk.texture;
            entry["chunk_x"] = chunk.x;
            entry["chunk_z"] = chunk.z;
            entry["origin"] = chunk.origin;
            entry["vertices"] = chunk.vertices;
            entry["normals"] = chunk.normals;
            entry["uvs"] = chunk.uvs;
            result.append(entry);
        }
        return result;
    }
} // namespace godot
