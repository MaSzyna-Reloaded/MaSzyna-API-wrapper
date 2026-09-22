extends Node
class_name CabinCommand

@export var action_name:String = ""
@export var command:String = ""
@export var command_param:String
## Cabin control id - the key press is reported to CabinSystem under this id.
@export var control_id:StringName = &""
@export_node_path("VehicleController") var controller_path:NodePath = NodePath("")

func _input(event):
    if action_name and command:
        if event.is_action_pressed(action_name):
            var controller:VehicleController = get_node(controller_path)
            if controller:
                CabinSystem.act(
                        controller.train_id, int(controller.state.get("cabin_occupied", 1)), control_id, &"hold")
