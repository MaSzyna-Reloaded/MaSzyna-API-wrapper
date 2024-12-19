extends HFlowContainer

@export_node_path("TrainController") var train_controller:NodePath = NodePath("")



@onready var LeftDoorsOpenLight = $VBoxContainer/HBoxContainer2/LeftDoorsOpenLight
@onready var RightDoorsOpenLight = $VBoxContainer/HBoxContainer2/RightDoorsOpenLight



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

    $EngineRPM.value = state.get("engine_rpm", 0.0) / 1400.0
    $EngineCurrent.value = state.get("engine_current", 0.0) / 1500.0
    $OilPressure.value = state.get("oil_pump_pressure", 0.0)
    $BrakeCylinderPressure.value = state.get("brake_air_pressure", 0.0) / state.get("brake_tank_volume", 1.0)
    $BrakePipePressure.value = state.get("pipe_pressure", 0.0)  / 10.0
    $Speed.value = state.get("speed", 0.0) / 100.0
    $%SecurityLight.enabled = true if state.get("blinking") else false
    $%SHPLight.enabled = true if state.get("cabsignal_blinking") else false
    $"%DoorsLocked".enabled = true if state.get("doors_locked") else false

    LeftDoorsOpenLight.color_active = Color.ORANGE if state.get("doors_left_operating") else Color.LIME_GREEN
    LeftDoorsOpenLight.enabled = state.get("doors_left_open") or state.get("doors_left_operating")
    RightDoorsOpenLight.color_active = Color.ORANGE if state.get("doors_right_operating") else Color.LIME_GREEN
    RightDoorsOpenLight.enabled = state.get("doors_right_open") or state.get("doors_right_operating")
