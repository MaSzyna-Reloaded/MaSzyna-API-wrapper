@tool
extends Node

const MAX_31B = 1 << 31
const MAX_32B = 1 << 32

func u32s(unsigned):
    return (unsigned + MAX_31B) % MAX_32B - MAX_31B

func _read_chunk_header(file):
    var chunk_id = file.get_buffer(4).get_string_from_ascii()
    var chunk_len = file.get_32()

    return {"id": chunk_id, "len": chunk_len, "data_len": chunk_len-8}

func unsigned32_to_signed(unsigned):
    return (unsigned + MAX_31B) % MAX_32B - MAX_31B


func _calculate_normals(vertices: Array, indices: Array) -> Array:
    var normals = []
    var has_indices = indices.size() > 0
    for i in range(vertices.size()):
        normals.append(Vector3(0, 0, 0))  # Initialize all normals to zero

    # Loop through each triangle in the mesh
    for i in range(0, indices.size(), 3):
        # Get the indices of the vertices forming the triangle

        var i1 = indices[i] if has_indices else i
        var i2 = indices[i + 1] if has_indices else i+1
        var i3 = indices[i + 2] if has_indices else i+2

        # Get the vertices of the triangle
        var v1 = vertices[i1]
        var v2 = vertices[i2]
        var v3 = vertices[i3]

        # Calculate the two edges of the triangle
        var edge1 = v2 - v1
        var edge2 = v3 - v1

        # Compute the face normal using the cross product
        var normal = edge1.cross(edge2).normalized() * -1

        # Accumulate the face normal into each vertex's normal
        normals[i1] += normal
        normals[i2] += normal
        normals[i3] += normal

    # Normalize all the accumulated vertex normals
    for i in range(normals.size()):
        normals[i] = normals[i].normalized()

    return normals


func _read_submodel(file, chunk_size: int) -> Dictionary:
    var result = {}
    result["next_idx"] = unsigned32_to_signed(file.get_32())
    result["first_child_idx"] = unsigned32_to_signed(file.get_32())
    result["type"] = file.get_32()
    result["name_idx"] = u32s(file.get_32())
    result["anim"] = unsigned32_to_signed(file.get_32())
    result["flags"] = file.get_32() & 0xFFFF
    result["matrix_idx"] = unsigned32_to_signed(file.get_32())
    result["vertex_count"] = u32s(file.get_32())
    result["first_vertex_idx"] = u32s(file.get_32())
    result["material_idx"] = unsigned32_to_signed(file.get_32())

    result["is_material_colored"] = true if result["material_idx"] == 0 else false
    result["lights_on_threshold"] = file.get_float()
    result["visibility_light_threshold"] = file.get_float()
    file.get_buffer(16)  # skip unused RGBA ambient
    result["diffuse_color"] = Color(
        file.get_float(), file.get_float(), file.get_float(), 1.0
    )
    file.get_float()  # unused alpha
    file.get_buffer(16)  # skip unused RGBA specular
    result["selfillum_color"] = Color(
        file.get_float(), file.get_float(), file.get_float(), 1.0
    )
    file.get_float()  # unused alpha
    var _transparent = result["flags"] & 32
    if not _transparent:
        result["selfillum_color"].a = 1.0
        result["diffuse_color"].a = 1.0

    result["gl_lines_size"] = file.get_float()

    result["lod_max_distance"] = file.get_float()
    result["lod_min_distance"] = file.get_float()
    file.get_buffer(32) # skip attrs for light
    result["index_count"] = file.get_32()
    result["first_index_idx"] = file.get_32()
    result["transparent"] = result["flags"] & 0b000001
    file.get_buffer(chunk_size-164)  # read to the end of the chunk
    result["vertices"] = PackedVector3Array()
    result["normals"] = PackedVector3Array()
    result["uv"] = PackedVector2Array()
    result["indices"] = PackedInt32Array()
    return result


func _read_string0(file: FileAccess):
    var output = []
    while true:
        var c = file.get_8()
        if c == 0:
            break
        output.append(char(c))
    return "".join(output)


func _buffer_to_strings(buf) -> Array:
    var output = []
    var tmp = []
    for i in range(0, buf.size(), 1):
        var c = buf[i]
        if c == 0:
            output.append("".join(tmp))
            tmp = []
        else:
            tmp.append(char(c))
    return output

