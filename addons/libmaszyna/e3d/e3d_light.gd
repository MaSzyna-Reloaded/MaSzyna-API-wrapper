@tool
extends SpotLight3D

var meshes_on: Array[Node3D] = []
var meshes_off: Array[Node3D] = []

@export var enabled: bool = true:
    set(v):
        enabled = v
        _update_state()

func _ready():
    _update_state()

func _update_state():
    var parent = get_parent()
    if parent == null:
        return
    var light_root = parent.get_parent()
    var is_end = parent.name.begins_with("end")
    if (!is_end):
        var base_name = parent.name.trim_suffix("_on")
        var lamp_off_node = light_root.get_node_or_null(NodePath(base_name + "_off"))
        parent.visible = enabled
        if lamp_off_node != null:
            lamp_off_node.visible = !enabled
    else:
        parent.visible = enabled
