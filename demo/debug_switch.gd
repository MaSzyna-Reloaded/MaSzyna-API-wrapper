@tool

extends Control
class_name DebugSwitch

var _dirty = false
var _controller:TrainController


@export var label:String:
    set(x):
        _dirty = true
        label = x

enum SwitchType { MONOSTABLE, BISTABLE, TOGGLE }

@export var type:SwitchType = SwitchType.TOGGLE

@export_node_path("TrainController") var controller:NodePath:
    set(x):
        _dirty = true
        controller = x

@export var state_property:String:
    set(x):
        _dirty = true
        state_property = x

@export var command:String

func _ready():
    _dirty = true
var _t = 0.0

func _process(delta):
    if _dirty:
        _dirty = false
        if type == SwitchType.TOGGLE:
            $Switch.action_mode = Button.ACTION_MODE_BUTTON_RELEASE
            $Switch.toggle_mode = true
        else:
            $Switch.action_mode = Button.ACTION_MODE_BUTTON_PRESS
            $Switch.toggle_mode = false


        $Label.text = label
        if not _controller and not controller.is_empty():
            _controller = get_node(controller)
            $Switch.disabled = false
        else:
            $Switch.disabled = true

    if not Engine.is_editor_hint():
        _t += delta
        if _t > 0.1:
            _t = 0.0
            if _controller:
                if state_property:
                    var value = _controller.state.get(state_property)
                    if not value == null:
                        $Switch.button_pressed = true if value else false
                else:
                    $Switch.disabled = false
            else:
                $Switch.disabled = true


func _on_switch_toggled(toggled_on):
    if $Switch.action_mode == Button.ACTION_MODE_BUTTON_RELEASE and _controller and command:
        _controller.send_command(command, toggled_on)

func _on_switch_pressed():
    if $Switch.action_mode == Button.ACTION_MODE_BUTTON_PRESS and _controller and command:
        _controller.send_command(command, $Switch.button_pressed)

func _on_switch_button_up():
    if not type == SwitchType.MONOSTABLE:
        _controller.send_command(command, $Switch.button_pressed)
