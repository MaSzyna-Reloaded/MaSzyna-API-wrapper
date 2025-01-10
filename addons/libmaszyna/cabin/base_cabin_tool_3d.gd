extends Node3D
class_name BaseCabinTool3D

var _controller:TrainController
var _dirty:bool = false

signal controller_changed

@export_node_path("TrainController") var controller_path:NodePath = "":
    set(x):
        controller_path = x
        _controller = null
        _dirty = true

func _process_dirty(delta):
    pass

func _process_tool(delta):
    pass

func _process(delta):
    if _dirty:
        _dirty = false
        if not _controller and controller_path:
            _controller = get_node(controller_path)
            controller_changed.emit()

        if has_method("_process_dirty"):
            call("_process_dirty", delta)
    _process_tool(delta)
