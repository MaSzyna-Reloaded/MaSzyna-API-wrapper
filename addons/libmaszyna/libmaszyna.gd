@tool
extends EditorPlugin

var e3d_loader = E3DResourceFormatLoader.new()

# Custom nodes

var maszyna_environment_node_script = preload("res://addons/libmaszyna/environment/maszyna_environment_node.gd")
var maszyna_environment_node_icon = preload("res://addons/libmaszyna/environment/maszyna_environment_node_icon.png")
var e3d_model_instance_script = preload("res://addons/libmaszyna/e3d/e3d_model_instance.gd")
var e3d_model_instance_icon = preload("res://addons/libmaszyna/e3d/e3d_model_instance.png")

# Editor plugins

var e3d_submodel_toolbar = preload("res://addons/libmaszyna/editor/toolbar_e3d_instance.tscn")
var e3d_submodel_toolbar_instance
var user_settings_dock
var user_settings_dock_scene = preload("res://addons/libmaszyna/editor/user_settings_dock.tscn")

func _enter_tree():
    e3d_submodel_toolbar_instance = e3d_submodel_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)

    add_autoload_singleton("PlayerSystem", "res://addons/libmaszyna/player/player_system.gd")
    add_autoload_singleton("MultiPlayerManager", "res://addons/libmaszyna/player/multiplayer_manager.gd")
    add_autoload_singleton("SceneryResourceLoader", "res://addons/libmaszyna/scenery/scenery_resource_loader.gd")
    add_autoload_singleton("MaszynaEnvironment", "res://addons/libmaszyna/environment/maszyna_environment.gd")
    add_autoload_singleton("Console", "res://addons/libmaszyna/console/console.gd")
    add_autoload_singleton("MaterialManager", "res://addons/libmaszyna/materials/material_manager.gd")
    add_autoload_singleton("MaterialParser", "res://addons/libmaszyna/materials/material_parser.gd")
    add_autoload_singleton("E3DParser", "res://addons/libmaszyna/e3d/e3d_parser.gd")
    add_autoload_singleton("E3DModelManager", "res://addons/libmaszyna/e3d/e3d_model_manager.gd")
    add_autoload_singleton("E3DNodesInstancer", "res://addons/libmaszyna/e3d/e3d_nodes_instancer.gd")
    add_autoload_singleton("UserSettings", "res://addons/libmaszyna/settings/user_settings.gd")
    add_autoload_singleton("E3DModelInstanceManager", "res://addons/libmaszyna/e3d/e3d_model_instance_manager.gd")

    add_custom_type(
        "MaszynaEnvironmentNode",
        "Node",
        maszyna_environment_node_script,
        maszyna_environment_node_icon,
    )

    add_custom_type(
        "E3DModelInstance",
        "VisualInstance3D",
        e3d_model_instance_script,
        e3d_model_instance_icon,
    )

    ResourceLoader.add_resource_format_loader(e3d_loader)
    user_settings_dock = user_settings_dock_scene.instantiate()
    add_control_to_dock(DOCK_SLOT_RIGHT_UL, user_settings_dock)

    # MOVE TO USER SETTINGS!
    add_custom_project_setting("maszyna/game/multiplayer/host", "127.0.0.1", TYPE_STRING)
    add_custom_project_setting("maszyna/game/multiplayer/port", 9797, TYPE_INT)


func _exit_tree():
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

    if user_settings_dock:
        remove_control_from_docks(user_settings_dock)

    ResourceLoader.remove_resource_format_loader(e3d_loader)

    remove_custom_type("E3DModelInstance")
    remove_custom_type("MaszynaEnvironmentNode")

    remove_autoload_singleton("PlayerSystem")
    remove_autoload_singleton("MultiPlayerManager")
    remove_autoload_singleton("E3DModelInstanceManager")
    remove_autoload_singleton("UserSettings")
    remove_autoload_singleton("E3DNodesInstancer")
    remove_autoload_singleton("E3DModelManager")
    remove_autoload_singleton("E3DParser")
    remove_autoload_singleton("MaterialParser")
    remove_autoload_singleton("MaterialManager")
    remove_autoload_singleton("Console")
    remove_autoload_singleton("MaszynaEnvironment")
    remove_autoload_singleton("SceneryResourceLoader")


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
