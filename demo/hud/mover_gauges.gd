extends HFlowContainer

## What each gauge's dial is scaled to - these are the ranges the gauges were drawn for, not
## anything the vehicle declares.
const ENGINE_RPM_FULL_SCALE:float = 1400.0
const ENGINE_CURRENT_FULL_SCALE:float = 1500.0
const PRESSURE_FULL_SCALE:float = 10.0
const SPEED_FULL_SCALE:float = 100.0

@export_node_path("VehiclePhysicsNode") var train_controller:NodePath = NodePath(""):
    set(x):
        if not train_controller == x:
            train_controller = x
            controller = null
            _do_update()

@onready var LeftDoorsOpenLight = $VBoxContainer/HBoxContainer2/LeftDoorsOpenLight
@onready var RightDoorsOpenLight = $VBoxContainer/HBoxContainer2/RightDoorsOpenLight

var controller:VehicleController

## Taken once per vehicle, not looked up per frame: what a gauge shows is a typed property of the
## component that owns it, and the component is a live view on the vehicle for as long as the
## vehicle lives.
var _engine:VehicleEngine
var _brakes:VehicleBrake
var _spring_brake:VehicleSpringBrake
var _doors:VehicleDoors
var _security:VehicleSecuritySystem
## The node whose vehicle_changed this panel listens to - one connection, one disconnection.
var _bound_node:VehiclePhysicsNode


func _do_update():
    var physics_node: VehiclePhysicsNode = get_node_or_null(train_controller) if train_controller else null
    if _bound_node and not _bound_node == physics_node:
        _bound_node.vehicle_changed.disconnect(_do_update)
        _bound_node = null
    if physics_node and not _bound_node:
        # the window can be opened before the vehicle is built, and then the components resolved
        # here are null - the vehicle says when it has them rather than being asked again later
        physics_node.vehicle_changed.connect(_do_update)
        _bound_node = physics_node
    controller = physics_node.get_controller() if physics_node else null
    _engine = _component(VehicleComponentType.COMPONENT_ENGINE) as VehicleEngine
    _brakes = _component(VehicleComponentType.COMPONENT_BRAKES) as VehicleBrake
    _spring_brake = _component(VehicleComponentType.COMPONENT_SPRING_BRAKE) as VehicleSpringBrake
    _doors = _component(VehicleComponentType.COMPONENT_DOORS) as VehicleDoors
    _security = _component(VehicleComponentType.COMPONENT_SECURITY) as VehicleSecuritySystem
    _propagate_vehicle_node(self, physics_node)
    modulate = Color.WHITE
    modulate.a = 1.0 if controller else 0.1

func _component(type:VehicleComponentType.Type) -> VehicleComponent:
    return controller.get_component(type) if controller else null


## The widgets below point at the vehicle's node, not at the controller it owns.
func _propagate_vehicle_node(node: Node, physics_node: VehiclePhysicsNode) -> void:
    for child in node.get_children():
        _propagate_vehicle_node(child, physics_node)
        if "controller" in child:
            child.controller = child.get_path_to(physics_node) if physics_node else NodePath("")

func _ready():
    _do_update()

func _process(_delta):
    if not controller:
        return

    if _engine:
        $EngineRPM.value = _engine.get_rpm() / ENGINE_RPM_FULL_SCALE
        $OilPressure.value = _engine.get_oil_pump_pressure()
    var motor:VehicleElectricEngine = _engine as VehicleElectricEngine
    $EngineCurrent.value = motor.get_motor_current() / ENGINE_CURRENT_FULL_SCALE if motor else 0.0
    if _brakes:
        var tank_volume:float = _brakes.get_tank_volume()
        $BrakeCylinderPressure.value = (
                _brakes.get_air_pressure() / tank_volume if tank_volume > 0.0 else 0.0)
        $BrakePipePressure.value = _brakes.get_pipe_pressure() / PRESSURE_FULL_SCALE
    if _spring_brake:
        $SpringBrakePressure.value = _spring_brake.get_cylinder_pressure() / PRESSURE_FULL_SCALE
        $VBoxContainer/HBoxContainer3/SpringBrakeActive.enabled = _spring_brake.get_active()
        $VBoxContainer/HBoxContainer3/SpringBrakeEnabled.enabled = not _spring_brake.get_shut_off()
        %SpringBrakeBraking.enabled = _spring_brake.get_braking()
    $Speed.value = controller.get_speed() / SPEED_FULL_SCALE
    if _security:
        $%SecurityLight.enabled = _security.get_blinking()
        $%SHPLight.enabled = _security.get_cabsignal_blinking()
    if not _doors:
        return
    $"%DoorsLocked".enabled = _doors.get_locked()
    LeftDoorsOpenLight.color_active = Color.ORANGE if _doors.get_left_operating() else Color.LIME_GREEN
    LeftDoorsOpenLight.enabled = _doors.get_left_open() or _doors.get_left_operating()
    RightDoorsOpenLight.color_active = Color.ORANGE if _doors.get_right_operating() else Color.LIME_GREEN
    RightDoorsOpenLight.enabled = _doors.get_right_open() or _doors.get_right_operating()
