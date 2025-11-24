extends HBoxContainer

@export_node_path("TrainNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()

@onready var FORWARD = $General/HBoxContainer2/Forward
@onready var REVERSE = $General/HBoxContainer2/Reverse

var controller:TrainNode

func _do_update():
    if train_controller:
        controller = get_node(train_controller)
    _propagate_train_controller(self, controller)
    modulate = Color.WHITE
    modulate.a = 1.0 if controller else 0.1

func _propagate_train_controller(node: Node, controller: TrainNode):
    for child in node.get_children():
        _propagate_train_controller(child, controller)
        if "controller" in child:
            child.controller = child.get_path_to(controller) if controller else NodePath("")

func _ready():
    _do_update()

func _process(delta):
    if controller:
        var state = controller.state

        FORWARD.modulate = Color.GREEN if state.get("direction", 0) > 0 else Color.WHITE
        REVERSE.modulate = Color.GREEN if state.get("direction", 0) < 0 else Color.WHITE
