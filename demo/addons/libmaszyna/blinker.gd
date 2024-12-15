extends Node3D

signal blink(state:bool)

var _visible:bool = false

@onready var timer = $Timer

@export var enabled:bool = false:
    set(x):
        if not x == enabled:
            enabled = x
            _dirty = true
            if not enabled:
                emit_signal("blink", false)

@export var blink_mode:bool = true:
    set(x):
        blink_mode = x
        _dirty = true

@export var blink_time:float = 0.2:
    set(x):
        blink_time = x
        _dirty = true


@export var target_path:NodePath = "":
    set(x):
        target_path = x
        _target = null
        _dirty = true

var _target:Node3D
var _dirty = false

func _ready():
    $OmniLight3D.light_energy = 0.0
    $OmniLight3D2.light_energy = 0.0
    $OmniLight3D3.light_energy = 0.0

func _process(delta):
    if _dirty:
        _dirty = false

        timer.wait_time = blink_time
        if not _target:
            _target = get_node(target_path)
            if _target:
                _visible = _target.visible

        if blink_mode:
            if timer.is_stopped():
                if enabled:
                    timer.start()
                else:
                    if _target:
                        _target.visible = false
            elif not timer.is_stopped() and not enabled:
                timer.stop()
                if _target:
                    _target.visible = false
            else:
                _target.visible = enabled

func _on_timer_timeout():
    _visible = not _visible
    if _target:
        _target.visible = _visible
    emit_signal("blink", _visible)
