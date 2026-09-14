@tool
extends Node

static var sky_importer = preload("res://addons/libmaszyna/importer/maszyna_sky_importer.gd").new()
static var atmo_importer = preload("res://addons/libmaszyna/importer/maszyna_atmo_importer.gd").new()
static var time_importer = preload("res://addons/libmaszyna/importer/maszyna_time_importer.gd").new()
static var node_importer = preload("res://addons/libmaszyna/importer/maszyna_node_importer.gd").new()
static var event_importer = preload("res://addons/libmaszyna/importer/maszyna_event_importer.gd").new()
static var origin_importer = preload("res://addons/libmaszyna/importer/maszyna_origin_importer.gd").new()
static var endorigin_importer = preload("res://addons/libmaszyna/importer/maszyna_endorigin_importer.gd").new()
static var rotate_importer = preload("res://addons/libmaszyna/importer/maszyna_rotate_importer.gd").new()
static var terrain_importer = preload("res://addons/libmaszyna/importer/maszyna_terrain_importer.gd").new()
static var include_importer = preload("res://addons/libmaszyna/importer/maszyna_include_importer.gd").new()
static var trainset_importer = preload("res://addons/libmaszyna/importer/maszyna_trainset_importer.gd").new()
static var endtrainset_importer = preload("res://addons/libmaszyna/importer/maszyna_endtrainset_importer.gd").new()
static var firstinit_importer = preload("res://addons/libmaszyna/importer/maszyna_firstinit_importer.gd").new()
const TRIANGLE_CHUNK_SIZE_M := 1000.0
const CACHE_FORMAT_VERSION:int = 4
const CACHE_DIRECTORY:String = "scenery_compiled"

static var _cache:ResourceCache = ResourceCache.create(CACHE_DIRECTORY)


## Wired into the "Clear caches" button (user_settings_dock.gd) alongside
## E3DModelManager.clear_cache()/MaterialManager.clear_cache() - nothing previously cleared this
## one, so a stale/corrupt compiled-scenery cache entry (e.g. from a killed process mid-write)
## had no way to be cleared from the settings UI.
static func clear_cache() -> void:
    _cache.clear()


## Parses root.filename and (re-)populates root with everything the scenery declares.
##
## Tracks and traction are built directly against TrackManager/TrackRenderingServer/
## TractionRenderingServer's RID-based API (_build_track()/_build_traction() below) instead of
## instantiating TrackNormal3D/TrackSwitch3D/MaszynaTraction3D nodes - a real scenery can have
## thousands of these, and a Node per segment (each with its own @tool script and per-frame
## _process()) is overhead that only actually earns its keep for a handful of hand-authored
## pieces edited directly in a scene like demo_3d.tscn. root keeps the resulting RIDs
## (_track_rids/_track_render_rids/_traction_rids/_wire_power_rids/_power_source_rids) so they
## can be freed on the next reload or when root itself leaves the tree - see maszyna_include.gd.
func instantiate(root: MaszynaIncludeNode, parameters: Dictionary = {}) -> void:
    var source_path:String = _get_source_path(root.filename)
    var parameters_hash:String = _get_parameters_hash(parameters)
    var cache_path:String = _get_cache_path(source_path, parameters_hash)
    var world_3d:World3D = root.get_world_3d()
    var compiled:MaszynaCompiledScenery
    if root.use_cache:
        compiled = _load_cached(cache_path, source_path, parameters_hash)

    if compiled:
        _instantiate_server_data(
            root,
            world_3d,
            compiled.tracks,
            compiled.traction,
            compiled.power_sources,
        )
        _attach_objects(root, _instantiate_cached_nodes(compiled.nodes))
        return

    var context := MaszynaImporterContext.new()
    context.rotate = root.context_rotate
    context.origin = root.context_origin
    var objects:Array = parse_file(root.filename, parameters, context)

    _instantiate_server_data(root, world_3d, context.tracks, context.traction, context.power_sources)
    objects.append_array(_build_triangle_nodes(context.triangles))

    if root.use_cache and context.cacheable:
        compiled = _compile_scenery(source_path, parameters_hash, context, objects)
        if compiled:
            _cache.set(cache_path, compiled)

    _attach_objects(root, objects)


