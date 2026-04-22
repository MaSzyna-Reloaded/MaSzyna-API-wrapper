extends Node3D
class_name BaseCabinTool3D

var _controller
var _vehicle:Train3D
var _dirty:bool = false

signal controller_changed

@export_node_path("Train3D") var vehicle_path:NodePath = "":
    set(x):
        vehicle_path = x
        _controller = null
        _dirty = true

func _process_dirty(delta):
    pass

func _process_tool(delta):
    pass

func _process(delta):
    if _dirty:
        _dirty = false
        if not _vehicle and vehicle_path:
            _vehicle = get_node_or_null(vehicle_path) as Train3D
            if _vehicle:
                _controller = _vehicle.get_controller()
                controller_changed.emit()

        if has_method("_process_dirty"):
            call("_process_dirty", delta)
    _process_tool(delta)
