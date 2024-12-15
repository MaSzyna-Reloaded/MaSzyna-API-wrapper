extends Node


@export var enabled:bool = false:
    set(x):
        enabled = x
        _dirty = true
@export var powered:bool = false
@export var acceleration:float = 4.0
@export var min_speed:float = 0.0
@export var max_speed:float = 30.0

@export var target_path:NodePath = "":
    set(x):
        target_path = x
        _target = null
        _dirty = true

var _target:Node3D

var _dirty = false
var _speed = 0.0

func _process(delta):
    if enabled and powered:
        _speed = clampf(_speed + delta * acceleration, min_speed, max_speed)
    else:
        _speed = lerpf(_speed, 0.0, delta * 0.6)

    if _target:
        _target.rotate_object_local(Vector3.UP, PI * delta * _speed)

    if _dirty:
        if not _target:
            _target = get_node(target_path)
