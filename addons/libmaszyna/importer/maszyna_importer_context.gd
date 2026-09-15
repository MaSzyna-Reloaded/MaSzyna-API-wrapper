extends RefCounted
class_name MaszynaImporterContext

var _states: Array[Dictionary] = []
var include_depth: int = 0
var rotate := Vector3.ZERO
var origin := Vector3.ZERO
var tracks:Array[MaszynaTrackData] = []
var traction:Array[MaszynaTractionData] = []
var power_sources:Array[MaszynaPowerSourceData] = []
var terrains: Array = []
var triangles: Array = []
var dependencies:Dictionary = {}
var cacheable:bool = true

## Set by "trainset:"/"endtrainset:" (maszyna_trainset_importer.gd/maszyna_endtrainset_importer.gd)
## and consumed by maszyna_node_dynamic_importer.gd - mirrors scene::scratch_data::trainset_data
## in the original engine (simulationstateserializer.h).
var trainset_open: bool = false
var trainset_name: String = ""
var trainset_track: String = ""
var trainset_offset: float = 0.0
var trainset_velocity: float = 0.0

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

    while _rotates.size() > state["rotates_size"]:
        _rotates.pop_front()

    while _origins.size() > state["origins_size"]:
        _origins.pop_front()
