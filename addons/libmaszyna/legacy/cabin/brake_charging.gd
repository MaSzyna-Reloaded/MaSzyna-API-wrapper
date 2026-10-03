extends RefCounted
class_name LegacyCabinBrakeCharging

## Train brake charging ("napełnianie uderzeniowe"), a keyboard-only control of the original cab layer:
## TTrain::OnCommand_trainbrakecharging (Train.cpp:1686) keeps the handle in the charging position while
## the key is held and on release lets a self-returning handle go back (brake_level_charging command).

const CONTROL:StringName = &"brake_charging"
const ACTION:StringName = &"brake_level_charging"

var _vehicle_rid:RID
var _cab:int


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, CONTROL, _brake_charging)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, CONTROL, _brake_charging)


func _brake_charging(state:CabinState, action:StringName, _value:Variant) -> Variant:
    if action == &"hold":
        state.set_value(CONTROL, true)
        return state.send_vehicle_command("brake_level_charging", true)
    if action == &"release":
        state.set_value(CONTROL, false)
        return state.send_vehicle_command("brake_level_charging", false)
    return null
