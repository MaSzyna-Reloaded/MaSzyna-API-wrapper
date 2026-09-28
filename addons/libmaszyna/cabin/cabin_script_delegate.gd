class_name CabinScriptDelegate
extends ScenarioScriptCabinDelegate

## The scenario scripts' way to the cabs (maszyna.cabin): a script's manipulation goes to
## CabinSystem.act(), as the driver's hand and the AI driver's do, and what changes in a cab goes
## back to the scripts that subscribed to it.


## No disconnect: the engine drops a connection to an object that is freed, and at
## NOTIFICATION_PREDELETE this script's methods are already out of reach
func _init() -> void:
    CabinSystem.control_changed.connect(_on_control_changed)


func _on_control_changed(vehicle_rid:RID, cab:int, control_id:StringName, value:Variant) -> void:
    control_changed.emit(vehicle_rid, cab, control_id, value)


func _act(vehicle:RID, cab:int, control_id:StringName, action:StringName, value:Variant) -> Variant:
    return CabinSystem.act(vehicle, cab, control_id, action, value)


func _get_control(vehicle:RID, cab:int, control_id:StringName) -> Variant:
    return CabinSystem.get_control(vehicle, cab, control_id)


func _get_controls(vehicle:RID, cab:int) -> Array:
    return CabinSystem.get_controls(vehicle, cab)


func _get_occupied_cab(vehicle:RID) -> int:
    return CabinSystem.occupied_cab(vehicle)
