@tool
extends EditorPlugin

var e3d_loader = E3DResourceFormatLoader.new()

var user_settings_dock_scene = preload("res://addons/libmaszyna/editor/user_settings_dock.tscn")
var maszyna_environment_node_script = preload("res://addons/libmaszyna/environment/maszyna_environment_node.gd")
var maszyna_environment_node_icon = preload("res://addons/libmaszyna/environment/maszyna_environment_node_icon.png")

var user_settings_dock


func _enter_tree():
    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)

    add_autoload_singleton("MaszynaEnvironment", "res://addons/libmaszyna/environment/maszyna_environment.gd")
    add_autoload_singleton("Console", "res://addons/libmaszyna/console.gd")
    add_autoload_singleton("MaterialManager", "res://addons/libmaszyna/materials/material_manager.gd")
    add_autoload_singleton("MaterialParser", "res://addons/libmaszyna/materials/material_parser.gd")
    add_autoload_singleton("E3DParser", "res://addons/libmaszyna/e3d/e3d_parser.gd")
    add_autoload_singleton("E3DModelManager", "res://addons/libmaszyna/e3d/e3d_model_manager.gd")
    add_autoload_singleton("E3DNodesInstancer", "res://addons/libmaszyna/e3d/e3d_nodes_instancer.gd")
    add_autoload_singleton("UserSettings", "res://addons/libmaszyna/user_settings.gd")

    add_custom_type(
        "MaszynaEnvironmentNode",
        "Node",
        maszyna_environment_node_script,
        maszyna_environment_node_icon,
    )

    ResourceLoader.add_resource_format_loader(e3d_loader)
    user_settings_dock = user_settings_dock_scene.instantiate()
    add_control_to_dock(DOCK_SLOT_RIGHT_UL, user_settings_dock)


func _exit_tree():
    if user_settings_dock:
        remove_control_from_docks(user_settings_dock)

    ResourceLoader.remove_resource_format_loader(e3d_loader)

    remove_custom_type("MaszynaEnvironmentNode")

    remove_autoload_singleton("UserSettings")
    remove_autoload_singleton("E3DNodesInstancer")
    remove_autoload_singleton("E3DModelManager")
    remove_autoload_singleton("E3DParser")
    remove_autoload_singleton("MaterialParser")
    remove_autoload_singleton("MaterialManager")
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
