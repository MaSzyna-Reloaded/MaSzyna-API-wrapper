extends RefCounted
class_name LegacyCabinTrainHeating

## Train heating switch (trainheating_sw) - TTrain::OnCommand_heatingtoggle/enable/disable
## (Train.cpp:6658-6717). A press flips HeatingAllow; a push-type switch (type: return,
## dynamic/pkp/e186_v2) springs back when released and switches nothing then. The original does
## nothing at all in a cab without the gauge (Train.cpp:6662).

const CONTROL:StringName = &"trainheating_sw"

var _has_gauge:bool
var _vehicle_rid:RID
var _cab:int


func _init(has_gauge:bool) -> void:
    _has_gauge = has_gauge


func control_ids() -> Array[StringName]:
    return [CONTROL]


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, CONTROL, _heating)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, CONTROL, _heating)


func _heating(state:CabinState, action:StringName, value:Variant) -> Variant:
    if not _has_gauge:
        return null
    if action == &"release":
        state.set_value(CONTROL, false)
        return null
    if not action in [&"hold", &"toggle", &"set"]:
        return null
    # Train.cpp:6674 - a press allows the heating when it is not allowed, and the other way round
    var allowed:bool = (not state.vehicle_state_value("heating_allowed", false)
            if value == null or action == &"hold" else bool(value))
    state.set_value(CONTROL, true if action == &"hold" else allowed)
    return state.send_vehicle_command("heating", allowed)
