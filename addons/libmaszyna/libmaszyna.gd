@tool
extends EditorPlugin

# Custom nodes
const PLUGIN_NAME = "libmaszyna"

var maszyna_environment_node_script = preload("res://addons/libmaszyna/environment/maszyna_environment_node.gd")
var maszyna_environment_node_icon = preload("res://addons/libmaszyna/environment/maszyna_environment_node_icon.png")
var e3d_model_instance_script = preload("res://addons/libmaszyna/e3d/e3d_model_instance.gd")
var e3d_model_instance_icon = preload("res://addons/libmaszyna/e3d/e3d_model_instance.png")
var track_3d_script = preload("res://addons/libmaszyna/tracks/track_3d.gd")
var track_normal_3d_script = preload("res://addons/libmaszyna/tracks/track_normal_3d.gd")
var track_switch_3d_script = preload("res://addons/libmaszyna/tracks/track_switch_3d.gd")

func _enable_plugin():
    add_autoload_singleton("Console", "res://addons/libmaszyna/console/console.gd")
    add_autoload_singleton("MaterialManager", "res://addons/libmaszyna/materials/material_manager.gd")
    add_autoload_singleton("MaterialParser", "res://addons/libmaszyna/materials/material_parser.gd")
    add_autoload_singleton("MaterialFactory", "res://addons/libmaszyna/materials/material_factory.gd")
    add_autoload_singleton("E3DModelManager", "res://addons/libmaszyna/e3d/e3d_model_manager.gd")
    add_autoload_singleton("E3DNodesInstancer", "res://addons/libmaszyna/e3d/e3d_nodes_instancer.gd")
    add_autoload_singleton("E3DModelTool", "res://addons/libmaszyna/e3d/e3d_model_tool.gd")
    add_autoload_singleton("AudioStreamManager", "res://addons/libmaszyna/sound/audio_stream_manager.gd")
    add_autoload_singleton("TrackManager", "res://addons/libmaszyna/tracks/track_manager.gd")
    add_autoload_singleton("RailVehiclePhysicsServer", "res://addons/libmaszyna/vehicle/rail_vehicle_physics_server.gd")

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
        "Track3D",
        "Node3D",
        track_3d_script,
        null
    )

    add_custom_type(
        "TrackNormal3D",
        "Node3D",
        track_normal_3d_script,
        null
    )

    add_custom_type(
        "TrackSwitch3D",
        "Node3D",
        track_switch_3d_script,
        null
    )

    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/e3d_toolbar", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/tracks", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/nodebank", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/user_settings_dock", true)

func _disable_plugin():
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/e3d_toolbar", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/tracks", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/nodebank", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/user_settings_dock", false)

    remove_custom_type("E3DModelInstance")
    remove_custom_type("MaszynaEnvironmentNode")
    remove_custom_type("Track3D")
    remove_custom_type("TrackNormal3D")
    remove_custom_type("TrackSwitch3D")

    remove_autoload_singleton("RailVehiclePhysicsServer")
    remove_autoload_singleton("TrackManager")
    remove_autoload_singleton("AudioStreamManager")
    remove_autoload_singleton("E3DModelTool")
    remove_autoload_singleton("E3DNodesInstancer")
    remove_autoload_singleton("E3DModelManager")
    remove_autoload_singleton("MaterialFactory")
    remove_autoload_singleton("MaterialManager")
    remove_autoload_singleton("MaterialParser")
    remove_autoload_singleton("Console")

func _enter_tree():
    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)
    add_custom_project_setting("maszyna/track_curve_bake_interval", 10.0, TYPE_FLOAT)

func _exit_tree():
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
