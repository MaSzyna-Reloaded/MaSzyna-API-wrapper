extends RefCounted
class_name LegacyCabinManualBrake

## Manual brake of a cabin whose MMD has no manualbrake gauge. The original cab layer handles
## TTrain::OnCommand_manualbrakeincrease/decrease (Train.cpp:1809-1837) regardless of the gauge,
## so the manual brake stays operable from the keyboard and CabinSystem.

const CONTROL:StringName = &"manualbrake"
const ACTION_INCREASE:StringName = &"manual_brake_increase"
const ACTION_DECREASE:StringName = &"manual_brake_decrease"

var _train_id:String
var _cab:int


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, CONTROL, _manual_brake)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, CONTROL, _manual_brake)


func _manual_brake(state:CabinState, action:StringName, _value:Variant) -> Variant:
    if action == &"increase":
        return state.send_vehicle_command("manual_brake_increase")
    if action == &"decrease":
        return state.send_vehicle_command("manual_brake_decrease")
    return null
