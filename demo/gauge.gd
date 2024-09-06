@tool
extends Node2D

@export var value = 0.0:
    set(x):
        value = x
        _target = clampf(x * max_angle, 0.0, max_angle)
@export var max_angle = 270.0;
@export var start_angle = 270.0;
@export var label = "":
    set(x):
        label = x
        $Label.text = x

var _current = 0.0;
var _target = 0.0;


func _process(_delta):
    _current = lerpf(_current, _target, 2.0 * _delta)
    $Arrow.rotation_degrees = _current + start_angle
