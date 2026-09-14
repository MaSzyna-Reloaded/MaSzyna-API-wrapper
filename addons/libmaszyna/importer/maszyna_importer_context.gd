extends RefCounted
class_name MaszynaImporterContext

var _states: Array[Dictionary] = []
var include_depth: int = 0
var rotate := Vector3.ZERO
var origin := Vector3.ZERO
var tracks: Array = [] # Array[MaszynaNodeTrackImporter.TrackData]
var traction: Array = []
var power_sources: Array = []
var terrains: Array = []
var triangles: Array = []

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
    if _states.is_empty():
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
