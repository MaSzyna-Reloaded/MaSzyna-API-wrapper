@tool
extends Node3D
class_name MaszynaIncludeNode

var _dirty := false
var _t := 0

const RELOAD_DEBOUNCE_MSEC := 500

@export var parameters:Dictionary = {}
@export var filename:String = "":
    set(x):
        if not filename == x:
            _dirty = true
            filename = x

func _ready() -> void:
    if self.filename:
        SceneryInstancer.instantiate(self, parameters)
    set_process(true)


func _process(delta: float) -> void:
    if Engine.is_editor_hint() and _dirty:
        if Time.get_ticks_msec() - _t > RELOAD_DEBOUNCE_MSEC:
            _t = Time.get_ticks_msec()
            _dirty = false
            _process_dirty(delta)
        
func _process_dirty(_delta: float) -> void:
    if self.filename:
        SceneryInstancer.instantiate(self, parameters)
    else:
        for child in self.get_children(true):
            remove_child(child)
