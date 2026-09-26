extends RefCounted
class_name MaszynaImporterContext

## An include submitted to [member queue]: its task and where its result belongs in this file
class PendingInclude:
    var task_id:int = -1
    ## Sizes of the result lists when the include was reached
    var sizes:Dictionary[String, int] = {}
    ## Task-local consist for vehicles of a trainset open across the include - its vehicles
    ## are moved into parent_trainset on merge (no node shared between threads)
    var trainset_proxy:TrainSet3D = null
    var parent_trainset:TrainSet3D = null

const RESULT_LISTS:Array[String] = [
    "tracks", "traction", "power_sources", "models", "light_events", "terrains", "triangles"
]

var _states: Array[Dictionary] = []
var include_depth: int = 0
## Number of cached subscenes (SceneryInstancer.parse_subscene_task()) enclosing the parsed file
var subscene_depth:int = 0
var rotate := Vector3.ZERO
var origin := Vector3.ZERO
var tracks:Array[MaszynaTrackData] = []
var traction:Array[MaszynaTractionData] = []
var power_sources:Array[MaszynaPowerSourceData] = []
var models:Array[MaszynaModelData] = []
var light_events:Array[MaszynaLightsEventData] = []
var terrains: Array = []
var triangles: Array = []
var dependencies:Dictionary = {}
var cacheable:bool = true
## Objects parsed from the file (set by SceneryInstancer.parse_file_task())
var objects:Array = []
## When set, "include" is parsed as a task of this queue instead of in place
## (see submit_include()/merge_pending_includes())
var queue:SceneryLoadingTaskQueue = null

## Set by "trainset:"/"endtrainset:" (maszyna_trainset_importer.gd/maszyna_endtrainset_importer.gd)
## and consumed by maszyna_node_dynamic_importer.gd - mirrors scene::scratch_data::trainset_data
## in the original engine (simulationstateserializer.h).
var trainset_open: bool = false
var trainset_name: String = ""
var trainset_track: String = ""
var trainset_offset: float = 0.0
var trainset_velocity: float = 0.0
## Consist node created by "trainset:" - vehicles of the open trainset become its children
var trainset_node: TrainSet3D = null

var _rotates = []
var _origins = []
var _triangles = []
var _active_files:Dictionary = {}


func register_dependency(path:String, size:int = -1) -> void:
    var normalized_path:String = path.simplify_path()
    if not FileAccess.file_exists(normalized_path):
        cacheable = false
        return
    var dependency_size:int = size
    if dependency_size < 0:
        var file:FileAccess = FileAccess.open(normalized_path, FileAccess.READ)
        if not file:
            cacheable = false
            return
        dependency_size = file.get_length()
    dependencies[normalized_path] = {
        "modified_time": FileAccess.get_modified_time(normalized_path),
        "size": dependency_size,
    }


func begin_file(path:String) -> bool:
    if _active_files.has(path):
        cacheable = false
        return false
    _active_files[path] = true
    return true


func end_file(path:String) -> void:
    _active_files.erase(path)

func push_rotate(new_rotate: Vector3):
    _rotates.push_front(rotate)
    rotate = new_rotate

func pop_rotate():
    rotate = _rotates.pop_front()

func push_origin(new_origin: Vector3):
    _origins.push_front(origin)
    origin = new_origin
    
func pop_origin():
    if _origins.size() > 0:
        origin = _origins.pop_front()


## State inherited by an included file (see from_state())
func get_state() -> Dictionary:
    return {
        "include_depth": include_depth,
        "subscene_depth": subscene_depth,
        "rotate": rotate,
        "origin": origin,
        "trainset_open": trainset_open,
        "trainset_name": trainset_name,
        "trainset_track": trainset_track,
        "trainset_offset": trainset_offset,
        "trainset_velocity": trainset_velocity,
        "trainset_node": trainset_node,
        "active_files": _active_files.duplicate(),
    }


