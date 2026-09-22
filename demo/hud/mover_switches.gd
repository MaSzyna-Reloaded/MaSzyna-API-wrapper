extends HBoxContainer

@export_node_path("VehiclePhysicsNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()


var controller:VehicleController

func _do_update():
    var physics_node: VehiclePhysicsNode = get_node_or_null(train_controller) if train_controller else null
    controller = physics_node.get_controller() if physics_node else null
    _propagate_vehicle_node(self, physics_node)
    modulate = Color.WHITE
    modulate.a = 1.0 if controller else 0.1

## The sections below point at the vehicle's node, not at the controller it owns.
func _propagate_vehicle_node(node: Node, physics_node: VehiclePhysicsNode) -> void:
    for child in node.get_children():
        if "train_controller" in child:
            child.train_controller = child.get_path_to(physics_node) if physics_node else NodePath("")

func _ready():
    _do_update()
