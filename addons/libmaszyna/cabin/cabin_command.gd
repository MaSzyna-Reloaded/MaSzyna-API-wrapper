extends Node

@export var action_name:String = ""
@export var command:String = ""
@export var command_param:String
@export_node_path("RailVehicle3D") var controller_path:NodePath = NodePath("")

func _input(event):
    if action_name and command:
        if event.is_action_pressed(action_name):
            var vehicle := get_node_or_null(controller_path) as RailVehicle3D
            if vehicle:
                vehicle.send_command(command, command_param)
