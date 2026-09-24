extends HFlowContainer


@export_node_path("VehiclePhysicsNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()

var controller:VehicleController
## Taken once per vehicle rather than looked up per frame - a component is a live view on the
## vehicle, valid for as long as the vehicle is.
var universal_controller:VehicleUniversalController
## The node whose vehicle_changed this panel listens to - one connection, one disconnection.
var _bound_node:VehiclePhysicsNode


func _ready() -> void:
    _do_update()

func _do_update():
    var physics_node: VehiclePhysicsNode = get_node_or_null(train_controller) if train_controller else null
    if _bound_node and not _bound_node == physics_node:
        _bound_node.vehicle_changed.disconnect(_do_update)
        _bound_node = null
    if physics_node and not _bound_node:
        # the window can be opened before the vehicle is built, and then the component resolved
        # here is null - the vehicle says when it has one rather than being asked again later
        physics_node.vehicle_changed.connect(_do_update)
        _bound_node = physics_node
    controller = physics_node.get_controller() if physics_node else null
    universal_controller = (
            controller.get_component(VehicleComponentType.COMPONENT_UNIVERSAL_CONTROLLER)
            as VehicleUniversalController) if controller else null
    _propagate_vehicle_node(self, physics_node)

## The widgets below point at the vehicle's node, not at the controller it owns - a controller is
## not a node and has no path.
func _propagate_vehicle_node(p_node: Node, p_physics_node: VehiclePhysicsNode) -> void:
    for child in p_node.get_children():
        _propagate_vehicle_node(child, p_physics_node)
        if "controller" in child:
            child.controller = child.get_path_to(p_physics_node) if p_physics_node else NodePath("")


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