static func _instantiate_server_data(
    root:MaszynaIncludeNode,
    world_3d:World3D,
    tracks:Array[MaszynaTrackData],
    traction:Array[MaszynaTractionData],
    power_sources:Array[MaszynaPowerSourceData],
) -> void:
    for track_data:MaszynaTrackData in tracks:
        var built:Dictionary = _build_track(track_data, world_3d)
        root._track_rids.append(built["track_rid"])
        root._track_render_rids.append(built["track_render_rid"])

    for power_source_data:MaszynaPowerSourceData in power_sources:
        root._power_source_rids.append(_build_power_source(power_source_data))
    for traction_data:MaszynaTractionData in traction:
        var traction_rid:RID = _build_traction(traction_data, world_3d)
        root._traction_rids.append(traction_rid)
        root._wire_power_rids.append(_build_wire_power(traction_data))
    if root._wire_power_rids.size() > 0:
        TractionPowerServer.network_build()

    if root._track_rids.size() > 0:
        TrackManager.topology_rebuild()


static func _build_triangle_nodes(triangles:Array) -> Array:
    var objects:Array = []
    # create meshinstances for triangles grouped by material and by their per-node
    # range_min/range_max (e.g. grass.inc's "node 300 0 ... triangles" is only meant to be
    # visible within 300m - see maszyna_node_importer.gd's own range_min/range_max handling for
    # regular model nodes). Triangles are bucketed by range before chunking so a distance-limited
    # patch of grass never ends up merged into an always-visible terrain chunk mesh.
    var triangles_by_range: Dictionary = {}
    for triangle_entry: Array in triangles:
        var entry_range_min: float = triangle_entry[4] if triangle_entry.size() > 4 else 0.0
        var entry_range_max: float = triangle_entry[5] if triangle_entry.size() > 5 else -1.0
        var range_key: Vector2 = Vector2(entry_range_min, entry_range_max)
        if not triangles_by_range.has(range_key):
            triangles_by_range[range_key] = []
        (triangles_by_range[range_key] as Array).append(triangle_entry)

    for range_key: Vector2 in triangles_by_range.keys():
        var range_min: float = range_key.x
        var range_max: float = range_key.y
        var triangle_chunks: Array = SceneryTrianglesBuilder.build_chunks(triangles_by_range[range_key], TRIANGLE_CHUNK_SIZE_M)
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
            if range_max > 0:
                node.visibility_range_begin = range_min
                node.visibility_range_end = range_max
            objects.append(node)
    return objects


static func _attach_objects(root:MaszynaIncludeNode, objects:Array) -> void:
    for obj:Variant in objects:
        if not obj:
            continue

        if obj is Node:
            root.add_child(obj)
    if Engine.is_editor_hint():
        root.SceneryEditor.update_owners(root)


static func _compile_scenery(
    source_path:String,
    parameters_hash:String,
    context:MaszynaImporterContext,
    objects:Array,
) -> MaszynaCompiledScenery:
    var packed_scene:PackedScene = _pack_objects(objects)
    if not packed_scene:
        return null

    var compiled := MaszynaCompiledScenery.new()
    compiled.format_version = CACHE_FORMAT_VERSION
    compiled.source_path = source_path
    compiled.parameters_hash = parameters_hash
    compiled.dependencies = context.dependencies.duplicate(true)
    compiled.nodes = packed_scene
    compiled.tracks = context.tracks
    compiled.traction = context.traction
    compiled.power_sources = context.power_sources
    return compiled


static func _pack_objects(objects:Array) -> PackedScene:
    var scene_root := Node3D.new()
    for object:Variant in objects:
        if object is Node:
            scene_root.add_child(object)
            object.owner = scene_root

    var packed_scene := PackedScene.new()
    var result:Error = packed_scene.pack(scene_root)

    for child:Node in scene_root.get_children():
        child.owner = null
        scene_root.remove_child(child)
    scene_root.free()

    if not result == OK:
        push_error("Cannot compile scenery cache")
        return null
    return packed_scene


static func _instantiate_cached_nodes(packed_scene:PackedScene) -> Array:
    var objects:Array = []
    if not packed_scene:
        return objects
    var scene_root:Node = packed_scene.instantiate()
    for child:Node in scene_root.get_children():
        scene_root.remove_child(child)
        child.owner = null
        objects.append(child)
    scene_root.free()
    return objects


static func _load_cached(
    cache_path:String,
    source_path:String,
    parameters_hash:String,
) -> MaszynaCompiledScenery:
    var compiled:MaszynaCompiledScenery = _cache.get(cache_path) as MaszynaCompiledScenery
    if not compiled:
        return null
    if not _is_cache_valid(compiled, source_path, parameters_hash):
        _cache.remove(cache_path)
        return null
    return compiled


