extends Object
class_name MaszynaImporterContext

var rotate := Vector3.ZERO
var origin := Vector3.ZERO
var tracks: Array[MaszynaTrack3D] = []
var traction: Array = []
var terrains: Array = []

var _rotates = []
var _origins = []

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
        
