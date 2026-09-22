extends Node3D
class_name BaseCabinTool3D

var _controller:VehicleController
var _dirty:bool = false
var _applying_control_value:bool = false

signal controller_changed
## Emitted while the previous controller is still available for disconnecting signals.
signal controller_changing

@export_node_path("VehicleController") var controller_path:NodePath = "":
    set(x):
        controller_path = x
        _dirty = true

## Cabin control id (MMD label) - manipulations are reported to CabinSystem under this id; the
## registered cabin logic decides what they do to the vehicle.
@export var control_id:StringName = &""

func set_train_controller(controller:VehicleController) -> void:
    if _controller == controller:
        return
    controller_changing.emit()
    _controller = controller
    if _controller:
        controller_changed.emit()

## Reports a manipulation of this control to CabinSystem, for the occupied cab of its train.
# FIXME(#184): train_id is taken from VehicleController, which should not own it (it belongs to the
# vehicle in the scene).
func _act(action:StringName, value:Variant = null) -> Variant:
    if _applying_control_value or not _controller or not control_id:
        return null
    return CabinSystem.act(
            _controller.train_id, int(_controller.state.get("cabin_occupied", 1)), control_id, action, value)

# _notification runs on every class of the hierarchy, unlike _ready/_enter_tree overridden below.
func _notification(what:int) -> void:
    if what == NOTIFICATION_ENTER_TREE:
        CabinSystem.control_changed.connect(_on_cabin_control_changed)
    elif what == NOTIFICATION_EXIT_TREE:
        CabinSystem.control_changed.disconnect(_on_cabin_control_changed)

## Shows a control value set in CabinSystem (e.g. from the console) without reporting it back.
func _on_cabin_control_changed(train_id:String, _cab:int, p_control_id:StringName, value:Variant) -> void:
    if not p_control_id == control_id or not _controller or not train_id == _controller.train_id:
        return
    _applying_control_value = true
    _apply_control_value(value)
    _applying_control_value = false

func _apply_control_value(_value:Variant) -> void:
    pass

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
        var controller:VehicleController = get_node_or_null(controller_path) if controller_path else null
        set_train_controller(controller)

        if has_method("_process_dirty"):
            call("_process_dirty", delta)
    _process_tool(delta)
