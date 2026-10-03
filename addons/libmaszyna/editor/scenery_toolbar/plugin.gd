@tool
extends EditorPlugin

const SceneryLoadInspector = preload("./scenery_load_inspector.gd")

var scenery_toolbar = preload("./toolbar_scenery_instance.tscn")
var scenery_toolbar_instance
var _load_inspector:SceneryLoadInspector = null

func _enter_tree() -> void:
    scenery_toolbar_instance = scenery_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, scenery_toolbar_instance)
    _load_inspector = SceneryLoadInspector.new()
    add_inspector_plugin(_load_inspector)

func _exit_tree() -> void:
    remove_inspector_plugin(_load_inspector)
    _load_inspector = null
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, scenery_toolbar_instance)
    scenery_toolbar_instance.free()
    scenery_toolbar_instance = null
