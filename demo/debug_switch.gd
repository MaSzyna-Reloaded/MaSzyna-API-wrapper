extends Control
class_name DebugSwitch

var _dirty = false
var _node:Node

@export var label:String:
    set(x):
        _dirty = true
        label = x

@export_node_path var node:NodePath:
    set(x):
        _dirty = true
        node = x

@export var property:String:
    set(x):
        _dirty = true
        property = x

func _ready():
    _dirty = true

func _process(delta):
    if _dirty:
        _dirty = false
        $Label.text = label
        _node = get_node(node)
        if _node and property and not _node.get(property) == null:
            $Switch.disabled = false
            $Switch.button_pressed = true if _node.get(property) else false
        else:
            $Switch.disabled = true

func _on_switch_toggled(toggled_on):
    if _node and property:
        _node.set(property, toggled_on)
