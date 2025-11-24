extends Node

@export var action_name:String = ""
@export var command:String = ""
@export var command_param:String
@export_node_path("TrainNode") var controller_path:NodePath = NodePath("")

func _input(event):
    if action_name and command:
        if event.is_action_pressed(action_name):
            var controller:TrainNode = get_node(controller_path)
            if controller:
                controller.send_command(command, command_param)
