extends Node3D
class_name BaseCabinTool3D

var _controller
var _dirty:bool = false

signal controller_changed

@export_node_path("RailVehicle3D") var controller_path:NodePath = "":
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
            var vehicle := get_node_or_null(controller_path) as RailVehicle3D
            if vehicle:
                _controller = vehicle.get_controller()
                controller_changed.emit()

        if has_method("_process_dirty"):
            call("_process_dirty", delta)
    _process_tool(delta)
