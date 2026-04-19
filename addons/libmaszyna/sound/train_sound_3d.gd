extends SfxPlayer3D
class_name TrainSound3D

enum TriggerMode { TOGGLE, CONTINUOUS }

var _trigger = TrainSoundTrigger.new()
var _dirty := false

@export var state_property = "":
    set(x):
        state_property = x
        _trigger.state_property = x
@export var trigger_mode:TriggerMode = TriggerMode.TOGGLE:
    set(x):
        trigger_mode = x
        _trigger.trigger_mode = x
@export var trigger_threshold_min:float = 0.0:
    set(x):
        trigger_threshold_min = x
        _trigger.trigger_threshold_min = x
@export var trigger_threshold_max:float = 1.0:
    set(x):
        trigger_threshold_max = x
        _trigger.trigger_threshold_max = x
        
@export var sound_event:StringName = "":
    set(x):
        sound_event = x
        _trigger.sound_event = x
@export var sound_parameter:StringName = "":
    set(x):
        sound_parameter = x
        _trigger.sound_parameter = x

@export_node_path("RailVehicle3D") var controller_path = NodePath(""):
    set(x):
        _dirty = true
        controller_path = x
        if x and is_inside_tree():
            var node = get_node(x)
            _trigger.controller_path = _trigger.get_path_to(node)
        else:
            _trigger.controller_path = NodePath("")
            

func _ready() -> void:
    var node = get_node_or_null(controller_path)
    if node:
        _trigger.controller_path = _trigger.get_path_to(node)
    
func _enter_tree() -> void:
    add_child(_trigger)    

func _exit_tree() -> void:
    super()
    remove_child(_trigger)
    
