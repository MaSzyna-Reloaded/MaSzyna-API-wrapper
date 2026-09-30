@tool

extends Button
class_name DebugButton

var _dirty = false

## The vehicle this widget drives, handed to it by the HUD - never looked up by a path into
## somebody else's scene.
var vehicle:RID = RID():
    set(x):
        if not vehicle == x:
            vehicle = x
            _dirty = true

@export var command:String
## Sent with the command as it is - a number, a word ("drive") or, with
## convert_argument_to_bool, the text "true"/"false" as a bool
@export var command_argument:Variant = ""
@export var convert_argument_to_bool:bool
func _ready():
    _dirty = true
var _t = 0.0

func _process(delta):
    if _dirty:
        _dirty = false

        if vehicle.is_valid():
            disabled = false
        else:
            disabled = true

    if not Engine.is_editor_hint():
        _t += delta
        if _t > 0.1:
            _t = 0.0
            if vehicle.is_valid():
                disabled = false
            else:
                disabled = true

func _on_pressed():
    if vehicle.is_valid() and command:
        if convert_argument_to_bool:
            if command_argument.to_lower() == "true":
                VehicleServer.vehicle_send_command(vehicle, command, true)
            if command_argument.to_lower() == "false":
                VehicleServer.vehicle_send_command(vehicle, command, false)
        else:
             VehicleServer.vehicle_send_command(vehicle, command, command_argument)
