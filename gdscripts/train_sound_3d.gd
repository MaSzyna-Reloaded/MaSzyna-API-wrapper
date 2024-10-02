extends AudioStreamPlayer3D
class_name TrainSound3D

@export var state_property = ""
@export_node_path("TrainController") var train_controller_path = NodePath(""):
    set(x):
        train_controller_path = x
        _dirty = true
        _train = null

var _t = 0.0
var _dirty = false
var _train = null
var _should_play = false


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

        if train_controller_path and not _train:
            _train = get_node(train_controller_path)
