extends RefCounted
class_name LegacyCabinDoorPermits

## The door permit switches, ported from the original cab layer: TTrain::OnCommand_doorpermitleft/
## right (Train.cpp:7196-7294) and the door permit timers of TTrain::Update() (Train.cpp:8475-8487).
##
## Left and right are the driver's - from the rear cab they permit the other side of the vehicle.
## A vehicle with door permit presets ignores them, its preset switch permits the doors. What a
## press does depends on the kind of switch:
## * push - permits its side, and held for DoorsOpenWithPermitAfter opens those doors as well;
## * two-state - a press flips the permit of its side.

const LEFT_SWITCH:StringName = &"doorleftpermit_sw"
const RIGHT_SWITCH:StringName = &"doorrightpermit_sw"
## A permit timer that does not run (Train.cpp:7241)
const TIMER_STOPPED:float = -1.0
const PERMIT_COMMANDS:Dictionary[RailVehicleDoors.Side, String] = {
    RailVehicleDoors.SIDE_LEFT: "doors_left_permit",
    RailVehicleDoors.SIDE_RIGHT: "doors_right_permit",
}
const OPEN_COMMANDS:Dictionary[RailVehicleDoors.Side, String] = {
    RailVehicleDoors.SIDE_LEFT: "doors_left",
    RailVehicleDoors.SIDE_RIGHT: "doors_right",
}
## state.data keys of m_doorpermittimers, by the side of the vehicle
const TIMERS:Dictionary[RailVehicleDoors.Side, String] = {
    RailVehicleDoors.SIDE_LEFT: "door_permit_timer_left",
    RailVehicleDoors.SIDE_RIGHT: "door_permit_timer_right",
}

var _left_button_type:CabinButton.ButtonType
var _right_button_type:CabinButton.ButtonType
var _vehicle_rid:RID
var _cab:int


func _init(left_button_type:CabinButton.ButtonType, right_button_type:CabinButton.ButtonType) -> void:
    _left_button_type = left_button_type
    _right_button_type = right_button_type


func control_ids() -> Array[StringName]:
    return [LEFT_SWITCH, RIGHT_SWITCH]


func register(vehicle_rid:RID, cab:int) -> void:
    _vehicle_rid = vehicle_rid
    _cab = cab
    CabinSystem.register_control(vehicle_rid, cab, LEFT_SWITCH, _left_switch)
    CabinSystem.register_control(vehicle_rid, cab, RIGHT_SWITCH, _right_switch)
    CabinSystem.register_process(vehicle_rid, cab, _process)


func unregister() -> void:
    CabinSystem.unregister_control(_vehicle_rid, _cab, LEFT_SWITCH, _left_switch)
    CabinSystem.unregister_control(_vehicle_rid, _cab, RIGHT_SWITCH, _right_switch)
    CabinSystem.unregister_process(_vehicle_rid, _cab, _process)


# Train.cpp:7208 - cab_to_end(): the rear cab is cab 2
func _left_switch(state:CabinState, action:StringName, value:Variant) -> Variant:
    var side:RailVehicleDoors.Side = RailVehicleDoors.SIDE_RIGHT if _cab < 0 else RailVehicleDoors.SIDE_LEFT
    return _permit(state, LEFT_SWITCH, _left_button_type, side, action, value)


func _right_switch(state:CabinState, action:StringName, value:Variant) -> Variant:
    var side:RailVehicleDoors.Side = RailVehicleDoors.SIDE_LEFT if _cab < 0 else RailVehicleDoors.SIDE_RIGHT
    return _permit(state, RIGHT_SWITCH, _right_button_type, side, action, value)


func _permit(state:CabinState, control:StringName, button_type:CabinButton.ButtonType,
        side:RailVehicleDoors.Side, action:StringName, value:Variant) -> Variant:
    # Train.cpp:7203 - the presets permit the doors
    if int(state.vehicle_state_value("doors_permit_preset_count", 0)) > 0:
        return null
    if button_type == CabinButton.ButtonType.PUSH:
        var pressed:bool = state.is_pressed(control, action, value)
        state.set_value(control, pressed)
        if not pressed:
            state.data[TIMERS[side]] = TIMER_STOPPED
            return null
        state.data[TIMERS[side]] = float(state.vehicle_state_value("doors_open_with_permit_after", TIMER_STOPPED))
        return state.send_vehicle_command(PERMIT_COMMANDS[side], true)
    # two-state: only a press counts, and flips the switch (Train.cpp:7225)
    if action == &"release":
        return null
    var permitted:bool = (not bool(state.get_value(control, false))
            if value == null or action == &"hold" else bool(value))
    state.set_value(control, permitted)
    return state.send_vehicle_command(PERMIT_COMMANDS[side], permitted)


# Train.cpp:8475-8487 - a push switch held long enough opens the doors of its side
func _process(state:CabinState, delta:float) -> void:
    for side:RailVehicleDoors.Side in TIMERS:
        var timer:float = state.data.get(TIMERS[side], TIMER_STOPPED)
        if timer < 0.0:
            continue
        timer -= delta
        state.data[TIMERS[side]] = timer
        if timer < 0.0:
            state.send_vehicle_command(OPEN_COMMANDS[side], true)
