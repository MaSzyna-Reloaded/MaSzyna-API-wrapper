@tool
extends EditorPlugin

var e3d_submodel_toolbar = preload("./toolbar_e3d_instance.tscn")
var e3d_submodel_toolbar_instance

func _enter_tree() -> void:
    e3d_submodel_toolbar_instance = e3d_submodel_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

func _exit_tree() -> void:
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)
    e3d_submodel_toolbar_instance.free()
    e3d_submodel_toolbar_instance = null
