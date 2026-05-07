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
const _TIMING_MS := 0.001
const TRIANGLE_CHUNK_SIZE_M := 1000.0


func instantiate(root: MaszynaIncludeNode, parameters: Dictionary = {}) -> void:
    var total_started_usec := Time.get_ticks_usec()

    var clear_started_usec := Time.get_ticks_usec()
    for child in root.get_children(true):
        root.remove_child(child)
    var clear_elapsed_usec := Time.get_ticks_usec() - clear_started_usec

    var context := MaszynaImporterContext.new()
    context.rotate = root.context_rotate
    context.origin = root.context_origin

    var parse_started_usec := Time.get_ticks_usec()
    var objects = parse_file(root.filename, parameters, context)
    var parse_elapsed_usec := Time.get_ticks_usec() - parse_started_usec
    
# create tracks
    var tracks_started_usec := Time.get_ticks_usec()
    var tracks = Node3D.new()
    tracks.name = "Tracks"
    for item in context.tracks:
        tracks.add_child(item)
    if tracks.get_child_count() > 0:
        objects.insert(0, tracks)
    var tracks_elapsed_usec := Time.get_ticks_usec() - tracks_started_usec

# create traction    
    var traction_started_usec := Time.get_ticks_usec()
    var traction = Node3D.new()
    traction.name = "Traction"
    for item in context.traction:
        traction.add_child(item)
    if traction.get_child_count() > 0:
        objects.insert(0, traction)
    var traction_elapsed_usec := Time.get_ticks_usec() - traction_started_usec

# create meshinstances for triangles grouped by material

    var triangles_group_started_usec := Time.get_ticks_usec()
    var triangle_chunks: Array = SceneryTrianglesBuilder.build_chunks(context.triangles, TRIANGLE_CHUNK_SIZE_M)
    var triangles_group_elapsed_usec := Time.get_ticks_usec() - triangles_group_started_usec
    
    var triangles_build_started_usec := Time.get_ticks_usec()
    for chunk in triangle_chunks:
        var texture: String = chunk["texture"]
        var chunk_x: int = chunk["chunk_x"]
        var chunk_z: int = chunk["chunk_z"]
        var chunk_origin: Vector3 = chunk["origin"]
        var node = MeshInstance3D.new()
        var mesh = ArrayMesh.new()
        var arrays: Array = []
        arrays.resize(Mesh.ARRAY_MAX)
        arrays[Mesh.ARRAY_VERTEX] = chunk["vertices"]
        arrays[Mesh.ARRAY_NORMAL] = chunk["normals"]
        arrays[Mesh.ARRAY_TEX_UV] = chunk["uvs"]
        mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
        node.mesh = mesh
        node.name = "%s_%s_%s" % [texture, chunk_x, chunk_z]
        node.position = chunk_origin
        var material = MaterialManager.get_material("", texture)
        if material:
            node.material_override = material
        objects.append(node)
    var triangles_build_elapsed_usec := Time.get_ticks_usec() - triangles_build_started_usec
    
    
    var attach_started_usec := Time.get_ticks_usec()
    for obj in objects:
        if not obj:
            continue

        if obj is Node:
            root.add_child(obj)
            #_set_owner_recursive(obj, root)
    var attach_elapsed_usec := Time.get_ticks_usec() - attach_started_usec
    var total_elapsed_usec := Time.get_ticks_usec() - total_started_usec

    #_print_timing_report(
    #    root.filename,
    #    clear_elapsed_usec,
    #    parse_elapsed_usec,
    #    tracks_elapsed_usec,
    #    traction_elapsed_usec,
    #    triangles_group_elapsed_usec,
    #    triangles_build_elapsed_usec,
    #    attach_elapsed_usec,
    #    total_elapsed_usec,
    #    context,
    #    objects.size(),
    #    triangles_by_material.size()
    #)


static func parse_file(filename: String, parameters: Dictionary, context: MaszynaImporterContext) -> Array:
    var abs_file = UserSettings.get_maszyna_game_dir().path_join("scenery").path_join(filename)
    var file := FileAccess.open(abs_file, FileAccess.READ)
    if not file:
        push_error("Cannot load scenery: " + abs_file)
        return []

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

    for token in ["sky", "atmo", "node", "event", "origin", "endorigin", "rotate", "terrain", "include", "trainset", "firstinit"]:
        parser.unregister_handler(token)
    parser.unreference()

    return objects


static func _make_importer_callback(importer, context) -> Callable:
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


func _print_timing_report(
    filename: String,
    clear_elapsed_usec: int,
    parse_elapsed_usec: int,
    tracks_elapsed_usec: int,
    traction_elapsed_usec: int,
    triangles_group_elapsed_usec: int,
    triangles_build_elapsed_usec: int,
    attach_elapsed_usec: int,
    total_elapsed_usec: int,
    context: MaszynaImporterContext,
    object_count: int,
    triangle_material_count: int
) -> void:
    print(
        "[SceneryInstancer] file=%s total=%.3fms clear=%.3fms parse=%.3fms tracks=%.3fms traction=%.3fms triangles_group=%.3fms triangles_build=%.3fms attach=%.3fms objects=%d tracks_count=%d traction_count=%d triangle_batches=%d triangles_count=%d"
        % [
            filename,
            total_elapsed_usec * _TIMING_MS,
            clear_elapsed_usec * _TIMING_MS,
            parse_elapsed_usec * _TIMING_MS,
            tracks_elapsed_usec * _TIMING_MS,
            traction_elapsed_usec * _TIMING_MS,
            triangles_group_elapsed_usec * _TIMING_MS,
            triangles_build_elapsed_usec * _TIMING_MS,
            attach_elapsed_usec * _TIMING_MS,
            object_count,
            context.tracks.size(),
            context.traction.size(),
            triangle_material_count,
            context.triangles.size(),
        ]
    )
