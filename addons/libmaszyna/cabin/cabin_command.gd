extends Node
class_name CabinCommand

## A keyboard action reported to CabinSystem as a manipulation of a cabin control. It knows which
## vehicle it belongs to and nothing else about it - the system decides what the manipulation does.

@export var action_name:String = ""
@export var command:String = ""
@export var command_param:String
## Cabin control id - the key press is reported to CabinSystem under this id.
@export var control_id:StringName = &""

var _train_id:String = ""


## The vehicle this element belongs to, as the cabin root hands it down.
func set_vehicle(train_id:String) -> void:
    _train_id = train_id


func _input(event):
    if not _train_id or not action_name or not command:
        return
    if event.is_action_pressed(action_name):
        CabinSystem.act(_train_id, CabinSystem.occupied_cab(_train_id), control_id, &"hold")
