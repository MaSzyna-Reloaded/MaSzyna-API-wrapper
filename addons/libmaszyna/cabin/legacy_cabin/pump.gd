extends RefCounted
class_name LegacyCabinPump

## A diesel engine's fuel or oil pump switch (fuelpump_sw, oilpump_sw) - TTrain::
## OnCommand_fuelpumptoggle/enable/disable (Train.cpp:3871-3968) and their oil pump twins
## (Train.cpp:3970-4067). What it does depends on the kind of switch:
## * push (type: return) - the pump runs while it is held (Train.cpp:3914);
## * two-state - a press flips it, and switching off also sets the pump's off flag
##   (FuelPumpSwitchOff, Train.cpp:3937, 3963).

var _control:StringName
var _command:String
var _switch_off_command:String
var _enabled_state:String
var _button_type:CabinButton.ButtonType
var _train_id:String
var _cab:int


## control: the MMD label; command/switch_off_command: the pump's vehicle commands;
## enabled_state: the state key of its switch (is_enabled)
func _init(control:StringName, command:String, switch_off_command:String, enabled_state:String,
        button_type:CabinButton.ButtonType) -> void:
    _control = control
    _command = command
    _switch_off_command = switch_off_command
    _enabled_state = enabled_state
    _button_type = button_type


func control_ids() -> Array[StringName]:
    return [_control]


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, _control, _pump)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, _control, _pump)


func _pump(state:CabinState, action:StringName, value:Variant) -> Variant:
    if _button_type == CabinButton.ButtonType.PUSH:
        var held:bool = state.is_pressed(_control, action, value)
        state.set_value(_control, held)
        return state.send_vehicle_command(_command, held)
    # two-state: only a press counts (Train.cpp:3889)
    if action == &"release":
        return null
    var enabled:bool = (not state.vehicle_state_value(_enabled_state, false)
            if value == null or action == &"hold" else bool(value))
    state.set_value(_control, enabled)
    state.send_vehicle_command(_switch_off_command, not enabled)
    return state.send_vehicle_command(_command, enabled)
