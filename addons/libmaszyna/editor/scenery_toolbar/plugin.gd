@tool
extends EditorPlugin

var scenery_toolbar = preload("./toolbar_scenery_instance.tscn")
var scenery_toolbar_instance

func _enter_tree() -> void:
    scenery_toolbar_instance = scenery_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scenery_toolbar_instance)

func _exit_tree() -> void:
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scenery_toolbar_instance)
    scenery_toolbar_instance.free()
    scenery_toolbar_instance = null
