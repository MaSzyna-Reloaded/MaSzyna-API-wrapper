@tool

extends Control
class_name DebugSwitch

var _dirty = false
var _node: Node

@export var label: String:
    set(x):
        _dirty = true
        label = x

enum SwitchType { MONOSTABLE, BISTABLE }
@export var type: SwitchType = SwitchType.BISTABLE

@export_node_path var node: NodePath:
    set(x):
        _dirty = true
        node = x

@export var property: String:
    set(x):
        _dirty = true
        property = x


func _ready():
    _dirty = true


func _process(delta):
    if _dirty:
        _dirty = false
        if type == SwitchType.MONOSTABLE:
            $Switch.action_mode = Button.ACTION_MODE_BUTTON_PRESS
            $Switch.toggle_mode = false
        else:
            $Switch.action_mode = Button.ACTION_MODE_BUTTON_RELEASE
            $Switch.toggle_mode = true

        $Label.text = label
        if not node.is_empty():
            _node = get_node(node)
            if _node and property and not _node.get(property) == null:
                $Switch.disabled = false
                $Switch.button_pressed = true if _node.get(property) else false
            else:
                $Switch.disabled = true
        else:
            $Switch.disabled = true


func _on_switch_toggled(toggled_on):
    if $Switch.action_mode == Button.ACTION_MODE_BUTTON_RELEASE and _node and property:
        _node.set(property, toggled_on)


func _on_switch_pressed():
    if $Switch.action_mode == Button.ACTION_MODE_BUTTON_PRESS and _node and property:
        _node.set(property, $Switch.button_pressed)
