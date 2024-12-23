extends OmniLight3D
class_name CabinOmniLight3D

var _controller:TrainController

var _dirty:bool = false
var _t = 0.0
var _target_light_energy = 0.0

@export var enabled:bool = false
@export_node_path("TrainController") var controller_path:NodePath = "":
    set(x):
        controller_path = x
        _controller = null
        _dirty = true

@export var state_property = ""
@export var light_energy_on = 1.0
@export var light_energy_off = 0.0
@export var animation_speed = 20.0


func _ready():
    _target_light_energy = light_energy_on if enabled else light_energy_off
    light_energy = _target_light_energy

func _process(delta):
    if _dirty:
        _dirty = false
        if not _controller and controller_path:
            _controller = get_node(controller_path)

    _t += delta
    if _t > 0.1:
        _t = 0.0

        if _controller and state_property:
            enabled = true if _controller.state.get(state_property, false) else false
            _target_light_energy = light_energy_on if enabled else light_energy_off

    light_energy = lerpf(light_energy, _target_light_energy, delta * animation_speed)
