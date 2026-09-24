extends HFlowContainer

@export_node_path("VehiclePhysicsNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()

@onready var LeftDoorsOpenLight = $VBoxContainer/HBoxContainer2/LeftDoorsOpenLight
@onready var RightDoorsOpenLight = $VBoxContainer/HBoxContainer2/RightDoorsOpenLight

var controller:VehicleController

func _do_update():
    var physics_node: VehiclePhysicsNode = get_node_or_null(train_controller) if train_controller else null
    controller = physics_node.get_controller() if physics_node else null
    _propagate_vehicle_node(self, physics_node)
    modulate = Color.WHITE
    modulate.a = 1.0 if controller else 0.1

## The widgets below point at the vehicle's node, not at the controller it owns.
func _propagate_vehicle_node(node: Node, physics_node: VehiclePhysicsNode) -> void:
    for child in node.get_children():
        _propagate_vehicle_node(child, physics_node)
        if "controller" in child:
            child.controller = child.get_path_to(physics_node) if physics_node else NodePath("")

func _ready():
    _do_update()

func _process(delta):
    if not controller:
        return

    var state = controller.state

    $EngineRPM.value = state.get("engine_rpm", 0.0) / 1400.0
    $EngineCurrent.value = state.get("engine_current", 0.0) / 1500.0
    $OilPressure.value = state.get("oil_pump_pressure", 0.0)
    $BrakeCylinderPressure.value = state.get("brake_air_pressure", 0.0) / state.get("brake_tank_volume", 1.0)
    $BrakePipePressure.value = state.get("pipe_pressure", 0.0)  / 10.0
    $SpringBrakePressure.value = state.get("spring_brake/cylinder_pressure", 0.0)  / 10.0
    $Speed.value = state.get("speed", 0.0) / 100.0
    $%SecurityLight.enabled = true if state.get("blinking") else false
    $%SHPLight.enabled = true if state.get("cabsignal_blinking") else false
    $"%DoorsLocked".enabled = true if state.get("doors_locked") else false
    $VBoxContainer/HBoxContainer3/SpringBrakeActive.enabled = true if state.get("spring_brake/active") else false
    $VBoxContainer/HBoxContainer3/SpringBrakeEnabled.enabled = not state.get("spring_brake/shut_off", true)

    LeftDoorsOpenLight.color_active = Color.ORANGE if state.get("doors_left_operating") else Color.LIME_GREEN
    LeftDoorsOpenLight.enabled = state.get("doors_left_open") or state.get("doors_left_operating")
    RightDoorsOpenLight.color_active = Color.ORANGE if state.get("doors_right_operating") else Color.LIME_GREEN
    RightDoorsOpenLight.enabled = state.get("doors_right_open") or state.get("doors_right_operating")
