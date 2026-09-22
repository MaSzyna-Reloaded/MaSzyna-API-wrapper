extends OmniLight3D
class_name CabinOmniLight3D

## The vehicle this element sits in, as the cabin root hands it down.
func set_vehicle(train_id:String) -> void:
    if _train_id == train_id:
        return
    _train_id = train_id
    _dirty = true


## Which vehicle this cabin element sits in; every read of it goes through CabinSystem.
var _train_id:String = ""

var _dirty:bool = false
var _t = 0.0
var _target_light_energy = 0.0

@export var enabled:bool = false

@export var state_property = ""
@export var light_energy_on = 1.0
@export var light_energy_off = 0.0
@export var animation_speed = 20.0
var _setup_phase:bool = true

func _ready():
    pass

func _update_state():
    var level:float = 1.0
    if _train_id and state_property:
        # a bool state or a 0..1 light level (e.g. roof_light_level)
        level = float(CabinSystem.vehicle_state(_train_id).get(state_property, false))
        enabled = level > 0.0

    _target_light_energy = lerpf(light_energy_off, light_energy_on, level) if enabled else light_energy_off

func _process(delta):
    if _dirty:
        _dirty = false
        if _train_id:
            _setup_phase = true
            _update_state()

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
