extends Node
class_name TrainSoundTrigger

enum TriggerMode { TOGGLE, CONTINUOUS }

@export var state_property = ""
@export var trigger_mode:TriggerMode = TriggerMode.TOGGLE
@export var trigger_threshold_min := 0.0
@export var trigger_threshold_max := 1.0
@export var sound_event:StringName = ""
@export var sound_parameter:StringName = ""

@export_node_path("Train3D") var controller_path = NodePath(""):
    set(x):
        controller_path = x
        _dirty = true
        _train = null

var _t := 0.0
var _dirty := false
var _train:Train3D = null
var _should_play := false
var _activated := false
var _timer:Timer
var _sfxplayer

func _ready() -> void:    
    _sfxplayer = get_parent()
    _timer = Timer.new()
    add_child(_timer)
    _timer.wait_time = 0.05
    _timer.one_shot = false
    _timer.timeout.connect(_check_sound_event)
    _timer.start()
    
func _exit_tree() -> void:
    remove_child(_timer)
    
func _check_sound_event():
    if _dirty:
        _dirty = false
        
        if controller_path and not _train:
            _train = get_node(controller_path) as Train3D
                
    if not _sfxplayer:
        return
        
    if state_property and _train:
        match trigger_mode:
            TriggerMode.TOGGLE:
                var value:float = float(_train.state.get(state_property, 0.0))
                _should_play = bool(value) and (value <= trigger_threshold_max and value >= trigger_threshold_min)
                if _should_play and not _activated:
                    _sfxplayer.play(sound_event)
                    _activated = true
                elif not _should_play and _activated:
                    _sfxplayer.stop(sound_event)
                    _activated = false
            TriggerMode.CONTINUOUS:
                if sound_event and sound_parameter:
                    var value = _train.state.get(state_property, 0.0) as float
                    _should_play = value <= trigger_threshold_max and value >= trigger_threshold_min 
                    if _should_play and not _activated:
                        _sfxplayer.play(sound_event, 0.0, {sound_parameter: value})
                        _activated = true
                    elif not _should_play and _activated:
                        _sfxplayer.stop(sound_event, 0.0)
                        _activated = false
                    elif _sfxplayer.is_playing(sound_event):
                        _sfxplayer.modulate(sound_event, {sound_parameter: value})
