extends Node3D
class_name Cabin3D

var _dirty = true

@export_node_path("Marker3D") var camera_position_path:NodePath = NodePath("")
@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(x):
        if not x == controller_path:
            _dirty = true
            controller_path = x

func get_camera_transform():
    var marker:Marker3D = get_node(camera_position_path)
    if marker:
        return marker.global_transform

func _propagate_train_controller(node: Node, controller: TrainController):
    for child in node.get_children():
        _propagate_train_controller(child, controller)
        if "controller_path" in child:
            if controller:
                child.controller_path = child.get_path_to(controller)
            else:
                child.controller_path = NodePath("")


func set_train_controller(controller:TrainController):
    _propagate_train_controller(self, controller)

func _ready():
    #var controller:TrainController = get_node(controller_path)
    #if controller:
    #    set_train_controller(controller)
    pass

func _process(delta):
    if _dirty:
        _dirty = false
        var controller:TrainController = get_node(controller_path)
        set_train_controller(controller)
