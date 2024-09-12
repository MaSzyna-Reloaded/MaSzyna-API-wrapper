extends Node3D
class_name TrainGauge

@export var value = 0.0:
    set(x):
        if not value == x:
            value = x
            _dirty = true

@export var max_value = 1.0:
    set(x):
        max_value = x
        _dirty = true

@export var max_angle = 270.0:
    set(x):
        max_angle = x
        _dirty = true
@export var mesh_rotation = Vector3.ZERO:
    set(x):
        mesh_rotation = x
        _dirty = true

@export var animation_speed = 4.0
#@export var start_angle = 270.0

@export_node_path("TrainController") var train_controller_path:NodePath:
    set(x):
        _train = null
        train_controller_path = x
        _dirty = true

@export var state_property:String = ""
@export var max_state_property:String = ""

@export var target_mesh_path:NodePath:
    set(x):
        target_mesh_path = x
        _mesh = null

#var _current = 0.0;
#var _target = 0.0;
var _current_rotation:Vector3 = Vector3.ZERO
var _target_mesh_rotation:Vector3 = Vector3.ZERO
var _mesh:MeshInstance3D = null
var _mesh_original_basis:Basis
#var _base_rot:Vector3 = Vector3.ZERO
var _dirty:bool = false
var _train:TrainController
var _t = 0.0


func _process(_delta):
    _t += _delta
    if _t > 0.1:
        _t = 0.0
        _on_state_update_timer_timeout()

    if _dirty:
        _dirty = false
        if not max_value == 0.0:
            _target_mesh_rotation = (value/max_value) * mesh_rotation
        if train_controller_path:
            _train = get_node(train_controller_path)

    if not _mesh and not target_mesh_path.is_empty():
        _mesh = get_node(target_mesh_path)
        if _mesh:
            _mesh_original_basis = _mesh.basis
            global_position = _mesh.global_position

    _current_rotation = _current_rotation.lerp(_target_mesh_rotation, _delta * animation_speed)

    if _mesh:
        var new_basis = _mesh_original_basis
        new_basis *= Basis(Vector3.RIGHT, deg_to_rad(_current_rotation.x))
        new_basis *= Basis(Vector3.UP, deg_to_rad(_current_rotation.y))
        new_basis *= Basis(Vector3.FORWARD, deg_to_rad(_current_rotation.z))
        _mesh.transform.basis = new_basis


func _on_state_update_timer_timeout():
    if state_property:
        var train_controller = get_node(train_controller_path) as TrainController
        if train_controller:
            if max_state_property:
                max_value = train_controller.state.get(max_state_property, 0.0)
            value = train_controller.state.get(state_property, 0.0)
