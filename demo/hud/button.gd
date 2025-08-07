@tool

extends Button
class_name DebugButton

var _dirty = false
var _controller:TrainController

@export_node_path("TrainController") var controller:NodePath:
    set(x):
        _dirty = true
        _controller = null
        controller = x

@export var command:String
@export var command_argument:String
@export var convert_argument_to_bool:bool #In godot 4.5 there will be ability to export variable with variant type
func _ready():
    _dirty = true
var _t = 0.0

func _process(delta):
    if _dirty:
        _dirty = false

        if not _controller and not controller.is_empty():
            _controller = get_node(controller)
            disabled = false
        else:
            disabled = true

    if not Engine.is_editor_hint():
        _t += delta
        if _t > 0.1:
            _t = 0.0
            if _controller:
                disabled = false
            else:
                disabled = true

func _on_pressed():
    if _controller and command:
        if convert_argument_to_bool:
            if command_argument.to_lower() == "true":
                _controller.send_command(command, true)
            if command_argument.to_lower() == "false":
                _controller.send_command(command, false)
        else:
             _controller.send_command(command, command_argument)
