extends SpotLight3D
class_name CabinSpotLight3D

var _controller:TrainController

var _dirty:bool = false
var _setup_phase: bool = true
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

## 0 (default) = steady on/off, matching prior behavior exactly. >0 = flash the light on/off at
## this interval while `enabled` stays true - mirrors CabinBlinker's own Timer-based algorithm
## (cabin_blinker.gd). Needed because the wrapper's "blinking"-family state properties
## (TrainSecuritySystem::is_blinking() etc., Mover.cpp) are STATIC "is the alert condition active"
## flags (`alert_timer > 0.0`), not a real-time oscillating value - the actual flashing pattern
## has always been a presentation-layer concern, never baked into the state itself.
@export var blink_time:float = 0.0

var _blink_on:bool = true
var _blink_timer:Timer


func set_enabled(value:bool) -> void:
    enabled = value
    _update_state()

func _enter_tree():
    _setup_phase = true

func _ready():
    if blink_time > 0.0:
        _blink_timer = Timer.new()
        add_child(_blink_timer)
        _blink_timer.wait_time = blink_time
        _blink_timer.timeout.connect(_on_blink_timeout)

func _on_blink_timeout():
    _blink_on = not _blink_on
    _update_state()

func _update_state():
    if _controller and state_property:
        enabled = true if _controller.state.get(state_property, false) else false

    var active_now:bool
    if blink_time <= 0.0:
        active_now = enabled
    elif enabled:
        if _blink_timer.is_stopped():
            _blink_on = true
            _blink_timer.start()
        active_now = _blink_on
    else:
        _blink_timer.stop()
        _blink_on = true
        active_now = false

    _target_light_energy = light_energy_on if active_now else light_energy_off

func _process(delta):
    if _dirty:
        _dirty = false
        if not _controller and controller_path:
            _controller = get_node(controller_path)
        if _controller:
            _update_state()
            _setup_phase = true

    _t += delta
    if _t > 0.1:
        _t = 0.0
        _update_state()

    if _setup_phase:
        light_energy = _target_light_energy
        _setup_phase = false
    else:
        if visible and not light_energy:
            visible = false
        elif not visible and light_energy:
            visible = true
        light_energy = lerpf(light_energy, _target_light_energy, delta * animation_speed)
