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
    add_autoload_singleton("DynamicRailVehicle3DManager", "res://addons/libmaszyna/dynamic_rail_vehicle_3d_manager.gd")
    add_autoload_singleton("AudioStreamManager", "res://addons/libmaszyna/sound/audio_stream_manager.gd")
    add_autoload_singleton("TrainSoundSystem", "res://addons/libmaszyna/sound/train_sound_system.gd")
    add_autoload_singleton("CabinSystem", "res://addons/libmaszyna/cabin/cabin_system.gd")
    add_autoload_singleton("FIZResourceLoaderRegistrar", "res://addons/libmaszyna/fiz/fiz_resource_loader_registrar.gd")
    add_autoload_singleton("TrackManager", "res://addons/libmaszyna/tracks/track_manager.gd")
    add_autoload_singleton("RailVehiclePhysicsServer", "res://addons/libmaszyna/servers/rail_vehicle_physics_server.gd")

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
        "VisualInstance3D",
        track_3d_script,
        null
    )

    add_custom_type(
        "TrackNormal3D",
        "VisualInstance3D",
        track_normal_3d_script,
        null
    )

    add_custom_type(
        "TrackSwitch3D",
        "VisualInstance3D",
        track_switch_3d_script,
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
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/scenery_toolbar", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/tracks", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/nodebank", true)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/user_settings_dock", true)

func _disable_plugin():
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/e3d_toolbar", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/fiz_toolbar", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/scenery_toolbar", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/tracks", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/nodebank", false)
    EditorInterface.set_plugin_enabled(PLUGIN_NAME + "/editor/user_settings_dock", false)

    remove_custom_type("E3DModelInstance")
    remove_custom_type("MaszynaEnvironmentNode")
    remove_custom_type("Track3D")
    remove_custom_type("TrackNormal3D")
    remove_custom_type("TrackSwitch3D")
    remove_custom_type("FIZTrainController")

    remove_autoload_singleton("TrainSoundSystem")
    remove_autoload_singleton("CabinSystem")
    remove_autoload_singleton("RailVehiclePhysicsServer")
    remove_autoload_singleton("TrackManager")
    remove_autoload_singleton("AudioStreamManager")
    remove_autoload_singleton("FIZResourceLoaderRegistrar")
    remove_autoload_singleton("DynamicRailVehicle3DManager")
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
    # Quirk: the original renders shadow maps with front faces culled (opengl33renderer.cpp:1634)
    # against self-shadowing acne; Godot's default culls the same faces as the color pass
    add_custom_project_setting("maszyna/rendering/lights_shadow_reverse_cull_face", true, TYPE_BOOL)
    add_custom_project_setting("maszyna/debug/physics_diagnostics", false, TYPE_BOOL)
    add_custom_project_setting(
        "maszyna/dds_maxtexturesize", 1024, TYPE_INT,
        PROPERTY_HINT_ENUM, "512,1024,2048,4096,8192"
    )
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
    add_custom_project_setting(
        "maszyna/sound/culling_distance", 1000.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,5000.0,10.0,or_greater"
    )
    add_custom_project_setting(
        "maszyna/weather/wind_turbulence", 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,4.0,0.01,or_greater"
    )
    add_custom_project_setting(MaszynaSkyEnvironment.SHADOW_ENABLED_SETTING, true, TYPE_BOOL)
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_MODE_SETTING, DirectionalLight3D.SHADOW_PARALLEL_4_SPLITS,
        TYPE_INT, PROPERTY_HINT_ENUM, "Orthogonal,PSSM 2 Splits,PSSM 4 Splits"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_BLUR_SETTING, 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,10.0,0.01,or_greater"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_OPACITY_SETTING, 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,1.0,0.01"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_BIAS_SETTING, 0.1, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,10.0,0.001,or_greater"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_NORMAL_BIAS_SETTING, 2.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,10.0,0.001,or_greater"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_EXTERIOR_MAX_DISTANCE_SETTING, 100.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,10000.0,1.0,suffix:m"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.SHADOW_CABIN_MAX_DISTANCE_SETTING, 150.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,10000.0,1.0,suffix:m"
    )
    add_custom_project_setting(MaszynaSkyEnvironment.SHADOW_BLEND_SPLITS_SETTING, true, TYPE_BOOL)
    for i: int in 3:
        add_custom_project_setting(
            MaszynaSkyEnvironment.SHADOW_EXTERIOR_SPLIT_SETTINGS[i], MaszynaSkyEnvironment.SHADOW_EXTERIOR_SPLITS[i],
            TYPE_FLOAT, PROPERTY_HINT_RANGE, "0.0,1.0,0.001"
        )
        add_custom_project_setting(
            MaszynaSkyEnvironment.SHADOW_CABIN_SPLIT_SETTINGS[i], MaszynaSkyEnvironment.SHADOW_CABIN_SPLITS[i],
            TYPE_FLOAT, PROPERTY_HINT_RANGE, "0.0,1.0,0.001"
        )
    add_custom_project_setting(
        MaszynaSkyEnvironment.VOLUMETRIC_FOG_ENERGY_SETTING, 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,16.0,0.001,or_greater"
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
