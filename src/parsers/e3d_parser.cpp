#include "parsers/e3d_parser.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <cmath>
#include <map>
#include <vector>

namespace godot {
    void E3DParser::_bind_methods() {}

    E3DParser::ChunkHeader E3DParser::_read_chunk_header(const Ref<FileAccess> &p_file) {
        if (!p_file.is_valid()) {
            return {};
        }
        const String chunk_id = p_file->get_buffer(4).get_string_from_ascii();
        const uint32_t chunk_len = p_file->get_32();
        return {chunk_id, chunk_len, chunk_len - 8};
    }

    int E3DParser::u32s(const uint32_t value) const {
        return (static_cast<int>(value) + MAX_31B) % MAX_32B - MAX_31B; // NOLINT(*-math-missing-parentheses)
    }

    PackedVector3Array E3DParser::_calculate_normals(PackedVector3Array vertices, PackedInt32Array indices) {
        PackedVector3Array normals;
        const bool has_indices = indices.size() > 0;
        for (int i = 0; i < vertices.size(); i++) {
            normals.append(Vector3(0, 0, 0));
        }

        for (int i = 0; i < indices.size(); i += 3) {
            Indices _indices;
            Vertices _vertices;
            Edges _edges;
            if (has_indices) {
                _indices.i1 = indices[i];
                _indices.i2 = indices[i + 1];
                _indices.i3 = indices[i + 2];
            } else {
                _indices.i1 = i;
                _indices.i2 = i + 1;
                _indices.i3 = i + 2;
            }

            _vertices.v1 = vertices[_indices.i1];
            _vertices.v2 = vertices[_indices.i2];
            _vertices.v3 = vertices[_indices.i3];

            _edges.e1 = _vertices.v2 - _vertices.v1;
            _edges.e2 = _vertices.v3 - _vertices.v1;

            const Vector3 normal = _edges.e1.cross(_edges.e2).normalized();

            normals[_indices.i1] += normal;
            normals[_indices.i2] += normal;
            normals[_indices.i3] += normal;
            for (Vector3 &_normal: normals) {
                _normal.normalize();
            }
        }

        return normals;
    }

    E3DParser::SubModelData E3DParser::_read_submodel(const Ref<FileAccess> &p_file, const int chunk_size) const {
        SubModelData result;
        result.next_idx = u32s(p_file->get_32());
        result.first_child_idx = u32s(p_file->get_32());
        result.type = p_file->get_32();
        result.name_idx = u32s(p_file->get_32());
        result.anim = u32s(p_file->get_32());
        result.flags = p_file->get_32() & 0xFFFF;
        result.matrix_idx = u32s(p_file->get_32());
        result.vertex_count = u32s(p_file->get_32());
        result.first_vertex_idx = u32s(p_file->get_32());
        result.material_idx = u32s(p_file->get_32());
        result.is_material_colored = result.material_idx == 0;
        result.lights_on_threshold = p_file->get_float();
        result.visibility_light_threshold = p_file->get_float();
        // ReSharper disable once CppExpressionWithoutSideEffects
        p_file->get_buffer(16); // skip unused RGBA ambient
        Color diffuse_color = Color(p_file->get_float(), p_file->get_float(), p_file->get_float(), 1.0);
        // ReSharper disable once CppExpressionWithoutSideEffects
        p_file->get_float(); // skip unused alpha
        // ReSharper disable once CppExpressionWithoutSideEffects
        p_file->get_buffer(16); // skip unused RGBA specular
        Color selfillum_color = Color(p_file->get_float(), p_file->get_float(), p_file->get_float(), 1.0);

        // ReSharper disable once CppExpressionWithoutSideEffects
        p_file->get_float(); // skip unused alpha
        if (const auto _transparent = result.flags & 32; _transparent == 0u) {
            selfillum_color.a = 1.0;
            diffuse_color.a = 1.0;
        }

        result.selfillum_color = selfillum_color;
        result.diffuse_color = diffuse_color;
        result.gl_lines_size = p_file->get_float();

        result.lod_max_distance = p_file->get_float();
        result.lod_min_distance = p_file->get_float();
        // ReSharper disable once CppExpressionWithoutSideEffects
        p_file->get_buffer(32); // skip attrs for lighting
        result.index_count = p_file->get_32();
        result.first_index_idx = p_file->get_32();
        result.transparent = result.flags & 0b000001;
        // ReSharper disable once CppExpressionWithoutSideEffects
        p_file->get_buffer(chunk_size - 164); // read to the end of the chunk
        result.vertices = PackedVector3Array();
        result.normals = PackedVector3Array();
        result.uvs = PackedVector2Array();
        result.indices = PackedInt32Array();
        return result;
    }

