#include "scenery/SceneryTrianglesBuilder.hpp"

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector3.hpp>

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

    struct ClipVertex {
            Vector3 position;
            Vector3 normal;
            Vector2 uv;
    };

    // The cut is always computed from the lower end of the edge and lands exactly on the bound, so
    // two triangles sharing the edge get the very same vertex and no crack opens between them.
    static ClipVertex cut_edge(const ClipVertex &p_a, const ClipVertex &p_b, const int p_axis, const real_t p_bound) {
        const bool a_is_lower = p_a.position[p_axis] <= p_b.position[p_axis];
        const ClipVertex &from = a_is_lower ? p_a : p_b;
        const ClipVertex &to = a_is_lower ? p_b : p_a;
        const real_t weight = (p_bound - from.position[p_axis]) / (to.position[p_axis] - from.position[p_axis]);
        ClipVertex cut;
        cut.position = from.position.lerp(to.position, weight);
        cut.position[p_axis] = p_bound;
        cut.normal = from.normal.lerp(to.normal, weight).normalized();
        cut.uv = from.uv.lerp(to.uv, weight);
        return cut;
    }

    // Sutherland-Hodgman against one axis-aligned bound; the winding of the polygon is kept.
    static std::vector<ClipVertex> clip_polygon(
            const std::vector<ClipVertex> &p_polygon, const int p_axis, const real_t p_bound, const bool p_keep_above) {
        std::vector<ClipVertex> clipped;
        for (size_t index = 0; index < p_polygon.size(); index++) {
            const ClipVertex &current = p_polygon[index];
            const ClipVertex &next = p_polygon[(index + 1) % p_polygon.size()];
            const bool current_inside =
                    p_keep_above ? current.position[p_axis] >= p_bound : current.position[p_axis] <= p_bound;
            const bool next_inside = p_keep_above ? next.position[p_axis] >= p_bound : next.position[p_axis] <= p_bound;
            if (current_inside) {
                clipped.push_back(current);
            }
            if (!(current_inside == next_inside)) {
                clipped.push_back(cut_edge(current, next, p_axis, p_bound));
            }
        }
        return clipped;
    }

    static std::vector<ClipVertex> clip_to_cell_range(
            const std::vector<ClipVertex> &p_polygon, const int p_axis, const int p_cell, const double p_chunk_size_m) {
        const real_t lower = static_cast<real_t>(static_cast<double>(p_cell) * p_chunk_size_m);
        const real_t upper = static_cast<real_t>(static_cast<double>(p_cell + 1) * p_chunk_size_m);
        return clip_polygon(clip_polygon(p_polygon, p_axis, lower, true), p_axis, upper, false);
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

            // A triangle is cut along the cell grid, each piece stored in its own cell: a chunk is
            // streamed and culled by its cell (SceneryStreamingServer), so geometry reaching outside
            // of it - terrain triangles can be kilometres long - went missing under the camera.
            for (int base = 0; base + 2 < vertices.size(); base += 3) {
                std::vector<ClipVertex> triangle_polygon;
                Vector3 lower = vertices[base];
                Vector3 upper = vertices[base];
                for (int vertex_offset = 0; vertex_offset < 3; vertex_offset++) {
                    const int index = base + vertex_offset;
                    triangle_polygon.push_back({vertices[index], normals[index], uvs[index]});
                    lower = lower.min(vertices[index]);
                    upper = upper.max(vertices[index]);
                }
                const int first_x = static_cast<int>(Math::floor(lower.x / p_chunk_size_m));
                const int last_x = static_cast<int>(Math::floor(upper.x / p_chunk_size_m));
                const int first_z = static_cast<int>(Math::floor(lower.z / p_chunk_size_m));
                const int last_z = static_cast<int>(Math::floor(upper.z / p_chunk_size_m));
                const bool single_cell = first_x == last_x && first_z == last_z;

                for (int chunk_x = first_x; chunk_x <= last_x; chunk_x++) {
                    const std::vector<ClipVertex> strip =
                            single_cell
                                    ? triangle_polygon
                                    : clip_to_cell_range(triangle_polygon, Vector3::AXIS_X, chunk_x, p_chunk_size_m);
                    for (int chunk_z = first_z; chunk_z <= last_z && strip.size() >= 3; chunk_z++) {
                        const std::vector<ClipVertex> piece =
                                single_cell ? strip
                                            : clip_to_cell_range(strip, Vector3::AXIS_Z, chunk_z, p_chunk_size_m);
                        if (piece.size() < 3) {
                            continue;
                        }
                        const String chunk_key = make_chunk_key(texture, chunk_x, chunk_z);
                        if (!chunk_indices.has(chunk_key)) {
                            chunk_indices[chunk_key] = static_cast<int64_t>(chunks.size());
                            chunks.push_back(
                                    {texture, chunk_x, chunk_z, get_chunk_origin(chunk_x, chunk_z, p_chunk_size_m),
                                     PackedVector3Array(), PackedVector3Array(), PackedVector2Array()});
                        }
                        Chunk &chunk = chunks[static_cast<int64_t>(chunk_indices[chunk_key])];
                        // the piece is convex, so a fan from its first vertex keeps the winding
                        for (size_t corner = 1; corner + 1 < piece.size(); corner++) {
                            const ClipVertex *fan[3] = {&piece[0], &piece[corner], &piece[corner + 1]};
                            // a triangle touching the cell with an edge or a corner leaves no area in it
                            const Vector3 area =
                                    (fan[1]->position - fan[0]->position).cross(fan[2]->position - fan[0]->position);
                            if (area.length_squared() < CMP_EPSILON2) {
                                continue;
                            }
                            for (const ClipVertex *vertex: fan) {
                                chunk.vertices.append(vertex->position - chunk.origin);
                                chunk.normals.append(vertex->normal);
                                chunk.uvs.append(vertex->uv);
                            }
                        }
                    }
                }
            }
        }

        Array result;
        for (const Chunk &chunk: chunks) {
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
