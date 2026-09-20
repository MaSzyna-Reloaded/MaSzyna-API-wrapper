extends RefCounted
class_name LegacyCabinWipers

## Wiper switch of a cabin whose MMD has no wipers_sw gauge. The original cab layer handles
## TTrain::OnCommand_wiperswitchincrease/decrease (Train.cpp:2638-2661) regardless of the gauge,
## so the wipers stay operable from the keyboard and CabinSystem.

const CONTROL:StringName = &"wipers_sw"
const ACTION_INCREASE:StringName = &"wipers_switch_increase"
const ACTION_DECREASE:StringName = &"wipers_switch_decrease"

var _train_id:String
var _cab:int


func register(train_id:String, cab:int) -> void:
    _train_id = train_id
    _cab = cab
    CabinSystem.register_control(train_id, cab, CONTROL, _wipers)


func unregister() -> void:
    CabinSystem.unregister_control(_train_id, _cab, CONTROL, _wipers)


func _wipers(state:CabinState, action:StringName, _value:Variant) -> Variant:
    if action == &"increase":
        return state.send_vehicle_command("wipers_switch_increase")
    if action == &"decrease":
        return state.send_vehicle_command("wipers_switch_decrease")
    return null
