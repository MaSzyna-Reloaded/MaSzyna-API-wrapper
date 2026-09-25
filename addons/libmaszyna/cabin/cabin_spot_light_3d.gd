extends SpotLight3D
class_name CabinSpotLight3D

## The vehicle this element sits in, as the cabin root hands it down.
func set_train_id(train_id:String) -> void:
    if _train_id == train_id:
        return
    _train_id = train_id
    _dirty = true


## Which vehicle this cabin element sits in; every read of it goes through CabinSystem.
var _train_id:String = ""

var _dirty:bool = false
var _setup_phase: bool = true
var _t = 0.0
var _target_light_energy = 0.0

@export var enabled:bool = false

@export var state_property = ""
## Most MMD indicator labels have no real per-vehicle lamp definition to derive
## light_color/spot_range/etc. from anywhere in the MMD - SM42's own hand-authored reference only
## has real numeric light data for its czuwak (alerter) lamps, so reusing those values for a
## different indicator produces a visibly wrong light (wrong color/range/angle). Defaults to false
## for this reason - only indicator widgets with real reference light data (currently
## i-security_aware) should opt in with light_enabled=true. When false, this widget still
## shows/hides its on_target/off_target submodel pair and still plays its click sound exactly as
## before - only the light itself (this node's own SpotLight3D energy) is suppressed.
@export var light_enabled:bool = false
@export var light_energy_on = 1.0
@export var light_energy_off = 0.0
@export var animation_speed = 20.0
## Played on every on/off FLASH transition (not just the overall enabled=false->true/true->false
## transition) - matches SM42's own hand-authored reference exactly (cabin_blinker.gd's `blink`
## signal, connected in sm_42_cabin.gd's _on_czuwak_blink(), plays a click on every single blink
## cycle while the alerter stays active, giving a realistic relay-clicking sound, not one click per
## alert session). The events live in the cab's bank and are named like CabinButton's own
## sound_on_event/sound_off_event, so MmdCabinInstancer._apply_sound() wires soundinc:/sounddec:
## here the same way.
@export var sound_player:SfxPlayer3D
@export var sound_on_event:StringName
@export var sound_off_event:StringName

var _sound_enabled_last:bool = false

## The original engine shows/hides a "<name>_on"/"<name>_off" submodel pair for these indicators
## (TButton::Update(), Button.cpp) instead of/alongside a real light - wired here (rather than as
## a separate widget) so both stay in sync with the exact same on/off timing this class already
## computes for its own light_energy. Either or both may be left unset (some vehicles have only
## one of the pair, or neither).
@export_node_path("Node3D") var on_target_path:NodePath = "":
    set(x):
        on_target_path = x
        _on_target = null
        _dirty = true
@export_node_path("Node3D") var off_target_path:NodePath = "":
    set(x):
        off_target_path = x
        _off_target = null
        _dirty = true

var _on_target:Node3D
var _off_target:Node3D

## 0 (default) = steady on/off, matching prior behavior exactly. >0 = flash the light on/off at
## this interval while `enabled` stays true - mirrors CabinBlinker's own Timer-based algorithm
## (cabin_blinker.gd). Needed because the wrapper's "blinking"-family state properties
## (VehicleSecuritySystem::is_blinking() etc., Mover.cpp) are STATIC "is the alert condition active"
## flags (`alert_timer > 0.0`), not a real-time oscillating value - the actual flashing pattern
## has always been a presentation-layer concern, never baked into the state itself.
@export var blink_time:float = 0.0

var _blink_on:bool = true
var _blink_timer:Timer

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
    if _train_id and state_property:
        enabled = true if CabinSystem.vehicle_state(_train_id).get(state_property, false) else false

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

    # Compared against active_now (this flash's on/off state), not `enabled` (the overall alert
    # session) - see this class's own header comment on sound_on_event/sound_off_event for why.
    if not active_now == _sound_enabled_last:
        _sound_enabled_last = active_now
        var event:StringName = sound_on_event if active_now else sound_off_event
        if sound_player and event:
            sound_player.play(event)

    _target_light_energy = (light_energy_on if active_now else light_energy_off) if light_enabled else 0.0
    if _on_target:
        _on_target.visible = active_now
    if _off_target:
        _off_target.visible = not active_now

func _process(delta):
    if _dirty:
        _dirty = false
        if not _on_target and on_target_path:
            _on_target = get_node_or_null(on_target_path)
        if not _off_target and off_target_path:
            _off_target = get_node_or_null(off_target_path)
        if _train_id:
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
