#@tool
extends Node3D
class_name TrainSwitch

signal switch_position_changed(previous_position, new_position)
signal switch_pushed()
signal switch_released()

enum ControllerMode { OnOff, On, Off }

@export var switch_position:int = 0:
    set(x):
        x = clampi(x, switch_min_position, switch_max_position)
        if not x == switch_position:
            _dirty = true
            var _prev = switch_position
            switch_position = _handle_position_change(_prev, x)
            if not _prev == switch_position:
                emit_signal("switch_position_changed", _prev, switch_position)


@export var switch_min_position:int = 0:
    set(x):
        switch_min_position = x
        _dirty = true

@export var switch_max_position:int = 1:
    set(x):
        switch_max_position = x
        _dirty = true

@export var switch_reset_position:int = 0:
    set(x):
        switch_reset_position = x
        _dirty = true

@export var automatic_reset:bool = false
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

@export var command_increase = ""
@export var command_decrease = ""
@export var command_set = ""

@export var state_property = ""

@export var mesh_position:Vector3 = Vector3.ZERO:
    set(x):
        mesh_position = x if x else Vector3.ZERO
        _dirty = true

@export var mesh_rotation:Vector3 = Vector3.ZERO:
    set(x):
        mesh_rotation = x if x else Vector3.ZERO
        _dirty = true

@export var animation_speed = 10.0
@export var sound_increase_stream:AudioStream
@export var sound_decrease_stream:AudioStream
@export var sound_neutral_position_stream:AudioStream
@export var sound_override:Array[AudioStream]
@export var sound_override_negative:Array[AudioStream]
@export var sound_max_distance:float = 3.0:
    set(x):
        sound_max_distance = x
        _sound.max_distance = x

@export var action_increase = ""
@export var action_decrease = ""
@export var action_toggle = ""

var _mesh:MeshInstance3D
var _controller:TrainController
var _mesh_original_basis:Basis
var _mesh_original_position:Vector3 = Vector3.ZERO
var _target_mesh_rotation:Vector3 = Vector3.ZERO
var _target_mesh_position:Vector3 = Vector3.ZERO
var _current_rotation:Vector3 = Vector3.ZERO
var _current_position:Vector3 = Vector3.ZERO
var _dirty = false
var _sound:AudioStreamPlayer3D = AudioStreamPlayer3D.new()

func _ready():
    add_child(_sound)
    _sound.max_distance = sound_max_distance
    self.switch_position_changed.connect(self._on_switch_position_changed)

func _input(event):
    if action_increase:
        if event.is_action_pressed(action_increase, false, true):
            switch_position += 1
        if automatic_reset and event.is_action_released(action_increase, true):
            switch_position = switch_reset_position
    if action_decrease:
        if event.is_action_pressed(action_decrease, false, true):
            switch_position -= 1
        if automatic_reset and event.is_action_released(action_decrease, true):
            switch_position = switch_reset_position
    if action_toggle:
        if event.is_action_pressed(action_toggle, false, true):
            if switch_position == switch_max_position:
                switch_position = switch_min_position
            else:
                switch_position = switch_max_position
        if automatic_reset and event.is_action_released(action_toggle, true):
            switch_position = switch_reset_position


func _process(delta):
    if _dirty:
        _target_mesh_position = switch_position * mesh_position
        _target_mesh_rotation = switch_position * mesh_rotation

        _dirty = false

        if not _mesh and mesh_path:
            _mesh = get_node(mesh_path)
            if _mesh:
                global_position = _mesh.global_position
                _mesh_original_basis = _mesh.transform.basis
                _mesh_original_position = _mesh.position
        if controller_path and not _controller:
            _controller = get_node(controller_path)


    _current_rotation = _current_rotation.lerp(_target_mesh_rotation, delta * animation_speed)
    _current_position = _current_position.lerp(_target_mesh_position, delta * animation_speed)

    if _mesh:
        var new_basis = _mesh_original_basis
        new_basis *= Basis(Vector3.RIGHT, deg_to_rad(_current_rotation.x))
        new_basis *= Basis(Vector3.UP, deg_to_rad(_current_rotation.y))
        new_basis *= Basis(Vector3.FORWARD, deg_to_rad(_current_rotation.z))
        _mesh.transform.basis = new_basis
        _mesh.position = _mesh_original_position + _current_position



func _play_sound():

    if _sound.stream:
        _sound.play()


func _on_switch_position_changed(previous, current):
    if current == 0 and sound_neutral_position_stream:
        _sound.stream = sound_neutral_position_stream
    elif current > 0 and current <= sound_override.size():
        _sound.stream = sound_override[current-1]
    elif current < 0 and -current <= sound_override_negative.size():
        _sound.stream = sound_override_negative[-current-1]
    else:
        _sound.stream = sound_increase_stream if current > previous else sound_decrease_stream

    if _sound.stream:
        _sound.play()


func _handle_position_change(prev, current) -> int:
    if _controller:
        var cmd = command_increase if current > prev else command_decrease
        if cmd:
            _controller.receive_command(cmd, true)
        if command_set:
            _controller.receive_command(command_set, current)

    if state_property:
        return _controller.state.get(state_property, current)
    else:
        return current
