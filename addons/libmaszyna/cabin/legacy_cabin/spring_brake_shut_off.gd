extends RefCounted
class_name LegacyCabinSpringBrakeShutOff

## The spring brake's shut-off valve, a keyboard-only control of the original cab layer:
## TTrain::OnCommand_springbrakeshutofftoggle (Train.cpp:6836) - no cab models the valve. A control
## of its own, so the AI turns it the way a player does.

const CONTROL:StringName = &"spring_brake_shut_off_toggle"
const ACTION:StringName = &"spring_brake_shut_off_toggle"

var _vehicle_rid:RID
var _cab:int


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, CONTROL, _toggle)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, CONTROL, _toggle)


func _toggle(state:CabinState, action:StringName, _value:Variant) -> Variant:
    if not action == &"hold":
        return null
    # the command takes "enabled", the opposite of the valve, so the valve's state is the new value
    return state.send_vehicle_command(
            "set_spring_brake_enabled", bool(state.vehicle_state_value("spring_brake/shut_off", false)))
