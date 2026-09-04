extends Node3D
class_name BaseCabinTool3D

var _controller:TrainController
var _dirty:bool = false

signal controller_changed
## Emitted while the previous controller is still available for disconnecting signals.
signal controller_changing

@export_node_path("TrainController") var controller_path:NodePath = "":
    set(x):
        controller_path = x
        _dirty = true

func set_train_controller(controller:TrainController) -> void:
    if _controller == controller:
        return
    controller_changing.emit()
    _controller = controller
    if _controller:
        controller_changed.emit()

func _exit_tree() -> void:
    set_train_controller(null)
    _dirty = true

func _process_dirty(delta):
    pass

func _process_tool(delta):
    pass

func _process(delta):
    if _dirty:
        _dirty = false
        var controller:TrainController = get_node_or_null(controller_path) if controller_path else null
        set_train_controller(controller)

        if has_method("_process_dirty"):
            call("_process_dirty", delta)
    _process_tool(delta)
