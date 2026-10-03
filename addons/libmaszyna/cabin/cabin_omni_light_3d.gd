extends OmniLight3D
class_name CabinOmniLight3D

## The vehicle this element sits in, as the cabin root hands it down.
func set_vehicle_rid(vehicle_rid:RID) -> void:
    if _vehicle_rid == vehicle_rid:
        return
    _vehicle_rid = vehicle_rid
    _dirty = true


## Which vehicle this cabin element sits in; every read of it goes through CabinSystem.
var _vehicle_rid:RID

var _dirty:bool = false
var _t = 0.0
var _target_light_energy = 0.0

@export var enabled:bool = false

@export var state_property = ""
## Lit by a light of the cab it sits in (CabinSystem's cab light signals) instead of state_property
@export var cab_light:CabinState.Light = CabinState.Light.NONE
## What that light of the cab is at: its level, or 1 for a lit instrument light
var _cab_light_level:float = 0.0
@export var light_energy_on = 1.0
@export var light_energy_off = 0.0
@export var animation_speed = 20.0
var _setup_phase:bool = true

func _ready():
    pass

func _enter_tree() -> void:
    match cab_light:
        CabinState.Light.CAB:
            CabinSystem.cab_light_level_changed.connect(_on_cab_light_changed)
        CabinState.Light.INSTRUMENT:
            CabinSystem.cab_instrument_light_changed.connect(_on_cab_light_changed)

func _exit_tree() -> void:
    match cab_light:
        CabinState.Light.CAB:
            CabinSystem.cab_light_level_changed.disconnect(_on_cab_light_changed)
        CabinState.Light.INSTRUMENT:
            CabinSystem.cab_instrument_light_changed.disconnect(_on_cab_light_changed)

func _on_cab_light_changed(vehicle_rid:RID, cab:int, value:Variant) -> void:
    if vehicle_rid == _vehicle_rid and cab == CabinSystem.occupied_cab(_vehicle_rid):
        _cab_light_level = float(value)
        _update_state()

func _update_state():
    var level:float = 1.0
    if not cab_light == CabinState.Light.NONE:
        level = _cab_light_level
        enabled = level > 0.0
    elif _vehicle_rid and state_property:
        # a bool state or a 0..1 level
        level = float(CabinSystem.vehicle_state(_vehicle_rid).get(state_property, false))
        enabled = level > 0.0

    _target_light_energy = lerpf(light_energy_off, light_energy_on, level) if enabled else light_energy_off

func _process(delta):
    if _dirty:
        _dirty = false
        if _vehicle_rid:
            # the light of the cab this element sits in, as the cab holds it now
            var cab:int = CabinSystem.occupied_cab(_vehicle_rid)
            match cab_light:
                CabinState.Light.CAB:
                    _cab_light_level = CabinSystem.cab_get_light_level(_vehicle_rid, cab)
                CabinState.Light.INSTRUMENT:
                    _cab_light_level = float(CabinSystem.cab_get_instrument_light_enabled(_vehicle_rid, cab))
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
