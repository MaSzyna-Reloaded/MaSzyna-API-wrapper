extends Node
class_name TrainSoundTrigger

enum TriggerMode { TOGGLE, CONTINUOUS }

@export var state_property:String = ""
@export var trigger_mode:TriggerMode = TriggerMode.TOGGLE
@export var trigger_threshold_min:float = 0.0
@export var trigger_threshold_max:float = 1.0
@export var sound_event:StringName = &""
@export var sound_parameter:StringName = &""

@export_node_path("TrainController") var controller_path:NodePath = NodePath(""):
    set(value):
        controller_path = value
        _dirty = true
        _train = null

var _dirty:bool = false
var _train:TrainController
var _activated:bool = false
var _timer:Timer
var _sfxplayer:SfxPlayer3D


func _ready() -> void:
    _sfxplayer = get_parent() as SfxPlayer3D
    _timer = Timer.new()
    add_child(_timer)
    _timer.wait_time = 0.05
    _timer.timeout.connect(_check_sound_event)
    _timer.start()


func _exit_tree() -> void:
    remove_child(_timer)


func _check_sound_event() -> void:
    if _dirty:
        _dirty = false
        if controller_path and not _train:
            _train = get_node(controller_path) as TrainController
    if not _sfxplayer or not state_property or not _train:
        return

    var raw_value:Variant = _train.state.get(state_property, 0.0)
    var value:float = float(raw_value) if raw_value else 0.0
    var should_play:bool = value <= trigger_threshold_max and value >= trigger_threshold_min
    if trigger_mode == TriggerMode.TOGGLE:
        should_play = bool(value) and should_play
    if should_play and not _activated:
        if trigger_mode == TriggerMode.CONTINUOUS and sound_parameter:
            _sfxplayer.play(sound_event, {sound_parameter: value})
        else:
            _sfxplayer.play(sound_event)
        _activated = true
    elif not should_play and _activated:
        _sfxplayer.stop(sound_event)
        _activated = false
    elif should_play and trigger_mode == TriggerMode.CONTINUOUS and sound_parameter:
        _sfxplayer.modulate(sound_event, {sound_parameter: value})
