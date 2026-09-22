extends HBoxContainer

@export_node_path("VehiclePhysicsNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()


var controller:VehicleController

func _do_update():
    if train_controller:
        var physics_node: VehiclePhysicsNode = get_node_or_null(train_controller)
        controller = physics_node.get_controller() if physics_node else null
    _propagate_train_controller(self, controller)
    modulate = Color.WHITE
    modulate.a = 1.0 if controller else 0.1

func _propagate_train_controller(node: Node, controller: VehicleController):
    for child in node.get_children():
        if "train_controller" in child:
            child.train_controller = child.get_path_to(controller) if controller else NodePath("")

func _ready():
    _do_update()