func _read_matrix(file: FileAccess):
    var o = []
    for i in range(0, 16):
        o.append(file.get_float())

    return Transform3D(
        Basis(
            Vector3(o[0], o[1], o[2]),
            Vector3(o[4], o[5], o[6]),
            Vector3(o[8], o[9], o[10])
        ),
        Vector3(o[12], o[13], o[14])
    )


func _parse_file(file: FileAccess, options:Dictionary):
    var e3d0 = _read_chunk_header(file)
    var submodels = []
    var submodel_names = []
    var material_names = []
    var submodel_vertices = []
    var submodel_normals = []
    var submodel_uvs = []
    var submodel_indices = []
    var submodel_tangents = []
    var matrices = []

    while not file.eof_reached():
        var chunk = _read_chunk_header(file)
        match chunk.id:
            "SUB0":
                var submodels_count = chunk.data_len / 256
                for i in range(0, submodels_count, 1):
                    var sm = _read_submodel(file, 256)
                    submodels.append(sm)
            "SUB1":
                var submodels_count = chunk.data_len / 320
                for i in range(0, submodels_count, 1):
                    var sm = _read_submodel(file, 320)
                    submodels.append(sm)
            "NAM0":
                submodel_names = _buffer_to_strings(file.get_buffer(chunk.data_len))
            "TEX0":
                material_names = _buffer_to_strings(file.get_buffer(chunk.data_len))
            "TRA0":
                var matrix_count = chunk.data_len / 64
                for i in range(0, matrix_count, 1):
                    matrices.append(_read_matrix(file))
            "IDX1":
                var _pos = file.get_position()
                for submodel in submodels:
                    file.seek(_pos + submodel["first_index_idx"])
                    var indices = PackedInt32Array()
                    var ccw_index = PackedInt32Array()
                    for i in range(0, submodel["index_count"]):
                        indices.append(file.get_8())
                    for i in range(0, indices.size(), 3):
                        ccw_index.append_array([indices[i], indices[i+2], indices[i+1]])
                    submodel_indices.append(ccw_index)
                file.seek(chunk.data_len + _pos)
            "IDX2":
                var _pos = file.get_position()
                for submodel in submodels:
                    file.seek(_pos + submodel["first_index_idx"] * 2)
                    var indices = PackedInt32Array()
                    var ccw_index = PackedInt32Array()
                    for i in range(0, submodel["index_count"]):
                        indices.append(file.get_16())
                    for i in range(0, indices.size(), 3):
                        ccw_index.append_array([indices[i], indices[i+2], indices[i+1]])
                    submodel_indices.append(ccw_index)
                file.seek(chunk.data_len + _pos)
            "IDX4":
                var _pos = file.get_position()
                for submodel in submodels:
                    file.seek(_pos + submodel["first_index_idx"] * 4)
                    var indices = PackedInt32Array()
                    var ccw_index = PackedInt32Array()
                    for i in range(0, submodel["index_count"]):
                        indices.append(file.get_32())
                    for i in range(0, indices.size(), 3):
                        ccw_index.append_array([indices[i], indices[i+2], indices[i+1]])
                    submodel_indices.append(ccw_index)
                file.seek(chunk.data_len + _pos)
            "VNT2":
                # 8 x 2 bytes
                var _pos = file.get_position()
                for submodel in submodels:
                    file.seek(_pos + submodel["first_vertex_idx"] * 48)

                    var vertices = PackedVector3Array()
                    var normals = PackedVector3Array()
                    var uvs = PackedVector2Array()
                    var tangents = PackedFloat64Array()

                    var bv = []
                    var bn = []
                    var bt = []
                    var bu = []
                    for i in range(0, submodel["vertex_count"]):
                        var x = file.get_float()
                        var y = file.get_float()
                        var z = file.get_float()
                        var nx = file.get_float()
                        var nz = file.get_float()
                        var ny = file.get_float()
                        var u = file.get_float()
                        var v = file.get_float()
                        var tx = file.get_float()
                        var ty = file.get_float()
                        var tz = file.get_float()
                        var tw = file.get_float()
                        var _vec = Vector3(x,y,z)
                        var _norm = Vector3(nx, ny, nz)
                        var _uv = Vector2(u, v)

                        vertices.append(_vec)
                        normals.append(_norm)
                        uvs.append(_uv)
                        tangents.append_array([tx, ty, tz, tw])

                    submodel_vertices.append(vertices)
                    submodel_normals.append(normals)
                    submodel_uvs.append(uvs)
                    submodel_tangents.append(tangents)

                file.seek(chunk.data_len + _pos)
            _:
                if chunk.id:
                    push_warning("Skipping unsupported chunk: " + chunk.id)
                if chunk.data_len > 0:
                    file.get_buffer(chunk.data_len)

    var i = 0

    for submodel in submodels:
        var name_idx = submodel["name_idx"]
        var tex_idx = submodel["material_idx"]
        var mtx_idx = submodel["matrix_idx"]
        if submodel_indices.size():
            submodel["indices"] = submodel_indices[i]
        else:
            submodel["indices"] = []

        submodel["vertices"] = submodel_vertices[i]
        submodel["normals"] = submodel_normals[i]
        submodel["tangents"] = submodel_tangents[i]
        submodel["uvs"] = submodel_uvs[i]
        submodel["name"] = submodel_names[name_idx] if name_idx > -1 else null
        submodel["material"] = material_names[tex_idx] if tex_idx > -1 else null
        submodel["matrix"] = matrices[mtx_idx] if mtx_idx > -1 else null
        i += 1
    return submodels



