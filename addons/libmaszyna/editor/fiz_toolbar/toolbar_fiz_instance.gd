@tool
extends HBoxContainer

var _selected_fiz:FIZTrainController

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


func _on_selection_changed():
    var sel:EditorSelection = EditorInterface.get_selection()
    var nodes = sel.get_selected_nodes()

    _selected_fiz = null
    btn.button_pressed = false

    if nodes.size() == 1:
        var n:Node = nodes[0]
        _selected_fiz = _find_parent_fiz(n)

    btn.disabled = false if _selected_fiz else true
    btn.button_pressed = _selected_fiz and _selected_fiz.editable_in_editor

func _on_editable_toggled(toggled_on):
    if _selected_fiz:
        _selected_fiz.editable_in_editor = toggled_on
