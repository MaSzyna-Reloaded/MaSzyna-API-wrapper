@tool
extends EditorPlugin

var fiz_instance_toolbar = preload("./toolbar_fiz_instance.tscn")
var fiz_instance_toolbar_instance

func _enter_tree() -> void:
    fiz_instance_toolbar_instance = fiz_instance_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, fiz_instance_toolbar_instance)

func _exit_tree() -> void:
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, fiz_instance_toolbar_instance)
    fiz_instance_toolbar_instance.free()
    fiz_instance_toolbar_instance = null