    std::vector<E3DParser::SubModelData> E3DParser::_parse_file(const Ref<FileAccess> &p_file) const {
        ChunkHeader chunk_header = _read_chunk_header(p_file);
        std::vector<SubModelData> submodels;
        TypedArray<String> submodel_names;
        TypedArray<String> material_names;
        TypedArray<PackedVector3Array> submodel_vertices;
        TypedArray<PackedVector3Array> submodel_normals;
        TypedArray<PackedVector2Array> submodel_uvs;
        TypedArray<PackedInt32Array> submodel_indices;
        TypedArray<PackedFloat64Array> submodel_tangents;
        TypedArray<Transform3D> matrices;

        while (!p_file->eof_reached()) {
            if (ChunkHeader chunk = _read_chunk_header(p_file); chunk.id == "SUB0") {
                const int submodels_count = static_cast<int>(chunk.data_len) / 256;
                for (int i = 0; i < submodels_count; i++) {
                    submodels.emplace_back(_read_submodel(p_file, 256));
                }
            } else if (chunk.id == "SUB1") {
                const int submodels_count = static_cast<int>(chunk.data_len) / 320;
                for (int i = 0; i < submodels_count; i++) {
                    submodels.emplace_back(_read_submodel(p_file, 320));
                }
            } else if (chunk.id == "NAM0") {
                submodel_names = _buffer_to_strings(p_file->get_buffer(chunk.data_len));
            } else if (chunk.id == "TEX0") {
                material_names = _buffer_to_strings(p_file->get_buffer(chunk.data_len));
            } else if (chunk.id == "TRA0") {
                const int matrix_count = static_cast<int>(chunk.data_len) / 64;
                for (int i = 0; i < matrix_count; i++) {
                    matrices.append(_read_matrix(p_file));
                }
            } else if (chunk.id == "IDX1") {
                uint64_t pos = p_file->get_position();
                for (SubModelData submodel : submodels) {
                    p_file->seek(pos + submodel.first_index_idx);
                    PackedInt32Array indices;
                    for (int j = 0; j < submodel.index_count; j++) {
                        indices.append(p_file->get_8());
                    }

                    submodel.indices.append_array(indices); //Experimental
                }

                p_file->seek(pos + chunk.data_len);
            } else if (chunk.id == "IDX2") {
                uint64_t pos = p_file->get_position();
                for (SubModelData submodel : submodels) {
                    p_file->seek(pos + (static_cast<uint64_t>(submodel.first_index_idx) * 2));

                    PackedInt32Array indices;

                    for (int j = 0; j < submodel.index_count; j++) {
                        indices.append(p_file->get_16());
                    }

                    submodel.indices.append_array(indices); //Experimental
                }

                p_file->seek(pos + chunk.data_len);
            } else if (chunk.id == "IDX4") {
                uint64_t pos = p_file->get_position();
                for (SubModelData submodel : submodels) {
                    p_file->seek(pos + (static_cast<uint64_t>(submodel.first_index_idx) * 4));

                    PackedInt32Array indices;

                    for (int j = 0; j < submodel.index_count; j++) {
                        indices.append(p_file->get_32());
                    }

                    submodel.indices.append_array(indices); //Experimental
                }

                p_file->seek(pos + chunk.data_len);
            } else if (chunk.id == "VNT2") {
                uint64_t pos = p_file->get_position();
                for (const SubModelData& submodel : submodels) {
                    p_file->seek(pos + (static_cast<uint64_t>(submodel.first_index_idx) * 48));

                    PackedVector3Array vertices;
                    PackedVector3Array normals;
                    PackedVector2Array uvs;
                    PackedFloat64Array tangents;

                    Array bv;
                    Array bn;
                    Array bt;
                    Array bu;

                    for (int j = 0; j < submodel.vertex_count; ++j) {
                        float x = p_file->get_float();
                        float y = p_file->get_float();
                        float z = p_file->get_float();
                        float nx = p_file->get_float();
                        float ny = p_file->get_float();
                        float nz = p_file->get_float();
                        float u = p_file->get_float();
                        float v = p_file->get_float();

                        float tx = p_file->get_float();
                        float ty = p_file->get_float();
                        float tz = p_file->get_float();
                        float tw = p_file->get_float();

                        Vector3 vertice(x, y, z);
                        Vector3 normal(nx, ny, nz);
                        Vector2 uv(u, v);

                        vertices.append(vertice);
                        normals.append(normal);
                        uvs.append(uv);
                        tangents.append_array({tx, ty, tz, tw});
                    }
                    submodel_vertices.push_back(vertices);
                    submodel_normals.append(normals);
                    submodel_uvs.append(uvs);
                    submodel_tangents.append(tangents);
                }
                p_file->seek(pos + chunk.data_len);
            } else {
                if (!chunk.id.is_empty()) {
                    UtilityFunctions::push_warning("Skipping unsupported chunk: " + chunk.id);
                }

                if (chunk.data_len > 0) {
                    // ReSharper disable once CppExpressionWithoutSideEffects
                    p_file->get_buffer(chunk.data_len);
                }
            }
        }

        for (int i = 0; i < submodels.size(); i++) {
            SubModelData submodel = submodels[i];

            if (!submodel.indices.is_empty()) {
                submodel.indices = submodel_indices[i];
            } else {
                submodel.indices.clear();
            }

            submodel.vertices = submodel_vertices[i];
            submodel.normals = submodel_normals[i];
            submodel.tangents = submodel_tangents[i];
            submodel.uvs = submodel_uvs[i];
            submodel.name = submodel_names[submodel.name_idx];
            submodel.material = material_names[submodel.material_idx];
            submodel.matrix = matrices[submodel.matrix_idx];
        }

        return submodels;
    }

