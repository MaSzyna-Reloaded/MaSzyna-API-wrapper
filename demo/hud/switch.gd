@tool

extends Control
class_name DebugSwitch

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

enum SwitchType { MONOSTABLE, BISTABLE, TOGGLE }

@export var type:SwitchType = SwitchType.TOGGLE

@export_node_path("VehiclePhysicsNode") var controller:NodePath:
    set(x):
        _dirty = true
        controller = x

@export var state_property:String:
    set(x):
        _dirty = true
        state_property = x

@export var command:String
## Sent before the switch's own state, for a command that also names what it switches
## (pantograph(selector, enabled)); null sends the state alone
@export var command_argument:Variant = null

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
        if vehicle.is_valid():
            $Switch.disabled = false
        else:
            $Switch.disabled = true

    if not Engine.is_editor_hint():
        _t += delta
        if _t > 0.1:
            _t = 0.0
            if vehicle.is_valid():
                if state_property:
                    var value = VehicleServer.vehicle_dump_state(vehicle).get(state_property)
                    if not value == null:
                        # shown, not switched: a state shown must not send it back to the vehicle
                        $Switch.set_pressed_no_signal(true if value else false)
                        $Switch.modulate = Color.GREEN if value else Color.WHITE
                else:
                    $Switch.disabled = false
            else:
                $Switch.disabled = true


func _on_switch_toggled(toggled_on):
    if $Switch.action_mode == Button.ACTION_MODE_BUTTON_RELEASE and vehicle.is_valid() and command:
        _send(toggled_on)

func _on_switch_pressed():
    if $Switch.action_mode == Button.ACTION_MODE_BUTTON_PRESS and vehicle.is_valid() and command:
        _send($Switch.button_pressed)

func _on_switch_button_up():
    if not type == SwitchType.MONOSTABLE:
        _send($Switch.button_pressed)


func _send(enabled:bool) -> void:
    if command_argument == null:
        VehicleServer.vehicle_send_command(vehicle, command, enabled)
        return
    VehicleServer.vehicle_send_command(vehicle, command, command_argument, enabled)