static func _is_cache_valid(
    compiled:MaszynaCompiledScenery,
    source_path:String,
    parameters_hash:String,
) -> bool:
    if not compiled.format_version == CACHE_FORMAT_VERSION:
        return false
    if not compiled.source_path == source_path:
        return false
    if not compiled.parameters_hash == parameters_hash:
        return false
    if not compiled.nodes:
        return false
    if not compiled.dependencies.has(source_path):
        return false
    for dependency_value:Variant in compiled.dependencies.keys():
        var dependency_path:String = dependency_value
        if not FileAccess.file_exists(dependency_path):
            return false
        var state:Dictionary = compiled.dependencies[dependency_path]
        if not FileAccess.get_modified_time(dependency_path) == int(state["modified_time"]):
            return false
        var file:FileAccess = FileAccess.open(dependency_path, FileAccess.READ)
        if not file:
            return false
        if not file.get_length() == int(state["size"]):
            return false
    return true


static func _get_parameters_hash(parameters:Dictionary) -> String:
    var keys:Array = parameters.keys()
    keys.sort()
    var parts:Array[String] = []
    for key_value:Variant in keys:
        var key:String = str(key_value)
        var value:String = JSON.stringify(parameters[key_value])
        parts.append("%d:%s=%d:%s" % [key.length(), key, value.length(), value])
    return "|".join(parts).md5_text()


static func _get_cache_path(source_path:String, parameters_hash:String) -> String:
    return (
        "%s:%s:%s" % [CACHE_FORMAT_VERSION, source_path, parameters_hash]
    ).md5_text() + ".res"


static func _get_source_path(filename:String) -> String:
    return UserSettings.get_maszyna_game_dir().path_join("scenery").path_join(filename).simplify_path()


func scenery_exists(filename: String):
    var abs_file:String = _get_source_path(filename)
    return FileAccess.file_exists(abs_file)


func parse_file(filename: String, parameters: Dictionary, context: MaszynaImporterContext) -> Array:
    var abs_file:String = _get_source_path(filename)
    if not context.begin_file(abs_file):
        push_error("Recursive scenery include: " + abs_file)
        return []
    var file := FileAccess.open(abs_file, FileAccess.READ)
    if not file:
        context.end_file(abs_file)
        push_error("Cannot load scenery: " + abs_file)
        return []
    context.register_dependency(abs_file, file.get_length())

    var parser := MaszynaParser.new()
    parser.set_parameters(parameters)
    parser.initialize(file.get_buffer(file.get_length()))
    parser.register_handler("sky", _make_importer_callback(sky_importer, context))
    parser.register_handler("atmo", _make_importer_callback(atmo_importer, context))
    parser.register_handler("time", _make_importer_callback(time_importer, context))
    parser.register_handler("node", _make_importer_callback(node_importer, context))
    parser.register_handler("event", _make_importer_callback(event_importer, context))
    parser.register_handler("origin", _make_importer_callback(origin_importer, context))
    parser.register_handler("endorigin", _make_importer_callback(endorigin_importer, context))
    parser.register_handler("rotate", _make_importer_callback(rotate_importer, context))
    parser.register_handler("terrain", _make_importer_callback(terrain_importer, context))
    parser.register_handler("include", _make_importer_callback(include_importer, context))
    parser.register_handler("trainset", _make_importer_callback(trainset_importer, context))
    parser.register_handler("endtrainset", _make_importer_callback(endtrainset_importer, context))
    parser.register_handler("firstinit", _make_importer_callback(firstinit_importer, context))

    var objects = parser.parse()

    for token in ["sky", "atmo", "node", "event", "origin", "endorigin", "rotate", "terrain", "include", "trainset", "endtrainset", "firstinit"]:
        parser.unregister_handler(token)
    parser.unreference()
    context.end_file(abs_file)

    return objects


static func _make_importer_callback(importer, context) -> Callable:
    var callback = func(p): return importer.import(p, context)
    return callback


