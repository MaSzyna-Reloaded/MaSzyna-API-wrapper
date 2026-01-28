#include "parsers/e3d_parser.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <cmath>
#include <map>
#include <vector>

namespace godot {
    void E3DParser::_bind_methods() {
        ClassDB::bind_method(D_METHOD("parse"), &E3DParser::parse);
    }

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

    PackedVector3Array E3DParser::_calculate_normals(const PackedVector3Array &vertices, const PackedInt32Array &indices) {
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
                _indices.i1 = indices.get(i);
                _indices.i2 = indices.get(i + 1);
                _indices.i3 = indices.get(i + 2);
            } else {
                _indices.i1 = i;
                _indices.i2 = i + 1;
                _indices.i3 = i + 2;
            }

            _vertices.v1 = vertices.get(_indices.i1);
            _vertices.v2 = vertices.get(_indices.i2);
            _vertices.v3 = vertices.get(_indices.i3);

            _edges.e1 = _vertices.v2 - _vertices.v1;
            _edges.e2 = _vertices.v3 - _vertices.v1;

            const Vector3 normal = _edges.e1.cross(_edges.e2).normalized();

            normals.set(_indices.i1, normals.get(_indices.i1) + normal);
            normals.set(_indices.i2, normals.get(_indices.i2) + normal);
            normals.set(_indices.i3, normals.get(_indices.i3) + normal);
        }

        for (int i = 0; i < normals.size(); i++) {
            normals.set(i, normals.get(i).normalized());
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
        result.is_material_colored = (result.material_idx == 0);
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
                for (SubModelData &submodel: submodels) {
                    if (submodel.index_count <= 0) continue;
                    p_file->seek(pos + submodel.first_index_idx);
                    PackedInt32Array indices;
                    for (int j = 0; j < submodel.index_count; j++) {
                        indices.append(p_file->get_8());
                    }

                    submodel.indices.append_array(indices); // Experimental
                }

                p_file->seek(pos + chunk.data_len);
            } else if (chunk.id == "IDX2") {
                uint64_t pos = p_file->get_position();
                for (SubModelData &submodel: submodels) {
                    if (submodel.index_count <= 0) continue;
                    p_file->seek(pos + (static_cast<uint64_t>(submodel.first_index_idx) * 2));

                    PackedInt32Array indices;

                    for (int j = 0; j < submodel.index_count; j++) {
                        indices.append(p_file->get_16());
                    }

                    submodel.indices.append_array(indices); // Experimental
                }

                p_file->seek(pos + chunk.data_len);
            } else if (chunk.id == "IDX4") {
                uint64_t pos = p_file->get_position();
                for (SubModelData &submodel: submodels) {
                    if (submodel.index_count <= 0) continue;
                    p_file->seek(pos + (static_cast<uint64_t>(submodel.first_index_idx) * 4));

                    PackedInt32Array indices;

                    for (int j = 0; j < submodel.index_count; j++) {
                        indices.append(p_file->get_32());
                    }

                    submodel.indices.append_array(indices); // Experimental
                }

                p_file->seek(pos + chunk.data_len);
            } else if (chunk.id == "VNT2") {
                uint64_t pos = p_file->get_position();
                for (SubModelData &submodel: submodels) {
                    if (submodel.vertex_count <= 0) continue;
                    p_file->seek(pos + (static_cast<uint64_t>(submodel.first_vertex_idx) * 48));

                    PackedVector3Array vertices;
                    PackedVector3Array normals;
                    PackedVector2Array uvs;
                    PackedFloat64Array tangents;

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
                        tangents.push_back(tx);
                        tangents.push_back(ty);
                        tangents.push_back(tz);
                        tangents.push_back(tw);
                    }
                    submodel.vertices = vertices;
                    submodel.normals = normals;
                    submodel.uvs = uvs;
                    submodel.tangents = tangents;
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
            SubModelData &submodel = submodels.at(i); //might be better to use operator[]

            if (submodel.name_idx >= 0 && submodel.name_idx < submodel_names.size()) {
                submodel.name = String(submodel_names.get(submodel.name_idx));
            }

            if (submodel.material_idx >= 0 && submodel.material_idx < material_names.size()) {
                submodel.material = String(material_names.get(submodel.material_idx));
            }

            if (submodel.matrix_idx >= 0 && submodel.matrix_idx < matrices.size()) {
                submodel.matrix = Transform3D(matrices.get(submodel.matrix_idx));
            }
        }

        return submodels;
    }

    TypedArray<String> E3DParser::_buffer_to_strings(const PackedByteArray &p_buffer) {
        TypedArray<String> output;
        PackedStringArray tmp;
        for (const unsigned char i: p_buffer) {
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
        float m[16];
        for (float &row : m) {
            row = p_file->get_float();
        }

        return Transform3D(
                Basis(
                        m[0], m[8], -m[4],
                        m[2], m[10], -m[6],
                        -m[1], -m[9], m[5]),
                Vector3(m[12], m[14], -m[13]));
    }

    Ref<E3DSubModel> E3DParser::_create_submodel(SubModelData &p_submodel) {
        UtilityFunctions::print_verbose("[E3DParser] Creating submodel from data: " + p_submodel.name);
        Ref<E3DSubModel> submodel;
        submodel.instantiate();

        const std::unordered_map<int, E3DSubModel::SubModelType> typeMap = {
                {0, E3DSubModel::SubModelType::GL_POINTS},       {1, E3DSubModel::SubModelType::GL_LINES},
                {2, E3DSubModel::SubModelType::GL_LINE_STRIP},   {3, E3DSubModel::SubModelType::GL_LINE_LOOP},
                {4, E3DSubModel::SubModelType::GL_TRIANGLES},    {5, E3DSubModel::SubModelType::GL_TRIANGLE_STRIP},
                {6, E3DSubModel::SubModelType::GL_TRIANGLE_FAN}, {7, E3DSubModel::SubModelType::GL_QUADS},
                {8, E3DSubModel::SubModelType::GL_QUAD_STRIP},   {9, E3DSubModel::SubModelType::GL_POLYGON},
                {256, E3DSubModel::SubModelType::TRANSFORM},     {257, E3DSubModel::SubModelType::FREE_SPOTLIGHT},
                {258, E3DSubModel::SubModelType::STARS}};

        const std::unordered_map<int, E3DSubModel::AnimationType> animMap = {
                {1, E3DSubModel::AnimationType::NONE},         {2, E3DSubModel::AnimationType::ROTATE_VEC},
                {3, E3DSubModel::AnimationType::ROTATE_XYZ},   {4, E3DSubModel::AnimationType::MOVE},
                {5, E3DSubModel::AnimationType::JUMP_SECONDS}, {6, E3DSubModel::AnimationType::JUMP_MINUTES},
                {5, E3DSubModel::AnimationType::JUMP_HOURS},   {6, E3DSubModel::AnimationType::JUMP_HOURS24},
                {7, E3DSubModel::AnimationType::SECONDS},      {8, E3DSubModel::AnimationType::MINUTES},
                {9, E3DSubModel::AnimationType::HOURS},        {10, E3DSubModel::AnimationType::HOURS24},
                {11, E3DSubModel::AnimationType::BILLBOARD},   {12, E3DSubModel::AnimationType::WIND},
                {13, E3DSubModel::AnimationType::SKY},         {14, E3DSubModel::AnimationType::DIGITAL},
                {15, E3DSubModel::AnimationType::DIGICLK},     {16, E3DSubModel::AnimationType::UNDEFINED},
                {256, E3DSubModel::AnimationType::IK},         {257, E3DSubModel::AnimationType::IK1},
                {258, E3DSubModel::AnimationType::IK2},        {-1, E3DSubModel::AnimationType::UNKNOWN}};

        if (const std::unordered_map<int, E3DSubModel::SubModelType>::const_iterator type_it =
                    typeMap.find(static_cast<int>(p_submodel.type)); type_it != typeMap.end()) {
             submodel->set_submodel_type(type_it->second);
        } else {
             UtilityFunctions::push_warning("Unknown submodel type: " + String::num_int64(p_submodel.type));
             submodel->set_submodel_type(E3DSubModel::SubModelType::GL_TRIANGLES);
        }

        submodel->set_visible(true);
        submodel->set_skip_rendering(false);
        const String _submodel_name = p_submodel.name;
        if (!_submodel_name.is_empty()) {
            submodel->set_name(_submodel_name);

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
                return submodel;
            case E3DSubModel::SubModelType::GL_TRIANGLES: {
                const int64_t vertices_count = p_submodel.vertices.size();
                const String _mat_name = p_submodel.material != "" ? p_submodel.material.split(":").get(0) : "";
                if (const std::unordered_map<int, E3DSubModel::AnimationType>::const_iterator anim_it =
                            animMap.find(p_submodel.anim); anim_it != animMap.end()) {
                    submodel->set_animation(anim_it->second);
                } else {
                    submodel->set_animation(E3DSubModel::AnimationType::NONE);
                }

                if (p_submodel.material_idx < 0) {
                    submodel->set_dynamic_material(true);
                    submodel->set_dynamic_material_index(abs(p_submodel.material_idx) - 1);
                }

                submodel->set_material_name(_mat_name);
                submodel->set_material_transparent((p_submodel.flags & (1 << 5)) != 0);
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
                    triangles.set(ArrayMesh::ARRAY_VERTEX, p_submodel.vertices);
                    const PackedInt32Array _indices = p_submodel.indices;
                    PackedInt32Array ccw_indices;

                    if (!_indices.is_empty()) {
                        if (p_submodel.type == E3DSubModel::SubModelType::GL_TRIANGLES) {
                            for (int i = 0; i < _indices.size(); i += 3) {
                                auto _i1 = static_cast<int32_t>(_indices.get(i));
                                auto _i2 = static_cast<int32_t>(_indices.get(i + 1));
                                auto _i3 = static_cast<int32_t>(_indices.get(i + 2));
                                ccw_indices.append_array(PackedInt32Array({_i1, _i3, _i2}));
                            }
                        } else if (p_submodel.type == E3DSubModel::SubModelType::GL_QUADS) {
                            for (int i = 0; i < _indices.size(); i += 4) {
                                auto _i1 = static_cast<int32_t>(_indices.get(i));
                                auto _i2 = static_cast<int32_t>(_indices.get(i + 1));
                                auto _i3 = static_cast<int32_t>(_indices.get(i + 2));
                                auto _i4 = static_cast<int32_t>(_indices.get(i + 3));
                                // Quad (1, 2, 3, 4) -> Tri1 (1, 3, 2), Tri2 (1, 4, 3) (CCW)
                                ccw_indices.append_array(PackedInt32Array({_i1, _i3, _i2, _i1, _i4, _i3}));
                            }
                        }
                    } else {
                        // Unindexed mesh: generate indices to allow CW -> CCW conversion
                        if (p_submodel.type == E3DSubModel::SubModelType::GL_TRIANGLES) {
                            for (int i = 0; i < vertices_count; i += 3) {
                                auto _i1 = static_cast<int32_t>(i);
                                auto _i2 = static_cast<int32_t>(i + 1);
                                auto _i3 = static_cast<int32_t>(i + 2);
                                ccw_indices.append_array(PackedInt32Array({_i1, _i3, _i2}));
                            }
                        } else if (p_submodel.type == E3DSubModel::SubModelType::GL_QUADS) {
                            for (int i = 0; i < vertices_count; i += 4) {
                                auto _i1 = static_cast<int32_t>(i);
                                auto _i2 = static_cast<int32_t>(i + 1);
                                auto _i3 = static_cast<int32_t>(i + 2);
                                auto _i4 = static_cast<int32_t>(i + 3);
                                ccw_indices.append_array(PackedInt32Array({_i1, _i3, _i2, _i1, _i4, _i3}));
                            }
                        }
                    }

                    p_submodel.indices = ccw_indices;
                    if (p_submodel.normals.is_empty()) {
                        p_submodel.normals = _calculate_normals(p_submodel.vertices, p_submodel.indices);
                    }

                    if (p_submodel.indices.size() > 0) {
                        triangles.set(ArrayMesh::ARRAY_INDEX, p_submodel.indices);
                    }

                    if (p_submodel.normals.size() > 0) {
                        triangles.set(ArrayMesh::ARRAY_NORMAL, p_submodel.normals);
                    }

                    if (p_submodel.tangents.size() > 0) {
                        triangles.set(ArrayMesh::ARRAY_TANGENT, p_submodel.tangents);
                    }

                    triangles.set(ArrayMesh::ARRAY_TEX_UV, p_submodel.uvs);
                    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, triangles);
                    submodel->set_mesh(mesh);
                }

                submodel->set_transform(p_submodel.matrix);
                return submodel;
            }
            default:
                UtilityFunctions::push_error(
                        "Unsupported submodel (name: " + p_submodel.name + ", type: " + String::num(p_submodel.type) + ")"); //@TODO Display type name based on p_sumbodel.type
                return submodel;
        }
    }

    Ref<E3DModel> E3DParser::parse(const Ref<FileAccess> &p_file) const {
        UtilityFunctions::print_verbose("[E3DParser] Parsing " + p_file->get_path());
        // Build a list of submodels first using a simple vector to avoid Variant conversions.
        std::vector<Ref<E3DSubModel>> submodels;
        std::vector<SubModelData> submodels_meta = _parse_file(p_file);
        submodels.reserve(submodels_meta.size());

        for (SubModelData &i: submodels_meta) {
            submodels.push_back(_create_submodel(i));
        }

        // Track parentage to avoid duplication in E3DModel
        std::vector<bool> has_parent(submodels.size(), false);

        // Apply parent/child relationships using references to actual stored elements
        for (size_t i = 0; i < submodels_meta.size(); i++) {
            const SubModelData &meta = submodels_meta.at(i);
            const Ref<E3DSubModel>& parent = submodels.at(i);

            if (meta.first_child_idx > -1 && static_cast<size_t>(meta.first_child_idx) < submodels.size()) {
                int child_idx = meta.first_child_idx;
                while (child_idx > -1 && static_cast<size_t>(child_idx) < submodels.size()) {
                    const Ref<E3DSubModel>& child = submodels.at(child_idx);
                    child->set_parent(parent.ptr());
                    if (child_idx >= 0 && static_cast<size_t>(child_idx) < has_parent.size()) {
                        has_parent[child_idx] = true;
                    }
                    child_idx = submodels_meta.at(child_idx).next_idx;
                }
            }
        }

        // Create the model and add only root-level submodels
        Ref<E3DModel> model;
        UtilityFunctions::print_verbose("[E3DParser] Creating model instance for " + p_file->get_path());
        model.instantiate();
        for (size_t i = 0; i < submodels.size(); i++) {
            if (!has_parent.at(i)) {
                model->add_child(*submodels.at(i));
            }
        }

        return model;
    }
} // namespace godot
