@tool
extends RefCounted

func import(p:MaszynaParser, context: MaszynaImporterContext) -> MeshInstance3D:
    var material
    var texture

    var _n = p.next_token()
    match _n:
        "material":
            material = p.get_tokens_until("endmaterial")
            texture = p.next_token()
        _:
            texture = _n

    var data:Array = p.get_tokens_until("endtri")
    var mesh = ArrayMesh.new()
    var vertices = PackedVector3Array()

    var normals = PackedVector3Array()
    var uvs = PackedVector2Array()

    while data.size():
        var x = data.pop_front()
        var y = data.pop_front()
        var z = data.pop_front()
        var nx = data.pop_front()
        var ny = data.pop_front()
        var nz = data.pop_front()
        var u = data.pop_front()
        var v = data.pop_front()
        var _stop = data.pop_front()
        if not _stop in ["end", "endtri"]:
            push_error("!!! Incorrect triangle format ")
            return
        var new_vector = Vector3(float(x), float(y), float(z))
        #min_vertex = new_vector if new_vector < min_vertex else min_vertex
        #max_vertex = new_vector if new_vector > max_vertex else max_vertex
        vertices.append(new_vector)
        normals.append(Vector3(float(nx), float(ny), float(nz)))
        uvs.append(Vector2(float(u), float(v)))

    vertices.reverse()
    normals.reverse()
    uvs.reverse()

    var arrays: Array = []
    arrays.resize(Mesh.ARRAY_MAX)
    arrays[Mesh.ARRAY_VERTEX] = vertices
    arrays[Mesh.ARRAY_NORMAL] = normals
    arrays[Mesh.ARRAY_TEX_UV] = uvs

    mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)

    var obj := MeshInstance3D.new()
    obj.mesh = mesh
    if texture:
        var mat = MaterialManager.get_material("", texture)
        if mat:
            obj.material_override = mat
    
    return obj
