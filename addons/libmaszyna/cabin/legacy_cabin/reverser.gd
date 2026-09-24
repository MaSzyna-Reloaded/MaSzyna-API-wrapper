extends RefCounted
class_name LegacyCabinReverser

## Reverser set by three push buttons instead of a dirkey lever (dynamic/pkp/e186_v2):
## TTrain::OnCommand_reverserforward/neutral/backward (Train.cpp:2748-2860) step the reverser
## until it reaches the position of the button.

const BUTTONS:Dictionary[StringName, int] = {
    &"dirforward_bt": 1,
    &"dirneutral_bt": 0,
    &"dirbackward_bt": -1,
}

var _train_id:String
var _cab:int
var _handlers:Dictionary[StringName, Callable] = {}


func control_ids() -> Array[StringName]:
    return BUTTONS.keys()


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    for button:StringName in BUTTONS:
        _handlers[button] = _button.bind(button)
        CabinSystem.register_control(train_id, cab, button, _handlers[button])


func unregister() -> void:
    for button:StringName in _handlers:
        CabinSystem.unregister_control(_train_id, _cab, button, _handlers[button])
    _handlers.clear()


func _button(state:CabinState, action:StringName, value:Variant, button:StringName) -> Variant:
    var pressed:bool = state.is_pressed(button, action, value)
    state.set_value(button, pressed)
    if not pressed:
        return null
    var result:Variant = null
    var steps:int = BUTTONS[button] - int(state.vehicle_state_value("direction", 0))
    for step:int in absi(steps):
        result = state.send_vehicle_command("direction_increase" if steps > 0 else "direction_decrease")
    return result
