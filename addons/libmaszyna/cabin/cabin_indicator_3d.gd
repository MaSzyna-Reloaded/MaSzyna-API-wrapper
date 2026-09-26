extends Node3D
class_name CabinIndicator3D

## The vehicle this element sits in, as the cabin root hands it down.
func set_vehicle_rid(vehicle_rid:RID) -> void:
    if _vehicle_rid == vehicle_rid:
        return
    _vehicle_rid = vehicle_rid
    _dirty = true


## Which vehicle this cabin element sits in; every read of it goes through CabinSystem.
var _vehicle_rid:RID
var _on_target:Node3D
var _off_target:Node3D
var _dirty:bool = false
var _update_elapsed:float = 0.0

## When the lamp is lit by state_property: a flag, or the sign of a number - the reverser's
## buttons light by the sign of DirActive (Train.cpp:8520)
enum LitCondition { TRUE, POSITIVE, ZERO, NEGATIVE }

@export var enabled:bool = false
@export var state_property:String = ""
@export var lit_condition:LitCondition = LitCondition.TRUE
## Lit while state_property is false - an "inactive" lamp of the same state (Train.cpp:9196).
@export var invert_value:bool = false
@export_node_path("Node3D") var on_target_path:NodePath = "":
    set(value):
        on_target_path = value
        _on_target = null
        _dirty = true
@export_node_path("Node3D") var off_target_path:NodePath = "":
    set(value):
        off_target_path = value
        _off_target = null
        _dirty = true


func _process(delta:float) -> void:
    if _dirty:
        _process_dirty()
        _dirty = false

    _update_elapsed += delta
    if _update_elapsed > 0.1:
        _update_elapsed = 0.0
        _update_state()


func _process_dirty() -> void:
    if not _on_target and on_target_path:
        _on_target = get_node_or_null(on_target_path)
    if not _off_target and off_target_path:
        _off_target = get_node_or_null(off_target_path)
    _update_state()


func _update_state() -> void:
    if _vehicle_rid and state_property:
        var state:Variant = CabinSystem.vehicle_state(_vehicle_rid).get(state_property, false)
        var value:bool = false
        match lit_condition:
            LitCondition.TRUE:
                value = true if state else false
            LitCondition.POSITIVE:
                value = float(state) > 0.0
            LitCondition.ZERO:
                value = float(state) == 0.0
            LitCondition.NEGATIVE:
                value = float(state) < 0.0
        enabled = not value if invert_value else value
    if _on_target:
        _on_target.visible = enabled
    if _off_target:
        _off_target.visible = not enabled
