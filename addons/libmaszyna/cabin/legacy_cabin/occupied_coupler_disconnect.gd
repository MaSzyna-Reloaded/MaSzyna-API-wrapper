extends RefCounted
class_name LegacyCabinOccupiedCouplerDisconnect

## Uncoupling at the occupied cab's end, a keyboard-only control of the original cab layer:
## TTrain::OnCommand_occupiedcarcouplingdisconnect (Train.cpp:6285) uncouples at the end the cab
## faces (cab_to_end(), Train.h:216), with or without a couplingdisconnect_sw: gauge. A control of
## its own, so the AI uncouples the way a player does. Cab 0 faces no end.

const CONTROL:StringName = &"coupler_disconnect_occupied"
const ACTION:StringName = &"coupler_disconnect_occupied"
## The couplers of the vehicle (end::front, end::rear)
const FRONT_END:int = 0
const REAR_END:int = 1

var _vehicle_rid:RID
var _cab:int


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, CONTROL, _disconnect)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, CONTROL, _disconnect)


func _disconnect(state:CabinState, action:StringName, _value:Variant) -> Variant:
    if not action == &"hold" or state.cab == 0:
        return null
    return state.send_vehicle_command("coupler_disconnect", REAR_END if state.cab < 0 else FRONT_END)
