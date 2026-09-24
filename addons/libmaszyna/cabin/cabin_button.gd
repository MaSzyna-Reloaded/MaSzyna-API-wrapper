extends BaseCabinTool3D
class_name CabinButton

signal pushed_changed()
signal button_pushed()

enum ControllerMode { OnOff, On, Off }

@export var pushed:bool = false:
    set(x):
        if not x == pushed:
            pushed = x
            _update_mesh_target()
            emit_signal("pushed_changed")

@export var monostable:bool = false
@export_node_path("MeshInstance3D") var mesh_path:NodePath = "":
    set(x):
        mesh_path = x
        _mesh = null
        _dirty = true

@export var command = ""
## Fixed leading argument sent before `pushed`, for commands that take a selector as their first
## parameter (e.g. VehicleElectricEngine::pantograph(PantographSelector, bool)) - unset (null) for
## every single-argument command, which keeps existing widgets (fuelpump_sw, battery_sw, ...)
## sending exactly the same single-argument call as before.
@export var command_param:Variant
@export var state_property = ""
@export var controller_mode:ControllerMode = ControllerMode.OnOff
@export var mesh_position:Vector3 = Vector3.ZERO:
    set(x):
        mesh_position = x
        _update_mesh_target()
@export var mesh_rotation:Vector3 = Vector3.ZERO:
    set(x):
        mesh_rotation = x
        _update_mesh_target()
## Pose of the released button, the MMD offset (value * scale + offset, Gauge.cpp:456). A model
## authored in its pushed pose rests displaced from it, e.g. E186's vigilance pedal
## (pedal_sifa rot 0.008 -0.008).
@export var mesh_position_offset:Vector3 = Vector3.ZERO:
    set(x):
        mesh_position_offset = x
        _update_mesh_target()
@export var mesh_rotation_offset:Vector3 = Vector3.ZERO:
    set(x):
        mesh_rotation_offset = x
        _update_mesh_target()
@export var speed = 10.0
@export var sound_on:AudioStream
@export var sound_off:AudioStream
@export var sound_max_distance:float = 3.0:
    set(x):
        sound_max_distance = x
        _sound.max_distance = x

@export var action = ""

var _mesh:Node3D
var _mesh_original_basis:Basis
var _mesh_original_position:Vector3 = Vector3.ZERO
var _target_mesh_rotation:Vector3 = Vector3.ZERO
var _target_mesh_position:Vector3 = Vector3.ZERO
var _current_rotation:Vector3 = Vector3.ZERO
var _current_position:Vector3 = Vector3.ZERO
var _t:float = 0.0

var _enabled:bool = true
var _setup_phase:bool = true
var _sound:AudioStreamPlayer3D = AudioStreamPlayer3D.new()

func _ready():
    add_child(_sound)
    _sound.max_distance = sound_max_distance
    connect("pushed_changed", self._on_pushed_changed)
    train_id_changed.connect(_update_state)
    Console.console_toggled.connect(_on_console_toggled)

func _enter_tree():
    _setup_phase = true

func _on_console_toggled(visible:bool):
    _enabled = not visible

func _update_state():
    if state_property and _train_id:
        pushed = _vehicle_state_value(state_property, pushed)
    else:
        pushed = false

func _input(event):
    if _enabled and action:
        if monostable:
            if event.is_action_pressed(action, false, true):
                pushed = true
            elif event.is_action_released(action, true):
                pushed = false
        else:
            if event.is_action_pressed(action, false, true):
                pushed = not pushed

func _update_mesh_target() -> void:
    _target_mesh_position = mesh_position_offset + (mesh_position if pushed else Vector3.ZERO)
    _target_mesh_rotation = mesh_rotation_offset + (mesh_rotation if pushed else Vector3.ZERO)

func _process_dirty(delta):
    if not _mesh and mesh_path:
        _mesh = get_node_or_null(mesh_path)
        if _mesh:
            global_position = _mesh.global_position
            _mesh_original_basis = _mesh.transform.basis
            _mesh_original_position = _mesh.position
    _update_state()

func _process_tool(delta):
    _t += delta
    if _t > 0.1:
        if _setup_phase and mesh_path and not _mesh:
            _dirty = true

    if _setup_phase and _mesh:
        _setup_phase = false
        _current_position = _target_mesh_position
        _current_rotation = _target_mesh_rotation
    else:
        _current_rotation = _current_rotation.lerp(_target_mesh_rotation, delta * speed)
        _current_position = _current_position.lerp(_target_mesh_position, delta * speed)

    if is_instance_valid(_mesh):
        var new_basis = _mesh_original_basis
        new_basis *= Basis(Vector3.RIGHT, deg_to_rad(_current_rotation.x))
        new_basis *= Basis(Vector3.UP, deg_to_rad(_current_rotation.y))
        new_basis *= Basis(Vector3.FORWARD, deg_to_rad(_current_rotation.z))

        _mesh.transform.basis = new_basis
        _mesh.position = _mesh_original_position + _current_position

func _apply_control_value(p_value:Variant) -> void:
    pushed = bool(p_value)

func _play_sound():
    _sound.stream = sound_on if pushed else sound_off
    if _sound.stream:
        _sound.play()

func _on_pushed_changed():
    if pushed:
        button_pushed.emit()

    if monostable:
        _act(&"hold" if pushed else &"release")
    else:
        _act(&"toggle", pushed)

    _play_sound()
