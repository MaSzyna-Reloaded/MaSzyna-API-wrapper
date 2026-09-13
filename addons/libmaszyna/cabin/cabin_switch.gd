extends BaseCabinTool3D
class_name CabinSwitch

signal switch_position_changed(previous_position, new_position)
signal switch_pushed()
signal switch_released()

enum ControllerMode { OnOff, On, Off }

@export var switch_position:int = 0:
    set(x):
        x = clampi(x, switch_min_position, switch_max_position)
        if not x == switch_position:
            _dirty = true
            var previous_position:int = switch_position
            switch_position = x
            emit_signal("switch_position_changed", previous_position, switch_position)


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

## Some state domains don't start at the switch's own visual rest position - e.g. radio_channel's
## real range is 1..10 (channel 0 isn't valid), but the physical knob's first notch is still the
## visual "zero" position, so switch_position=1 must render as ONE STEP from rest, not two.
## Subtracted from switch_position before computing mesh rotation/position only - command
## dispatch and clamping (switch_min/max_position) still use the real, unshifted switch_position.
@export var value_offset:int = 0:
    set(x):
        value_offset = x
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
var _mesh_original_basis:Basis
var _mesh_original_position:Vector3 = Vector3.ZERO
var _target_mesh_rotation:Vector3 = Vector3.ZERO
var _target_mesh_position:Vector3 = Vector3.ZERO
var _current_rotation:Vector3 = Vector3.ZERO
var _current_position:Vector3 = Vector3.ZERO

var _sound:AudioStreamPlayer3D = AudioStreamPlayer3D.new()
var _handle_actions:bool = true
var _t:float = 0.0
var _setup_phase:bool = true

func _ready():
    add_child(_sound)
    _sound.max_distance = sound_max_distance
    self.switch_position_changed.connect(self._on_switch_position_changed)

    if not Engine.is_editor_hint() and Console:
        Console.console_toggled.connect(_on_console_toggle)
    controller_changed.connect(_on_controller_changed)

func _exit_tree() -> void:
    if _controller:
        _controller.command_received.disconnect(_on_command_received)

func _on_controller_changed() -> void:
    _controller.command_received.connect(_on_command_received)
    _update_state()

func _update_state() -> void:
    if state_property and _controller:
        switch_position = int(_controller.state.get(state_property, switch_position))
    _target_mesh_position = (switch_position - value_offset) * mesh_position
    _target_mesh_rotation = (switch_position - value_offset) * mesh_rotation

func _on_command_received(p_command:String, p_p1:Variant, _p_p2:Variant) -> void:
    if command_set and p_command == command_set:
        switch_position = int(p_p1) if p_p1 else 0
    elif (p_command == command_increase or p_command == command_decrease):
        _update_state()

func _enter_tree():
    _setup_phase = true

func _on_console_toggle(console_visible):
    _handle_actions = not console_visible

func _input(event):
    if not _handle_actions:
        return

    if action_increase:
        if event.is_action_pressed(action_increase, false, true):
            _set_position_from_input(switch_position + 1)
        if automatic_reset and event.is_action_released(action_increase, true):
            _set_position_from_input(switch_reset_position)
    if action_decrease:
        if event.is_action_pressed(action_decrease, false, true):
            _set_position_from_input(switch_position - 1)
        if automatic_reset and event.is_action_released(action_decrease, true):
            _set_position_from_input(switch_reset_position)
    if action_toggle:
        if event.is_action_pressed(action_toggle, false, true):
            if switch_position == switch_max_position:
                _set_position_from_input(switch_min_position)
            else:
                _set_position_from_input(switch_max_position)
        if automatic_reset and event.is_action_released(action_toggle, true):
            _set_position_from_input(switch_reset_position)

func _process_dirty(delta):
    if not _mesh and mesh_path:
        _mesh = get_node_or_null(mesh_path)
        if _mesh:
            global_position = _mesh.global_position
            _mesh_original_basis = _mesh.transform.basis
            _mesh_original_position = _mesh.position
    if controller_path and not _controller:
        _controller = get_node_or_null(controller_path)

func _process_tool(delta):
    _t += delta
    if _t > 0.05:
        _t = 0.0
        _target_mesh_position = (switch_position - value_offset) * mesh_position
        _target_mesh_rotation = (switch_position - value_offset) * mesh_rotation

    if _setup_phase and mesh_path and not _mesh:
        _dirty = true

    if _setup_phase and _mesh:
        _setup_phase = false
        _current_position = _target_mesh_position
        _current_rotation = _target_mesh_rotation
    else:
        _current_rotation = _current_rotation.lerp(_target_mesh_rotation, delta * animation_speed)
        _current_position = _current_position.lerp(_target_mesh_position, delta * animation_speed)

    if is_instance_valid(_mesh):
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
    elif current == 0:
        _sound.stream = null
    else:
        _sound.stream = sound_increase_stream if current > previous else sound_decrease_stream

    if _sound.stream:
        _sound.play()

func _set_position_from_input(p_position:int) -> void:
    var previous_position:int = switch_position
    switch_position = p_position

    if previous_position == switch_position:
        return

    if _controller:
        var command:String = command_increase if switch_position > previous_position else command_decrease
        if command:
            _controller.send_command(command)
        if command_set:
            _controller.send_command(command_set, switch_position)
