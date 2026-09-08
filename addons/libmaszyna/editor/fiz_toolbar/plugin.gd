@tool
extends EditorPlugin

var _button:CheckButton
var _selected_fiz:FIZTrainController

func _enter_tree() -> void:
    _button = CheckButton.new()
    _button.text = "Edit FIZ"
    _button.disabled = true
    _button.toggled.connect(_on_editable_toggled)
    EditorInterface.get_selection().selection_changed.connect(_on_selection_changed)
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, _button)

func _exit_tree() -> void:
    EditorInterface.get_selection().selection_changed.disconnect(_on_selection_changed)
    _button.toggled.disconnect(_on_editable_toggled)
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, _button)
    _button.free()
    _button = null


func _find_parent_fiz(node:Node) -> FIZTrainController:
    if not node:
        return null
    if node is FIZTrainController:
        return node
    return _find_parent_fiz(node.get_parent())


func _on_selection_changed() -> void:
    var nodes:Array[Node] = EditorInterface.get_selection().get_selected_nodes()
    _selected_fiz = _find_parent_fiz(nodes[0]) if nodes.size() == 1 else null
    _button.disabled = not _selected_fiz
    _button.button_pressed = true if _selected_fiz and _selected_fiz.editable_in_editor else false


func _on_editable_toggled(toggled_on:bool) -> void:
    if _selected_fiz:
        _selected_fiz.editable_in_editor = toggled_on
