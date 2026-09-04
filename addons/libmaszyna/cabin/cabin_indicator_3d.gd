extends Node3D
class_name CabinIndicator3D

var _controller:TrainController
var _on_target:Node3D
var _off_target:Node3D
var _dirty:bool = false
var _update_elapsed:float = 0.0

@export var enabled:bool = false
@export var state_property:String = ""
@export_node_path("TrainController") var controller_path:NodePath = "":
    set(value):
        controller_path = value
        _controller = null
        _dirty = true
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
    if not _controller and controller_path:
        _controller = get_node(controller_path)
    if not _on_target and on_target_path:
        _on_target = get_node_or_null(on_target_path)
    if not _off_target and off_target_path:
        _off_target = get_node_or_null(off_target_path)
    _update_state()


func _update_state() -> void:
    if _controller and state_property:
        enabled = true if _controller.state.get(state_property, false) else false
    if _on_target:
        _on_target.visible = enabled
    if _off_target:
        _off_target.visible = not enabled
