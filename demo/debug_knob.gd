@tool

extends Control
class_name DebugKnob

var _dirty = false
var _controller:TrainController


@export var label:String:
    set(x):
        _dirty = true
        label = x

@export var min_value:float = 0.0:
    set(x):
        _dirty = true
        min_value = x

@export var max_value:float = 1.0:
    set(x):
        _dirty = true
        max_value = x

@export var step:float = 0.1:
    set(x):
        _dirty = true
        step = x

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

        $SpinBox.min_value = min_value
        $SpinBox.max_value = max_value
        $SpinBox.step = step

        $Label.text = label
        if not _controller and not controller.is_empty():
            _controller = get_node(controller)
            #$SpinBox.disabled = false
        else:
            pass
            #$SpinBox.disabled = true

    if not Engine.is_editor_hint():
        _t += delta
        if _t > 0.1:
            _t = 0.0
            if _controller:
                if state_property:
                    var value = _controller.state.get(state_property)
                    if not value == null:
                        $SpinBox.value = value
                else:
                    pass
                    #$SpinBox.disabled = false
            else:
                pass
                #$SpinBox.disabled = true


func _on_spin_box_value_changed(value):
    if _controller and command:
        _controller.send_command(command, value)
