@tool
extends Node3D

@export var start_track_name := "":
    set(x):
        start_track_name = x
        _apply_start_track()
@export var start_track_offset := 0.0:
    set(x):
        start_track_offset = x
        _apply_start_track()


func _apply_start_track():
    var offset := start_track_offset
    for vehicle:RailVehicle3D in find_children("*", "RailVehicle3D", false):
        vehicle.start_track_name = start_track_name
        vehicle.start_track_offset = offset
        offset += vehicle.length
