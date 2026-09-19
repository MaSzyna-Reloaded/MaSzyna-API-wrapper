extends RefCounted
class_name LegacyCabinBrakeCharging

## Train brake charging ("napełnianie uderzeniowe"), a keyboard-only control of the original cab layer:
## TTrain::OnCommand_trainbrakecharging (Train.cpp:1686) keeps the handle in the charging position while
## the key is held and on release lets a self-returning handle go back (brake_level_charging command).

const CONTROL:StringName = &"brake_charging"
const ACTION:StringName = &"brake_level_charging"

var _train_id:String
var _cab:int


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, CONTROL, _brake_charging)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, CONTROL, _brake_charging)


func _brake_charging(state:CabinState, action:StringName, _value:Variant) -> Variant:
    if action == &"hold":
        state.set_value(CONTROL, true)
        return state.send_vehicle_command("brake_level_charging", true)
    if action == &"release":
        state.set_value(CONTROL, false)
        return state.send_vehicle_command("brake_level_charging", false)
    return null