    Array E3DParser::_buffer_to_strings(const PackedByteArray& p_buffer) {
        Array output;
        Array tmp;
        for (const unsigned char i : p_buffer) {
            if (i == 0) {
                output.append(String("").join(tmp));
                tmp.clear();
            } else {
                tmp.append(String::chr(i));
            }
        }

        return output;
    }

    Transform3D E3DParser::_read_matrix(const Ref<FileAccess> &p_file) {
        Array o;
        for (int row = 0; row < 16; row++) {
            o.append(p_file->get_float());
        }

        return Transform3D(
                Basis(
                        Vector3(o[0], o[1], o[2]),
                        Vector3(o[4], o[5], o[6]),
                        Vector3(o[8], o[9], o[10])
                        ),
                Vector3(o[12], o[13], o[14])
                );
    }

    E3DSubModel E3DParser::_create_submodel(SubModelData &p_submodel) {
        E3DSubModel *submodel = memnew(E3DSubModel);

        const std::unordered_map<int, E3DSubModel::SubModelType> typeMap = {
            {0, E3DSubModel::SubModelType::GL_POINTS},
            {1, E3DSubModel::SubModelType::GL_LINES},
            {2, E3DSubModel::SubModelType::GL_LINE_STRIP},
            {3, E3DSubModel::SubModelType::GL_LINE_LOOP},
            {4, E3DSubModel::SubModelType::GL_TRIANGLES},
            {5, E3DSubModel::SubModelType::GL_TRIANGLE_STRIP},
            {6, E3DSubModel::SubModelType::GL_TRIANGLE_FAN},
            {7, E3DSubModel::SubModelType::GL_QUADS},
            {8, E3DSubModel::SubModelType::GL_QUAD_STRIP},
            {9, E3DSubModel::SubModelType::GL_POLYGON},
            {256, E3DSubModel::SubModelType::TRANSFORM},
            {257, E3DSubModel::SubModelType::FREE_SPOTLIGHT},
            {258, E3DSubModel::SubModelType::STARS}
        };

        const std::unordered_map<int, E3DSubModel::AnimationType> animMap = {
            {1, E3DSubModel::AnimationType::NONE},
            {2, E3DSubModel::AnimationType::ROTATE_VEC},
            {3, E3DSubModel::AnimationType::ROTATE_XYZ},
            {4, E3DSubModel::AnimationType::MOVE},
            {5, E3DSubModel::AnimationType::JUMP_SECONDS},
            {6, E3DSubModel::AnimationType::JUMP_MINUTES},
            {5, E3DSubModel::AnimationType::JUMP_HOURS},
            {6, E3DSubModel::AnimationType::JUMP_HOURS24},
            {7, E3DSubModel::AnimationType::SECONDS},
            {8, E3DSubModel::AnimationType::MINUTES},
            {9, E3DSubModel::AnimationType::HOURS},
            {10, E3DSubModel::AnimationType::HOURS24},
            {11, E3DSubModel::AnimationType::BILLBOARD},
            {12, E3DSubModel::AnimationType::WIND},
            {13, E3DSubModel::AnimationType::SKY},
            {14, E3DSubModel::AnimationType::DIGITAL},
            {15, E3DSubModel::AnimationType::DIGICLK},
            {16, E3DSubModel::AnimationType::UNDEFINED},
            {256, E3DSubModel::AnimationType::IK},
            {257, E3DSubModel::AnimationType::IK1},
            {258, E3DSubModel::AnimationType::IK2},
            {-1, E3DSubModel::AnimationType::UNKNOWN}
        };
        submodel->set_submodel_type(typeMap.find(static_cast<int>(p_submodel.type))->second);
        submodel->set_visible(true);
        submodel->set_skip_rendering(false);
        const String _submodel_name = submodel->get_name();
        if (!_submodel_name.is_empty()) {
            submodel->set_name(p_submodel.name);

            if (_submodel_name.begins_with("Light_On")) {
                submodel->set_visible(false);
            } else if (_submodel_name.to_lower().ends_with("_on")) {
                submodel->set_visible(false);
            } else if (_submodel_name.to_lower().ends_with("_xon")) {
                submodel->set_visible(false);
            } else if (_submodel_name == "cien") {
                submodel->set_visible(false);
                submodel->set_skip_rendering(true);
            }
        }

        switch (p_submodel.type) {
            case E3DSubModel::SubModelType::TRANSFORM:
                if (!_submodel_name) {
                    submodel->set_name("banan");
                }

                submodel->set_transform(p_submodel.matrix);
                return *submodel;
            case E3DSubModel::SubModelType::GL_TRIANGLES: {
                const int64_t vertices_count = p_submodel.vertices.size();
                const String _mat_name = p_submodel.material != "" ? p_submodel.material.split(":")[0] : "";
                submodel->set_animation(animMap.find(p_submodel.anim)->second);

                if (p_submodel.material_idx < 0) {
                    submodel->set_dynamic_material(true);
                    submodel->set_dynamic_material_index(abs(p_submodel.material_idx)-1);
                }

                submodel->set_material_name(_mat_name);
                submodel->set_material_transparent((p_submodel.flags & (1 << 5)) != 0 );
                submodel->set_material_colored(p_submodel.is_material_colored);
                submodel->set_visibility_range_begin(std::sqrt(p_submodel.lod_min_distance));
                submodel->set_visibility_range_end(std::sqrt(p_submodel.lod_max_distance));
                submodel->set_visibility_light(p_submodel.visibility_light_threshold);
                submodel->set_lights_on_threshold(p_submodel.lights_on_threshold);
                submodel->set_diffuse_color(p_submodel.diffuse_color);
                submodel->set_self_illumination(p_submodel.selfillum_color);

                if (vertices_count > 0) {
                    Ref<ArrayMesh> mesh;
                    mesh.instantiate();
                    Array triangles;
                    triangles.resize(ArrayMesh::ARRAY_MAX);
                    triangles[ArrayMesh::ARRAY_VERTEX] = p_submodel.vertices;
                    PackedInt32Array _indices = p_submodel.indices;
                    PackedInt32Array ccw_indices;
                    for (int i = 0; i < _indices.size(); i += 3) {
                        ccw_indices.append_array({_indices[i], _indices[i + 2], _indices[i + 1]});
                    }

                    p_submodel.indices = ccw_indices;
                    PackedVector3Array _normals = _calculate_normals(p_submodel.vertices, p_submodel.indices);
                    if (p_submodel.indices.size() > 0) {
                        triangles[ArrayMesh::ARRAY_INDEX] = p_submodel.indices;
                    }

                    if (p_submodel.normals.size() > 0) {
                        triangles[ArrayMesh::ARRAY_NORMAL] = p_submodel.normals;
                    }

                    if (p_submodel.tangents.size() > 0) {
                        triangles[ArrayMesh::ARRAY_TANGENT] = p_submodel.tangents;
                    }

                    triangles[ArrayMesh::ARRAY_TEX_UV] = p_submodel.uvs;
                    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, triangles);
                    submodel->set_mesh(mesh);
                }

                submodel->set_transform(p_submodel.matrix);
                return *submodel;
            }
            default:
                UtilityFunctions::push_error("FIXME! Unsupported submodel (name: " + p_submodel.name + ", type: " + p_submodel.type + ")");
                return {};
        }
    }

    Ref<E3DModel> E3DParser::parse(const Ref<FileAccess> &p_file) const {
        // Build a list of submodels first using a simple vector to avoid Variant conversions.
        std::vector<E3DSubModel> submodels;
        std::vector<SubModelData> submodels_meta = _parse_file(p_file);
        submodels.reserve(submodels_meta.size());

        for (SubModelData & i : submodels_meta) {
            submodels.push_back(_create_submodel(i));
        }

        // Apply parent/child relationships using references to actual stored elements
        for (size_t i = 0; i < submodels_meta.size(); i++) {
            const SubModelData &meta = submodels_meta[i];
            E3DSubModel &parent = submodels[i];

            if (meta.first_child_idx > -1 && static_cast<size_t>(meta.first_child_idx) < submodels.size()) {
                E3DSubModel &child = submodels[meta.first_child_idx];
                child.set_parent(&parent);
            }

            if (meta.next_idx > -1 && static_cast<size_t>(meta.next_idx) < submodels.size()) {
                E3DSubModel &next = submodels[meta.next_idx];
                next.set_parent(&parent);
            }
        }

        // Create the model and add only root-level submodels
        Ref<E3DModel> model;
        model.instantiate();
        if (!submodels.empty()) {
            model->add_child(submodels[0]);
            int next_idx = submodels_meta[0].next_idx;
            while (next_idx > -1 && static_cast<size_t>(next_idx) < submodels.size()) {
                model->add_child(submodels[next_idx]);
                next_idx = submodels_meta[next_idx].next_idx;
            }
        }

        return model;
    }
} // namespace godot
