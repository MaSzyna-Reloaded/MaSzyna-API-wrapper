extends HFlowContainer


@export_node_path("VehiclePhysicsNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()

var controller:VehicleController

func _ready() -> void:
    _do_update()

func _do_update():
    if train_controller:
        var physics_node: VehiclePhysicsNode = get_node_or_null(train_controller)
        controller = physics_node.get_controller() if physics_node else null
    _propagate_train_controller(self, controller)
    
func _propagate_train_controller(p_node: Node, p_controller: VehicleController):
    for child in p_node.get_children():
        _propagate_train_controller(child, p_controller)
        if "controller" in child:
            child.controller = child.get_path_to(p_controller) if p_controller else NodePath("")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
