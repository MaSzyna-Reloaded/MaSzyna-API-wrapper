extends Node
class_name CabinCommand

## A keyboard action reported to CabinSystem as a manipulation of a cabin control. It knows which
## vehicle it belongs to and nothing else about it - the system decides what the manipulation does.

@export var action_name:String = ""
@export var command:String = ""
@export var command_param:String
## Cabin control id - the key press is reported to CabinSystem under this id.
@export var control_id:StringName = &""

var _vehicle_rid:RID


## The vehicle this element belongs to, as the cabin root hands it down.
func set_vehicle_rid(vehicle_rid:RID) -> void:
    _vehicle_rid = vehicle_rid


func _input(event):
    if not _vehicle_rid or not action_name or not command:
        return
    if event.is_action_pressed(action_name):
        CabinSystem.act(_vehicle_rid, CabinSystem.occupied_cab(_vehicle_rid), control_id, &"hold")