func _create_submodel(root, parent, submodel):
    var obj = E3DSubModel.new()
    obj.submodel_type = submodel["type"]
    parent.add_child(obj)  # root dla banana?

    if submodel.get("name"):
        obj.name = submodel["name"]

    match submodel["type"]:
        E3DSubModel.SubModelType.TRANSFORM:
            if not obj.name:
                obj.name = "banan"
            obj.transform = submodel["matrix"]

        E3DSubModel.SubModelType.GL_TRIANGLES:
            var _vc = submodel["vertices"].size()
            var _nc = submodel["normals"].size()
            var _uc = submodel["uvs"].size()
            var _ic = submodel["indices"].size()
            var _tc = submodel["tangents"].size() / 4


            var _matname = submodel["material"].split(":")[0] if submodel["material"] else ""
            var _ismatabs = _matname.begins_with("/")

            obj.animation = submodel["anim"]

            if submodel["material_idx"] < 0:
                obj.dynamic_material = true
                obj.dynamic_material_index = abs(submodel["material_idx"])-1

            obj.material_name = _matname if _matname else ""
            obj.material_transparent = submodel["flags"] & 32
            obj.material_colored = submodel["is_material_colored"]
            obj.visibility_range_begin = sqrt(submodel["lod_min_distance"])
            obj.visibility_range_end = sqrt(submodel["lod_max_distance"])

            obj.visibility_light = submodel["visibility_light_threshold"]
            obj.lights_on_threshold = submodel["lights_on_threshold"]
            obj.diffuse_color = submodel["diffuse_color"]
            obj.self_illumination = submodel["selfillum_color"]

            var mesh = ArrayMesh.new()
            var triangles = []
            triangles.resize(ArrayMesh.ARRAY_MAX)
            triangles[ArrayMesh.ARRAY_VERTEX] = submodel["vertices"]

            var _normals = _calculate_normals(submodel["vertices"], submodel["indices"])
            triangles[ArrayMesh.ARRAY_NORMAL] = PackedVector3Array(_normals)
            triangles[ArrayMesh.ARRAY_TANGENT] = submodel["tangents"]
            triangles[ArrayMesh.ARRAY_TEX_UV] = submodel["uvs"]
            triangles[ArrayMesh.ARRAY_INDEX] = submodel["indices"]

            mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, triangles)
            mesh.regen_normal_maps()

            obj.mesh = mesh
            obj.transform = submodel["matrix"]


            return obj
        _:
            push_error("FIXME! Unsupported submodel type=%s name=%s parent=%s root=%s" % [
                submodel["type"],
                submodel["name"],
                parent.name,
                root.name,
            ])


func _add_submodels(root, parent, submodel, submodels):
    var created = _create_submodel(root, parent, submodel)
    var created_submodel = submodel
    if submodel["next_idx"] > -1:
        submodel = submodels[submodel["next_idx"]]
        _add_submodels(root, parent, submodel, submodels)

    var child = submodels[created_submodel["first_child_idx"]] if created_submodel["first_child_idx"] > -1 else null

    if child:
        _add_submodels(root, created if created else parent, child, submodels)

func parse(file:FileAccess, options={}) -> E3DModel:
    var node = E3DModel.new()
    var submodels = _parse_file(file, options)
    var parent = node

    _add_submodels(node, node, submodels[0], submodels)
    return node
