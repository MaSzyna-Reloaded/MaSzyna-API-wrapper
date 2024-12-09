#@tool
extends Node3D
class_name TrainKnob

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
@export_node_path("TrainController") var controller_path:NodePath = "":
    set(x):
        controller_path = x
        _controller = null
        _dirty = true
@export var command = ""
@export var state_property = ""
@export var state_min_property = ""
@export var state_max_property = ""

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
var _controller:TrainController
var _mesh_original_rotation:Vector3 = Vector3.ZERO
var _mesh_original_basis:Basis
var _mesh_original_position:Vector3 = Vector3.ZERO
var _target_mesh_rotation:Vector3 = Vector3.ZERO
var _target_mesh_position:Vector3 = Vector3.ZERO
var _current_rotation:Vector3 = Vector3.ZERO
var _current_position:Vector3 = Vector3.ZERO
var _dirty = false
var _sound:AudioStreamPlayer3D = AudioStreamPlayer3D.new()

var _t = 0.0
var _value_normalized = 0.0

func _process(delta):

    if action_increase and not Console.is_visible():
        if Input.is_action_pressed(action_increase, true):
            var new_value = clampf(value + step * delta, value_min, value_max)
            if not new_value == value and _controller and command:
                _controller.send_command(command, new_value)
            value = new_value

    if action_decrease and not Console.is_visible():
        if Input.is_action_pressed(action_decrease, true):
            var new_value = clampf(value - step * delta, value_min, value_max)
            if not new_value == value and _controller and command:
                _controller.send_command(command, value)
            value = new_value

    _t += delta
    if _t > 0.05:
        _t = 0.0
        if _controller:
            if state_min_property:
                value_min = _controller.state.get(state_min_property, value_min)
            if state_max_property:
                value_max = _controller.state.get(state_max_property, value_max)
            if state_property:
                value = _controller.state.get(state_property, value)

        _value_normalized = value / (value_max - value_min)
        _target_mesh_position = mesh_position * _value_normalized
        _target_mesh_rotation = mesh_rotation * _value_normalized


    _current_rotation = _current_rotation.lerp(_target_mesh_rotation, delta * speed)
    _current_position = _current_position.lerp(_target_mesh_position, delta * speed)

    if _dirty:
        _dirty = false

        if not _mesh and mesh_path:
            _mesh = get_node(mesh_path)
            if _mesh:
                #global_position = _mesh.global_position
                _mesh_original_rotation = _mesh.rotation_degrees
                _mesh_original_basis = _mesh.transform.basis
                _mesh_original_position = _mesh.position

        if controller_path and not _controller:
            _controller = get_node(controller_path)

    if _mesh:
        #_mesh.rotation_degrees = _mesh_original_rotation + _current_rotation

        var new_basis = _mesh_original_basis
        new_basis *= Basis(Vector3.RIGHT, deg_to_rad(_current_rotation.x))
        new_basis *= Basis(Vector3.UP, deg_to_rad(_current_rotation.y))
        new_basis *= Basis(Vector3.FORWARD, deg_to_rad(_current_rotation.z))
        _mesh.transform.basis = new_basis

        _mesh.position = _mesh_original_position + _current_position
