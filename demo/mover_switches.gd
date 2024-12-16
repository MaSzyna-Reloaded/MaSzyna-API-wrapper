extends HBoxContainer

@export_node_path("TrainController") var train_controller:NodePath = NodePath("")
@onready var FORWARD = $General/HBoxContainer2/Forward
@onready var REVERSE = $General/HBoxContainer2/Reverse

func _propagate_train_controller(node: Node, controller: TrainController):
    for child in node.get_children():
        _propagate_train_controller(child, controller)
        if "controller" in child:
            child.controller = child.get_path_to(controller)

var controller:TrainController

func _ready():
    controller = get_node(train_controller)
    _propagate_train_controller(self, controller)


func _process(delta):
    var state = controller.state

    FORWARD.modulate = Color.GREEN if state.get("direction", 0) > 0 else Color.WHITE
    REVERSE.modulate = Color.GREEN if state.get("direction", 0) < 0 else Color.WHITE
