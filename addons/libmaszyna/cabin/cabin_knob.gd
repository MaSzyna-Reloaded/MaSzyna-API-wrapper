extends BaseCabinTool3D
class_name CabinKnob

signal value_changed()

enum ControllerMode { OnOff, On, Off }

@export var value:float = 0.0:
    set(x):
        if not x == value:
            value = x
            emit_signal("value_changed")

@export var value_min:float = 0.0;
@export var value_max:float = 1.0;

@export_node_path("MeshInstance3D") var mesh_path:NodePath = "":
    set(x):
        mesh_path = x
        _mesh = null
        _dirty = true

@export var command = ""
@export var state_property = ""
@export var config_min_property = ""
@export var config_max_property = ""

@export var mesh_position:Vector3 = Vector3.ZERO:
    set(x):
        mesh_position = x
        _target_mesh_position = x
        _dirty = true

@export var mesh_rotation:Vector3 = Vector3.ZERO:
    set(x):
        mesh_rotation = x
        _target_mesh_rotation = x
        _dirty = true

@export var speed = 10.0
@export var step = 1.0

@export var action_increase = ""
@export var action_decrease = ""

var _mesh:MeshInstance3D
var _mesh_original_rotation:Vector3 = Vector3.ZERO
var _mesh_original_basis:Basis
var _mesh_original_position:Vector3 = Vector3.ZERO
var _target_mesh_rotation:Vector3 = Vector3.ZERO
var _target_mesh_position:Vector3 = Vector3.ZERO
var _current_rotation:Vector3 = Vector3.ZERO
var _current_position:Vector3 = Vector3.ZERO

var _sound:AudioStreamPlayer3D = AudioStreamPlayer3D.new()

var _t = 0.0
var _value_normalized = 0.0
var _handle_actions:bool = true
var _setup_phase:bool = true

func _enter_tree():
    _setup_phase = true

func _update_state():
    if _controller:
        if config_min_property:
            value_min = _controller.config.get(config_min_property, value_min)
        if config_max_property:
            value_max = _controller.config.get(config_max_property, value_max)
        if state_property:
            value = _controller.state.get(state_property, value)
        _value_normalized = value / (value_max - value_min)
        _target_mesh_position = mesh_position * _value_normalized
        _target_mesh_rotation = mesh_rotation * _value_normalized

func _ready():
    _sound.doppler_tracking = AudioStreamPlayer3D.DOPPLER_TRACKING_PHYSICS_STEP
    if not Engine.is_editor_hint() and Console:
        Console.console_toggled.connect(_on_console_toggle)
    controller_changed.connect(_update_state)

func _on_console_toggle(console_visible):
    _handle_actions = not console_visible

func _process_tool(delta):
    if _handle_actions and action_increase:
        if Input.is_action_pressed(action_increase, true):
            var new_value = clampf(value + step * delta, value_min, value_max)
            if not new_value == value and _controller and command:
                _controller.send_command(command, new_value)
            value = new_value

    if _handle_actions and action_decrease:
        if Input.is_action_pressed(action_decrease, true):
            var new_value = clampf(value - step * delta, value_min, value_max)
            if not new_value == value and _controller and command:
                _controller.send_command(command, value)
            value = new_value

    _t += delta
    if _t > 0.05:
        _t = 0.0
        _update_state()

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


func _process_dirty(delta):
    if not _mesh and mesh_path:
        _mesh = get_node_or_null(mesh_path)
        if _mesh:
            _mesh_original_rotation = _mesh.rotation_degrees
            _mesh_original_basis = _mesh.transform.basis
            _mesh_original_position = _mesh.position
