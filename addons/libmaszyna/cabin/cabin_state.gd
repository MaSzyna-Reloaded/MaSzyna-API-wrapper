extends RefCounted
class_name CabinState

## State of one cabin of one train - (train_id, cab), where cab is 1 (cab 1), 0 (machine room) or
## -1 (cab 2), matching VehicleController.state["cabin_occupied"]. Owned by CabinSystem and handed to
## every registered cabin control handler, which may read and modify it.
##
## The train is reached only through the high-level API (TrainSystem commands and state), never
## through the Mover directly.

var train_id:String = ""
var cab:int = 1
## control_id -> current value of the physical control (button pressed, switch position, ...)
var values:Dictionary = {}
## Private state of the cabin logic behaviours (timers, state machines, ...)
var data:Dictionary = {}


func _init(p_train_id:String, p_cab:int) -> void:
    train_id = p_train_id
    cab = p_cab


func get_value(control_id:StringName, default:Variant = null) -> Variant:
    return values.get(control_id, default)


func set_value(control_id:StringName, value:Variant) -> void:
    if values.has(control_id) and values[control_id] == value:
        return
    values[control_id] = value
    CabinSystem.control_changed.emit(train_id, cab, control_id, value)


## Whether a push control ends up pressed by the manipulation: held, or toggled to the given value -
## a toggle without one (e.g. from the console) flips its current position.
func is_pressed(control_id:StringName, action:StringName, value:Variant) -> bool:
    if action == &"toggle":
        return not get_value(control_id, false) if value == null else bool(value)
    return action == &"hold"


# FIXME(#57): reads the per-frame copied VehicleController.state; see the refresh workaround in
# TrainSystem::send_command for why a read right after a command could be stale.
func vehicle_state() -> Dictionary:
    return TrainSystem.get_train_state(train_id)


func send_vehicle_command(command:String, p1:Variant = null, p2:Variant = null) -> Variant:
    return TrainSystem.send_command(train_id, command, p1, p2)
