extends Node3D
class_name Cabin3D

signal cabin_ready

var _dirty = true
var _cabin_ready:bool = false
var _e3d_instances:Array[E3DModelInstance] = []
var _e3d_loaded_count:int = 0
var _shake_controller:TrainController
var _engine_angle:float = PI * 0.5
var _shake_velocity:Vector3 = Vector3.ZERO
var _shake_offset:Vector3 = Vector3.ZERO
var _shake_accumulator:float = 0.0

const SHAKE_STEP:float = 1.0 / 50.0
const SPRING_REST_LENGTH:float = 0.01

@export var cab_number:int = 1
@export var cab_window_open:bool = false

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(x):
        if not x == controller_path:
            _dirty = true
            controller_path = x

@export var camera_bound_min = Vector3.ZERO
@export var camera_bound_max = Vector3.ZERO
@export var camera_bound_enabled:bool = false
@export var driver_position = Vector3.ZERO

@export_group("Camera Shake")
@export var shake_spring_stiffness:float = 125.0
@export var shake_spring_damping:float = 0.002
@export var shake_jolt_scale:Vector3 = Vector3(0.2, 0.2, 0.1)
@export var shake_jolt_limit:float = 0.15
@export var shake_angle_scale:Vector2 = Vector2(0.05, 0.1)
@export var engine_shake_scale:float = 2.0
@export var engine_shake_fade_in_rpm:float = 90.0
@export var engine_shake_fade_in_factor:float = 0.3
@export var engine_shake_fade_out_rpm:float = 600.0
@export var engine_shake_fade_out_factor:float = 0.5

func get_camera_transform():
    return global_transform.translated_local(driver_position)

func _propagate_train_controller(node: Node, controller: TrainController):
    for child in node.get_children():
        _propagate_train_controller(child, controller)
        if "controller_path" in child:
            if controller:
                child.controller_path = child.get_path_to(controller)
            else:
                child.controller_path = NodePath("")

func set_train_controller(controller:TrainController):
    _propagate_train_controller(self, controller)


func get_sound_listener_context() -> int:
    if cab_window_open:
        return 3
    return 2 if cab_number > 0 else 0


func _process(delta:float) -> void:
    _process_dirty()
    _shake_accumulator += delta
    while _shake_accumulator >= SHAKE_STEP:
        _process_engine_shake(SHAKE_STEP)
        _shake_accumulator -= SHAKE_STEP

func _process_dirty() -> void:
    if not _dirty:
        return
    _dirty = false
    if controller_path:
        _shake_controller = get_node(controller_path)
        set_train_controller(_shake_controller)

func _process_engine_shake(delta:float) -> void:
    var shake_vector:Vector3 = Vector3.ZERO
    if _shake_controller and _shake_controller.config.get("engine_shake_enabled", false):
        var engine_revolutions:float = absf(float(_shake_controller.state.get("engine_rpm_count", 0.0)))
        if engine_revolutions > 0.0:
            _engine_angle = fmod(_engine_angle + engine_revolutions * delta, TAU)
            var fade_in:float = clampf(
                    (engine_revolutions - engine_shake_fade_in_rpm / 60.0) * engine_shake_fade_in_factor,
                    0.0, 1.0)
            var fade_out:float = 1.0 - clampf(
                    (engine_revolutions - engine_shake_fade_out_rpm / 60.0) * engine_shake_fade_out_factor,
                    0.0, 1.0)
            shake_vector.x = sin(_engine_angle * 4.0) * delta * engine_shake_scale * fade_in * fade_out

    var spring_delta:Vector3 = shake_vector - _shake_offset
    var distance:float = spring_delta.length()
    var spring_force:Vector3 = Vector3.ZERO
    if distance > SPRING_REST_LENGTH:
        var force:float = (distance - SPRING_REST_LENGTH) * shake_spring_stiffness
        force += distance * shake_spring_damping
        spring_force = spring_delta / distance * -force

    var shake:Vector3 = spring_force * 1.0625
    var damping:float = (shake_jolt_scale.x + shake_jolt_scale.y + shake_jolt_scale.z) / 200.0
    _shake_velocity -= (shake + _shake_velocity * 100.0) * damping
    _shake_offset += _shake_velocity * delta
    if absf(_shake_offset.y) > absf(shake_jolt_limit):
        _shake_velocity.y = -_shake_velocity.y

func get_camera_shake_offset() -> Vector3:
    return _shake_offset

func get_camera_shake_roll() -> float:
    return atan(_shake_velocity.x * shake_angle_scale.x)
            
func _ready() -> void:
    _cabin_ready = true
    cabin_ready.emit()                

func is_cabin_ready() -> bool:
    return _cabin_ready
