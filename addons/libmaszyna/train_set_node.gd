@tool
extends Node
class_name TrainSetNode

var _dirty:bool = true

@export var auto_couple: bool = true
@export var start_track_id: String = "":
    set(value):
        if value == start_track_id:
            return
        start_track_id = value
        _dirty = true

@export var start_track_offset: float = 0.0:
    set(value):
        if is_equal_approx(value, start_track_offset):
            return
        start_track_offset = value
        _dirty = true


func _ready() -> void:
    place_on_track()
    if auto_couple:
        couple_all_vehicles()
    
    

func couple_all_vehicles():
    var previous_vehicle:RailVehicle3D
    
    for child in get_children():
        var vehicle = child as RailVehicle3D
        if vehicle:
            if previous_vehicle:
                previous_vehicle.get_controller().couple_back(vehicle.get_controller(), TrainController.FRONT)
            previous_vehicle = vehicle


func place_on_track():
    var current_offset = start_track_offset
    if not start_track_id:
        push_error("Trainset has no start_track_id set")
        return
        
    var track:VirtualTrack = TrackManager.get_track_by_name(start_track_id)
    if not track:
        push_error("Track not found: " + start_track_id)
        return
        
    for child in get_children():
        var vehicle = child as RailVehicle3D
        if vehicle:
            print(track, " ", current_offset, " ", track.get_world_position(current_offset))
            vehicle.global_position = track.get_world_position(current_offset)
            vehicle.global_transform.basis.z = track.get_axis(current_offset)
            current_offset -= vehicle.get_controller().length
            

func _process(_delta: float) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(_delta)
            _dirty = false

func _process_dirty(_delta: float) -> void:
    place_on_track()
