@tool

extends Control
class_name DebugKnob

var _dirty = false

## The vehicle this widget drives, handed to it by the HUD - never looked up by a path into
## somebody else's scene.
var vehicle:RID = RID():
    set(x):
        if not vehicle == x:
            vehicle = x
            _dirty = true



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

@export_node_path("VehiclePhysicsNode") var controller:NodePath:
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

    if not Engine.is_editor_hint():
        _t += delta
        if _t > 0.1:
            _t = 0.0
            if vehicle.is_valid():
                if state_property:
                    var value = VehicleServer.vehicle_dump_state(vehicle).get(state_property)
                    if not value == null:
                        $SpinBox.value = value
                else:
                    pass
                    #$SpinBox.disabled = false
            else:
                pass
                #$SpinBox.disabled = true


func _on_spin_box_value_changed(value):
    if vehicle.is_valid() and command:
        VehicleServer.vehicle_send_command(vehicle, command, value)
