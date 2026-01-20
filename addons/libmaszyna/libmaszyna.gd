@tool
extends EditorPlugin

# Custom nodes

var maszyna_environment_node_script = preload("res://addons/libmaszyna/environment/maszyna_environment_node.gd")
var maszyna_environment_node_icon = preload("res://addons/libmaszyna/environment/maszyna_environment_node_icon.png")

# Editor plugins

var e3d_submodel_toolbar = preload("res://addons/libmaszyna/editor/toolbar_e3d_instance.tscn")
var e3d_submodel_toolbar_instance
var user_settings_dock
var user_settings_dock_scene = preload("res://addons/libmaszyna/editor/user_settings_dock.tscn")

func _enter_tree():
    e3d_submodel_toolbar_instance = e3d_submodel_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)

    add_autoload_singleton("MaszynaEnvironment", "res://addons/libmaszyna/environment/maszyna_environment.gd")
    add_autoload_singleton("Console", "res://addons/libmaszyna/console/console.gd")
    add_autoload_singleton("AudioStreamManager", "res://addons/libmaszyna/sound/audio_stream_manager.gd")

    add_custom_type(
        "MaszynaEnvironmentNode",
        "Node",
        maszyna_environment_node_script,
        maszyna_environment_node_icon,
    )

    user_settings_dock = user_settings_dock_scene.instantiate()
    add_control_to_dock(DOCK_SLOT_RIGHT_UL, user_settings_dock)


func _exit_tree():
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

    if user_settings_dock:
        remove_control_from_docks(user_settings_dock)

    remove_custom_type("MaszynaEnvironmentNode")

    remove_autoload_singleton("AudioStreamManager")
    remove_autoload_singleton("Console")
    remove_autoload_singleton("MaszynaEnvironment")


func add_custom_project_setting(name: String, default_value, type: int, hint: int = PROPERTY_HINT_NONE, hint_string: String = "") -> void:

    if ProjectSettings.has_setting(name): return

    var setting_info: Dictionary = {
        "name": name,
        "type": type,
        "hint": hint,
        "hint_string": hint_string
    }

    ProjectSettings.set_setting(name, default_value)
    ProjectSettings.add_property_info(setting_info)
    ProjectSettings.set_initial_value(name, default_value)
