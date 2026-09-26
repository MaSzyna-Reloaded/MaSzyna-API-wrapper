extends RefCounted
class_name CabinState

## State of one cabin of one vehicle - (vehicle_rid, cab), where cab is 1 (cab 1), 0 (machine room)
## or -1 (cab 2), matching VehicleController.state["cabin_occupied"]. Owned by CabinSystem and
## handed to every registered cabin control handler, which may read and modify it.
##
## The vehicle is reached only by its RailVehicleServer handle (commands and state), never through
## the Mover directly.

var vehicle_rid:RID
var cab:int = 1
## control_id -> current value of the physical control (button pressed, switch position, ...)
var values:Dictionary = {}
## Private state of the cabin logic behaviours (timers, state machines, ...)
var data:Dictionary = {}


func _init(p_vehicle_rid:RID, p_cab:int) -> void:
    vehicle_rid = p_vehicle_rid
    cab = p_cab


func get_value(control_id:StringName, default:Variant = null) -> Variant:
    return values.get(control_id, default)


func set_value(control_id:StringName, value:Variant) -> void:
    if values.has(control_id) and values[control_id] == value:
        return
    values[control_id] = value
    CabinSystem.control_changed.emit(vehicle_rid, cab, control_id, value)


## Whether a push control ends up pressed by the manipulation: held, or toggled to the given value -
## a toggle without one (e.g. from the console) flips its current position.
func is_pressed(control_id:StringName, action:StringName, value:Variant) -> bool:
    if action == &"toggle":
        return not get_value(control_id, false) if value == null else bool(value)
    return action == &"hold"


## The vehicle's state, through the cab's own system. RailVehicleServer builds the dump once per
## step for every element of every cab, and a read right after a command is not stale either - the
## dump is keyed on the vehicle's command counter as well as on the step (see `FINDINGS.md`,
## 2026-09-23).
func vehicle_state() -> Dictionary:
    return CabinSystem.vehicle_state(vehicle_rid)


## One named value of it - what a control driven by an MMD property name actually reads.
func vehicle_state_value(key:String, default_value:Variant = null) -> Variant:
    return CabinSystem.vehicle_state_value(vehicle_rid, key, default_value)


func send_vehicle_command(command:String, p1:Variant = null, p2:Variant = null) -> Variant:
    return RailVehicleServer.vehicle_send_command(vehicle_rid, command, p1, p2)
