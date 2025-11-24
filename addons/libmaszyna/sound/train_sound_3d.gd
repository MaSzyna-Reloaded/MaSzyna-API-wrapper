extends SteamAudioPlayer
class_name TrainSound3D

@export var state_property = ""
@export_node_path("TrainNode") var controller_path = NodePath(""):
    set(x):
        controller_path = x
        _dirty = true
        _train = null

var _t = 0.0
var _dirty = false
var _train = null
var _should_play = false

func _ready() -> void:
    self.distance_attenuation = true
    self.air_absorption = true
    self.occlusion = true
    self.reflection = true
    self.emission_angle_enabled = true
    self.doppler_tracking = AudioStreamPlayer3D.DOPPLER_TRACKING_PHYSICS_STEP

func _process(_delta):
    _t += _delta
    if _t > 0.1:
        _t = 0.0
        if state_property and _train:
            _should_play = true if _train.state.get(state_property, false) else false
            if _should_play and not playing:
                play()
            elif not _should_play and playing:
                stop()
    if _dirty:
        _dirty = false

        if controller_path and not _train:
            _train = get_node(controller_path)
