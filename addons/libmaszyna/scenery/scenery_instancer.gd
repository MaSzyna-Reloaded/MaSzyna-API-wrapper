@tool
extends Node

static var sky_importer = preload("res://addons/libmaszyna/importer/maszyna_sky_importer.gd").new()
static var atmo_importer = preload("res://addons/libmaszyna/importer/maszyna_sky_importer.gd").new()
static var node_importer = preload("res://addons/libmaszyna/importer/maszyna_node_importer.gd").new()
static var event_importer = preload("res://addons/libmaszyna/importer/maszyna_event_importer.gd").new()
static var origin_importer = preload("res://addons/libmaszyna/importer/maszyna_origin_importer.gd").new()
static var endorigin_importer = preload("res://addons/libmaszyna/importer/maszyna_endorigin_importer.gd").new()
static var rotate_importer = preload("res://addons/libmaszyna/importer/maszyna_rotate_importer.gd").new()
static var terrain_importer = preload("res://addons/libmaszyna/importer/maszyna_terrain_importer.gd").new()
static var include_importer = preload("res://addons/libmaszyna/importer/maszyna_include_importer.gd").new()
static var trainset_importer = preload("res://addons/libmaszyna/importer/maszyna_trainset_importer.gd").new()
static var firstinit_importer = preload("res://addons/libmaszyna/importer/maszyna_firstinit_importer.gd").new()


func instantiate(root: MaszynaIncludeNode, parameters: Dictionary = {}) -> void:
    for child in root.get_children(true):
        root.remove_child(child)

    var context := MaszynaImporterContext.new()
    context.rotate = root.context_rotate
    context.origin = root.context_origin
    var abs_file = UserSettings.get_maszyna_game_dir().path_join("scenery").path_join(root.filename)

    var file = FileAccess.open(abs_file, FileAccess.READ)
    if not file:
        push_error("Cannot load scenery: "+abs_file)
        return

    var parser := MaszynaParser.new()
    parser.set_parameters(parameters)
    
    parser.initialize(file.get_buffer(file.get_length()))
    parser.register_handler("sky", _make_importer_callback(sky_importer, context))
    parser.register_handler("atmo", _make_importer_callback(atmo_importer, context))
    parser.register_handler("node", _make_importer_callback(node_importer, context))
    parser.register_handler("event", _make_importer_callback(event_importer, context))
    parser.register_handler("origin", _make_importer_callback(origin_importer, context))
    parser.register_handler("endorigin", _make_importer_callback(endorigin_importer, context))
    parser.register_handler("rotate", _make_importer_callback(rotate_importer, context))
    parser.register_handler("terrain", _make_importer_callback(terrain_importer, context))
    parser.register_handler("include", _make_importer_callback(include_importer, context))
    parser.register_handler("trainset", _make_importer_callback(trainset_importer, context))
    parser.register_handler("firstinit", _make_importer_callback(firstinit_importer, context))

    var objects = parser.parse()
    var metadata = parser.get_parsed_metadata()
    
    
    # Clear handlers manually to break reference cycles
    for token in ["sky", "atmo", "node", "event", "origin", "endorigin", "rotate", "terrain", "include", "trainset", "firstinit"]:
        parser.unregister_handler(token)
    parser.unreference()
    
    #root.category = _get_meta_value(metadata, "l")
    #root.title = _get_meta_value(metadata, "n")
    #root.description = _get_meta_value(metadata, "d")
    
# create tracks
    var tracks = Node3D.new()
    tracks.name = "Tracks"
    for item in context.tracks:
        tracks.add_child(item)
    if tracks.get_child_count() > 0:
        objects.insert(0, tracks)

# create traction    
    var traction = Node3D.new()
    traction.name = "Traction"
    for item in context.traction:
        traction.add_child(item)
    if traction.get_child_count() > 0:
        objects.insert(0, traction)

# create meshinstances for triangles grouped by material

    var triangles_by_material = {}
    for triangle in context.triangles:
        # [texture, vertices, normals, uvs]
        var texture = triangle[0]
        if not texture in triangles_by_material:
            triangles_by_material[texture] = []
        triangles_by_material[texture].append(triangle)
    
    for texture in triangles_by_material.keys():
        var node = MeshInstance3D.new()
        var mesh = ArrayMesh.new()
        var arrays: Array = []
        arrays.resize(Mesh.ARRAY_MAX)
        arrays[Mesh.ARRAY_VERTEX] = PackedVector3Array()
        arrays[Mesh.ARRAY_NORMAL] = PackedVector3Array()
        arrays[Mesh.ARRAY_TEX_UV] = PackedVector2Array()
        for triangles in triangles_by_material[texture]:
            arrays[Mesh.ARRAY_VERTEX].append_array(triangles[1])
            arrays[Mesh.ARRAY_NORMAL].append_array(triangles[2])
            arrays[Mesh.ARRAY_TEX_UV].append_array(triangles[3])
        mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
        node.mesh = mesh
        node.name = texture
        var material = MaterialManager.get_material("", texture)
        if material:
            node.material_override = material
        objects.append(node)
    
    
    for obj in objects:
        if not obj:
            continue

        if obj is Node:
            root.add_child(obj)
            _set_owner_recursive(obj, root)


func _make_importer_callback(importer, context) -> Callable:
    var callback = func(p): return importer.import(p, context)
    return callback


func _set_owner_recursive(node, owner):
    node.owner = owner
    for child in node.get_children():
        _set_owner_recursive(child, owner)


func _get_meta_value(metadata, key:String) -> String:
    var _v = metadata.get(key, "")
    if _v is Array:
        return "\n".join(_v)
    else:
        return _v