## Mirrors TrackNormal3D/TrackSwitch3D's own _create_track()/_update_track_data()/
## _update_track_rendering() (addons/libmaszyna/tracks/track_normal_3d.gd,
## track_switch_3d.gd), minus the Node - see instantiate()'s doc comment for why.
static func _build_track(track_data:MaszynaTrackData, world_3d:World3D) -> Dictionary:
    var track_rid:RID = TrackManager.track_create()
    var track_render_rid:RID = TrackRenderingServer.create_track(track_rid)
    TrackRenderingServer.set_track_scenario(track_render_rid, world_3d.scenario)

    TrackManager.track_update_curves(track_rid, track_data.curve, track_data.diverging_curve)
    TrackManager.track_update(track_rid, track_data.type, track_data.track_name, track_data.width)
    if track_data.type == TrackManager.TrackType.TRACK_SWITCH:
        TrackManager.switch_set_active_track(track_rid, TrackManager.SwitchTrack.TRACK_COMMON)

    TrackRenderingServer.set_track_render_options(
        track_render_rid,
        track_data.tex_length,
        track_data.tex_height,
        track_data.tex_width,
        track_data.tex_slope,
        track_data.material1,
        track_data.material2,
        "", # trackbed_material - the .scn format never declares one, falls back to material2
        track_data.railprofile,
        true, # rail_visible
        true, # ballast_visible
    )
    TrackRenderingServer.rebuild_track(track_render_rid)
    TrackRenderingServer.set_track_visible(track_render_rid, track_data.visible)

    return {"track_rid": track_rid, "track_render_rid": track_render_rid}


## Mirrors MaszynaTraction3D's own _enter_tree()/_update() (addons/libmaszyna/traction/
## maszyna_traction_3d.gd), minus the Node. contact_p1/p2/support_p1/p2 are already absolute
## world coordinates read straight from the .scn (same as track curve points), so the traction's
## own transform is identity - there's nothing local left to place.
static func _build_traction(traction_data:MaszynaTractionData, world_3d:World3D) -> RID:
    var traction_rid:RID = TractionRenderingServer.create_traction()
    TractionRenderingServer.set_traction_geometry(
        traction_rid,
        traction_data.contact_p1,
        traction_data.contact_p2,
        traction_data.support_p1,
        traction_data.support_p2,
        traction_data.wire_thickness,
        traction_data.wires,
        traction_data.wire_offset,
        traction_data.min_height,
        traction_data.segment_length,
    )
    TractionRenderingServer.set_traction_transform(traction_rid, Transform3D.IDENTITY)
    TractionRenderingServer.set_traction_visible(traction_rid, traction_data.visible)
    TractionRenderingServer.set_traction_scenario(traction_rid, world_3d.scenario)
    TractionRenderingServer.set_traction_material(traction_rid, _get_traction_material(traction_data).get_rid())
    return traction_rid


## Mirrors _build_traction() - a tractionpowersource node has no visual representation, so this
## only ever registers electrical data against TractionPowerServer.
static func _build_power_source(power_source_data:MaszynaPowerSourceData) -> RID:
    var power_source_rid:RID = TractionPowerServer.power_source_create()
    TractionPowerServer.power_source_set_params(
        power_source_rid,
        power_source_data.name,
        power_source_data.nominal_voltage,
        power_source_data.voltage_frequency,
        power_source_data.internal_resistance,
        power_source_data.max_output_current,
        power_source_data.fast_fuse_timeout,
        power_source_data.fast_fuse_repetition,
        power_source_data.slow_fuse_timeout,
        power_source_data.recuperation,
    )
    return power_source_rid


## Registers a traction wire's electrical data (as opposed to _build_traction()'s visual mesh)
## against TractionPowerServer - a second, purely-electrical RID for the same wire span.
static func _build_wire_power(traction_data:MaszynaTractionData) -> RID:
    var wire_rid:RID = TractionPowerServer.wire_create()
    TractionPowerServer.wire_set_params(
        wire_rid,
        traction_data.contact_p1,
        traction_data.contact_p2,
        traction_data.power_supply_name,
        traction_data.nominal_voltage,
        traction_data.max_current,
        traction_data.resistivity,
    )
    return wire_rid


static func _get_traction_material(traction_data:MaszynaTractionData) -> Material:
    var is_copper:bool = traction_data.material == 0 # TractionMaterial.COPPER
    if traction_data.damage_flag & MaszynaTraction3D.DamageFlag.PATINA:
        return (
            MaszynaTraction3D.TRACTION_CU_PATINA_MATERIAL if is_copper
            else MaszynaTraction3D.TRACTION_AL_PATINA_MATERIAL
        )
    return MaszynaTraction3D.TRACTION_CU_MATERIAL if is_copper else MaszynaTraction3D.TRACTION_AL_MATERIAL
