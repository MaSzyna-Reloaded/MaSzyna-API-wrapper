@tool
extends Node3D
class_name MaszynaIncludeNode

signal loaded

var _dirty := false
var _t := 0

const RELOAD_DEBOUNCE_MSEC := 500

@export var filename:String = "":
    set(x):
        if not filename == x:
            _dirty = true
            filename = x
@export var parameters:Dictionary = {}
@export var context_rotate: Vector3 = Vector3.ZERO
@export var context_origin: Vector3 = Vector3.ZERO
@export var autoload:bool = true:
    set(x):
        if not x == autoload:
            autoload = x
            _dirty = true

var _scenery:MaszynaSceneryNode 

func _ready() -> void:
    var node = self
    
    if autoload:
        self.load()
        
    set_process(true)

func load() -> void:
    if self.filename:
        SceneryInstancer.instantiate(self, parameters)
        loaded.emit()
             
    
func _process(delta: float) -> void:
    if Engine.is_editor_hint() and _dirty:
        if Time.get_ticks_msec() - _t > RELOAD_DEBOUNCE_MSEC:
            _t = Time.get_ticks_msec()
            _dirty = false
            _process_dirty(delta)
        
func _process_dirty(_delta: float) -> void:
    if autoload:
        if self.filename:
            self.load()
        else:
            for child in self.get_children(true):
                remove_child(child)