static func from_state(state:Dictionary) -> MaszynaImporterContext:
    var context := MaszynaImporterContext.new()
    context.include_depth = state["include_depth"]
    context.subscene_depth = state["subscene_depth"]
    context.rotate = state["rotate"]
    context.origin = state["origin"]
    context.trainset_open = state["trainset_open"]
    context.trainset_name = state["trainset_name"]
    context.trainset_track = state["trainset_track"]
    context.trainset_offset = state["trainset_offset"]
    context.trainset_velocity = state["trainset_velocity"]
    context.trainset_node = state["trainset_node"]
    context._active_files = state["active_files"]
    return context


## Submits task(filename, parameters, state, queue) -> MaszynaImporterContext parsing an included
## file; returns the placeholder that stands in the parsed objects until merge_pending_includes()
func submit_include(task:Callable, filename:String, parameters:Dictionary) -> PendingInclude:
    var pending := PendingInclude.new()
    for list_name:String in RESULT_LISTS:
        pending.sizes[list_name] = (get(list_name) as Array).size()
    var state:Dictionary = get_state()
    state["include_depth"] = include_depth + 1
    if trainset_node:
        pending.parent_trainset = trainset_node
        pending.trainset_proxy = TrainSet3D.new()
        state["trainset_node"] = pending.trainset_proxy
    # one bind() - chained binds prepend the later arguments
    pending.task_id = queue.submit(task.bind(filename, parameters, state, queue))
    return pending


## Waits for the submitted includes and puts their results where the includes were in the file
func merge_pending_includes() -> void:
    var pendings:Array[PendingInclude] = []
    var children:Array[MaszynaImporterContext] = []
    var own_objects:Array = []
    var object_sizes:Array[int] = []
    for object:Variant in objects:
        if object is PendingInclude:
            pendings.append(object)
            object_sizes.append(own_objects.size())
        else:
            own_objects.append(object)
    if not pendings:
        return

    for pending:PendingInclude in pendings:
        var child:MaszynaImporterContext = queue.wait(pending.task_id) as MaszynaImporterContext
        children.append(child)
        if not child:
            cacheable = false
            continue
        dependencies.merge(child.dependencies)
        cacheable = cacheable and child.cacheable
        if pending.trainset_proxy:
            for vehicle:Node in pending.trainset_proxy.get_children():
                pending.trainset_proxy.remove_child(vehicle)
                pending.parent_trainset.add_child(vehicle)
            pending.trainset_proxy.free()

    objects = _merge_list(own_objects, object_sizes, children, "objects")
    for list_name:String in RESULT_LISTS:
        var sizes:Array[int] = []
        for pending:PendingInclude in pendings:
            sizes.append(pending.sizes[list_name])
        (get(list_name) as Array).assign(_merge_list(get(list_name), sizes, children, list_name))


## own split at sizes, with the children's list_name inserted in between
static func _merge_list(
    own:Array, sizes:Array[int], children:Array[MaszynaImporterContext], list_name:String
) -> Array:
    var merged:Array = []
    var start:int = 0
    for i:int in sizes.size():
        merged.append_array(own.slice(start, sizes[i]))
        if children[i]:
            merged.append_array(children[i].get(list_name))
        start = sizes[i]
    merged.append_array(own.slice(start))
    return merged


func push_state() -> void:
    _states.push_front({
        "include_depth": include_depth,
        "rotate": rotate,
        "origin": origin,
        "rotates_size": _rotates.size(),
        "origins_size": _origins.size(),
        "trainset_open": trainset_open,
        "trainset_name": trainset_name,
        "trainset_track": trainset_track,
        "trainset_offset": trainset_offset,
        "trainset_velocity": trainset_velocity,
        "trainset_node": trainset_node,
    })


func pop_state() -> void:
    if not _states:
        return

    var state: Dictionary = _states.pop_front()
    include_depth = state["include_depth"]
    rotate = state["rotate"]
    origin = state["origin"]
    trainset_open = state["trainset_open"]
    trainset_name = state["trainset_name"]
    trainset_track = state["trainset_track"]
    trainset_offset = state["trainset_offset"]
    trainset_velocity = state["trainset_velocity"]
    trainset_node = state["trainset_node"]

    while _rotates.size() > state["rotates_size"]:
        _rotates.pop_front()

    while _origins.size() > state["origins_size"]:
        _origins.pop_front()
