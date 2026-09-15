@tool
extends HBoxContainer

var _selected_fiz:FIZTrainController
## Set only when _selected_fiz was found by searching descendants (DynamicRailVehicle3D case,
## see _find_child_fiz below) - its own editable_in_editor must be toggled together with
## _selected_fiz's, since it wraps FIZTrainController's ancestor chain as INTERNAL nodes that
## the Scene dock won't descend into no matter what _selected_fiz's own flag is set to.
var _selected_vehicle_wrapper:DynamicRailVehicle3D

@onready var btn = $Editable

func _ready():
    EditorInterface.get_selection().selection_changed.connect(_on_selection_changed)
    btn.disabled = true

func _exit_tree():
    EditorInterface.get_selection().selection_changed.disconnect(_on_selection_changed)

func _find_parent_fiz(node: Node):
    if not node:
        return null

    if node is FIZTrainController:
        return node
    return _find_parent_fiz(node.get_parent())


## DynamicRailVehicle3D builds its FIZTrainController as an internal descendant
## (see dynamic_rail_vehicle_3d.gd) instead of an ancestor, so it can't be found
## by walking up the tree like a hand-authored RailVehicle3D scene.
func _find_child_fiz(node: Node) -> FIZTrainController:
    var found:Array = node.find_children("", "FIZTrainController", true, false)
    return found[0] as FIZTrainController if found else null


func _on_selection_changed():
    var sel:EditorSelection = EditorInterface.get_selection()
    var nodes = sel.get_selected_nodes()

    _selected_fiz = null
    _selected_vehicle_wrapper = null
    btn.button_pressed = false

    if nodes.size() == 1:
        var n:Node = nodes[0]
        _selected_fiz = _find_parent_fiz(n)
        if not _selected_fiz and n is DynamicRailVehicle3D:
            _selected_fiz = _find_child_fiz(n)
            if _selected_fiz:
                _selected_vehicle_wrapper = n

    btn.disabled = false if _selected_fiz else true
    btn.button_pressed = _selected_fiz and _selected_fiz.editable_in_editor

func _on_editable_toggled(toggled_on):
    if not _selected_fiz:
        return
    if toggled_on and _selected_vehicle_wrapper:
        _selected_vehicle_wrapper.editable_in_editor = true
    _selected_fiz.editable_in_editor = toggled_on
    if not toggled_on and _selected_vehicle_wrapper:
        _selected_vehicle_wrapper.editable_in_editor = false
