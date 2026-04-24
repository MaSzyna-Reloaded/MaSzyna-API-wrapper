extends HFlowContainer


@export_node_path("RailVehicle3D") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()

var controller:RailVehicle3D

func _ready() -> void:
    _do_update()

func _do_update():
    if train_controller:
        controller = get_node(train_controller)
    _propagate_train_controller(self, controller)
    
func _propagate_train_controller(p_node: Node, p_controller: TrainController):
    for child in p_node.get_children():
        _propagate_train_controller(child, p_controller)
        if "controller" in child:
            child.controller = child.get_path_to(p_controller) if p_controller else NodePath("")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
