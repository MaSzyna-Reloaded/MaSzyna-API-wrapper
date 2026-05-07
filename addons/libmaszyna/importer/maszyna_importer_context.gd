extends RefCounted
class_name MaszynaImporterContext

var _states: Array[Dictionary] = []
var include_depth: int = 0
var rotate := Vector3.ZERO
var origin := Vector3.ZERO
var tracks: Array[MaszynaTrack3D] = []
var traction: Array = []
var terrains: Array = []
var triangles: Array = []

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
    })


func pop_state() -> void:
    if _states.is_empty():
        return

    var state: Dictionary = _states.pop_front()
    include_depth = state["include_depth"]
    rotate = state["rotate"]
    origin = state["origin"]

    while _rotates.size() > state["rotates_size"]:
        _rotates.pop_front()

    while _origins.size() > state["origins_size"]:
        _origins.pop_front()
