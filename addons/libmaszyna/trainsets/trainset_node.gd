@tool
extends Node
class_name TrainSetNode

var _dirty:bool = true

@export var auto_couple: bool = true
@export var track_name: String = "":
    set(value):
        if value == track_name:
            return
        track_name = value
        _dirty = true

@export var track_offset: float = 0.0:
    set(value):
        if value == track_offset:
            return
        track_offset = value
        _dirty = true


func get_rail_vehicles() -> Array[RailVehicle3D]:
    var children = get_children()
    var vehicles:Array[RailVehicle3D] = []
    for child in children:
        var vehicle = child as RailVehicle3D
        if vehicle:
            vehicles.append(vehicle)
    return vehicles


func _process(_delta: float) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(_delta)
            _dirty = false


func _process_dirty(_delta: float) -> void:
    pass
