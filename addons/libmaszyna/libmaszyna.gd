@tool
extends EditorPlugin

# Custom nodes
const PLUGIN_NAME = "libmaszyna"

var maszyna_environment_node_script = preload("res://addons/libmaszyna/environment/maszyna_environment_node.gd")
var maszyna_environment_node_icon = preload("res://addons/libmaszyna/environment/maszyna_environment_node_icon.png")
var e3d_model_instance_script = preload("res://addons/libmaszyna/e3d/e3d_model_instance.gd")
var e3d_model_instance_icon = preload("res://addons/libmaszyna/e3d/e3d_model_instance.png")
var maszyna_track_3d_script = preload("res://addons/libmaszyna/maszyna_track_3d.gd")
var maszyna_switch_3d_script = preload("res://addons/libmaszyna/maszyna_switch_3d.gd")
var fiz_train_controller_script = preload("res://addons/libmaszyna/fiz/fiz_train_controller.gd")
var fiz_import_plugin = preload("res://addons/libmaszyna/fiz/fiz_import_plugin.gd").new()

func _enable_plugin():
    add_autoload_singleton("Console", "res://addons/libmaszyna/console/console.gd")
    add_autoload_singleton("MaterialManager", "res://addons/libmaszyna/materials/material_manager.gd")
    add_autoload_singleton("MaterialParser", "res://addons/libmaszyna/materials/material_parser.gd")
    add_autoload_singleton("MaterialFactory", "res://addons/libmaszyna/materials/material_factory.gd")
    add_autoload_singleton("E3DModelManager", "res://addons/libmaszyna/e3d/e3d_model_manager.gd")
    add_autoload_singleton("E3DNodesInstancer", "res://addons/libmaszyna/e3d/e3d_nodes_instancer.gd")
    add_autoload_singleton("E3DModelTool", "res://addons/libmaszyna/e3d/e3d_model_tool.gd")
    add_autoload_singleton("AudioStreamManager", "res://addons/libmaszyna/sound/audio_stream_manager.gd")
    add_autoload_singleton("FIZResourceLoaderRegistrar", "res://addons/libmaszyna/fiz/fiz_resource_loader_registrar.gd")

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

    add_custom_type(
        "MaszynaTrack3D",
        "Path3D",
        maszyna_track_3d_script,
        null
    )

    add_custom_type(
        "MaszynaSwitch3D",
        "Node3D",
        maszyna_switch_3d_script,
        null
    )

    add_custom_type(
        "FIZTrainController",
        "Node",
        fiz_train_controller_script,
        null
    )

    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/e3d_toolbar", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/fiz_toolbar", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/nodebank", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/user_settings_dock", true)

func _disable_plugin():
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/e3d_toolbar", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/fiz_toolbar", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/nodebank", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/user_settings_dock", false)

    remove_custom_type("E3DModelInstance")
    remove_custom_type("MaszynaEnvironmentNode")
    remove_custom_type("MaszynaTrack3D")
    remove_custom_type("MaszynaSwitch3D")
    remove_custom_type("FIZTrainController")

    remove_autoload_singleton("AudioStreamManager")
    remove_autoload_singleton("FIZResourceLoaderRegistrar")
    remove_autoload_singleton("E3DModelTool")
    remove_autoload_singleton("E3DNodesInstancer")
    remove_autoload_singleton("E3DModelManager")
    remove_autoload_singleton("MaterialFactory")
    remove_autoload_singleton("MaterialManager")
    remove_autoload_singleton("MaterialParser")
    remove_autoload_singleton("Console")

func _enter_tree():
    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)
    add_custom_project_setting(
        "maszyna/sound/brake_volume_factor", 2.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,4.0,0.05,or_greater"
    )
    add_custom_project_setting(
        "maszyna/sound/brake_exterior_volume_factor", 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,4.0,0.05,or_greater"
    )
    add_custom_project_setting(
        "maszyna/sound/brake_cabin_unit_size_factor", 2.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.1,8.0,0.05,or_greater"
    )
    add_custom_project_setting(
        "maszyna/sound/brake_exterior_unit_size_factor", 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.1,8.0,0.05,or_greater"
    )
    add_import_plugin(fiz_import_plugin)

func _exit_tree():
    remove_import_plugin(fiz_import_plugin)
    print_verbose("Libmaszyna.gd _exit_tree finished!")

func add_custom_project_setting(name: String, default_value, type: int, hint: int = PROPERTY_HINT_NONE, hint_string: String = "") -> void:
    if ProjectSettings.has_setting(name):
        return

    var setting_info: Dictionary = {
        "name": name,
        "type": type,
        "hint": hint,
        "hint_string": hint_string
    }

    ProjectSettings.set_setting(name, default_value)
    ProjectSettings.add_property_info(setting_info)
    ProjectSettings.set_initial_value(name, default_value)
