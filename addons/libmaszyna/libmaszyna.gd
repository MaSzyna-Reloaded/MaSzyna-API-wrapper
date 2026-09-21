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
    add_autoload_singleton("E3DModelTool", "res://addons/libmaszyna/e3d/e3d_model_tool.gd")
    add_autoload_singleton("DynamicRailVehicle3DManager", "res://addons/libmaszyna/dynamic_rail_vehicle_3d_manager.gd")
    add_autoload_singleton("VehicleProfileManager", "res://addons/libmaszyna/vehicle_profile_manager.gd")
    add_autoload_singleton("AudioStreamManager", "res://addons/libmaszyna/sound/audio_stream_manager.gd")
    add_autoload_singleton("TrainSoundSystem", "res://addons/libmaszyna/sound/train_sound_system.gd")
    add_autoload_singleton("CabinSystem", "res://addons/libmaszyna/cabin/cabin_system.gd")
    add_autoload_singleton("FIZResourceLoaderRegistrar", "res://addons/libmaszyna/fiz/fiz_resource_loader_registrar.gd")
    add_autoload_singleton("TrackManager", "res://addons/libmaszyna/tracks/track_manager.gd")
    add_autoload_singleton("RailVehiclePhysicsServer", "res://addons/libmaszyna/servers/rail_vehicle_physics_server.gd")
    add_autoload_singleton("SceneryChunkRenderingServer", "res://addons/libmaszyna/servers/scenery_chunk_rendering_server.gd")

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
    remove_autoload_singleton("VehicleProfileManager")
    remove_autoload_singleton("DynamicRailVehicle3DManager")
    remove_autoload_singleton("E3DModelTool")
    remove_autoload_singleton("E3DModelManager")
    remove_autoload_singleton("MaterialFactory")
    remove_autoload_singleton("MaterialManager")
    remove_autoload_singleton("MaterialParser")
    remove_autoload_singleton("SceneryChunkRenderingServer")
    remove_autoload_singleton("Console")

func _enter_tree():
    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)
    add_custom_project_setting("maszyna/track_curve_bake_interval", 10.0, TYPE_FLOAT)
    # Quirk: the original renders shadow maps with front faces culled (opengl33renderer.cpp:1634)
    # against self-shadowing acne; Godot's default culls the same faces as the color pass
    add_custom_project_setting("maszyna/rendering/lights_shadow_reverse_cull_face", true, TYPE_BOOL)
    # E3DRenderingServer streams registered scenery models in and out around the camera; this caps
    # every node's own range and stands in for the nodes that declare none (read at startup)
    add_custom_project_setting(
        "maszyna/rendering/scenery_draw_distance", 3000.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "100.0,20000.0,10.0,suffix:m"
    )
    # A scenery light is streamed with a range of its own, far shorter than the model's: a street
    # lamp is visible from half a kilometre and lights fifteen metres. The densest 150 m of
    # stary_jawor holds 128 of them, which is why they cast no shadows by default.
    add_custom_project_setting(
        "maszyna/rendering/scenery_light_distance", 150.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "10.0,1000.0,5.0,suffix:m"
    )
    add_custom_project_setting("maszyna/rendering/scenery_lights_shadows", false, TYPE_BOOL)
    add_custom_project_setting(
        "maszyna/rendering/scenery_light_energy", 1.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,10.0,0.05"
    )
    # Sun altitude between which the light level ramps from night to full day; a scenery light set
    # to come on automatically lights below a level of 0.325 (AnimModel.cpp:598), which lands about
    # 1.4 degrees below the horizon on this ramp. A winter noon sun peaks at 16-19 degrees at 50 N,
    # so the day end must stay well below that (FINDINGS.md).
    add_custom_project_setting(
        "maszyna/rendering/light_level_night_altitude", -6.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "-18.0,0.0,0.5,suffix:°"
    )
    add_custom_project_setting(
        "maszyna/rendering/light_level_day_altitude", 6.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,15.0,0.5,suffix:°"
    )
    # The distance a vehicle stops rendering from a node hierarchy at and switches to
    # RenderingServer instances; it switches back 25% closer. 350 m is where E3DNodesBackend has
    # already faded its spotlights out completely (distance_fade_begin 150 + length 200), and the
    # OPTIMIZED backend's own lights are streamed by scenery_light_distance instead.
    add_custom_project_setting(
        "maszyna/rendering/vehicle_detail_distance", 350.0, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "50.0,10000.0,10.0,suffix:m"
    )
    add_custom_project_setting("maszyna/debug/physics_diagnostics", false, TYPE_BOOL)
    add_custom_project_setting(
        "maszyna/dds_maxtexturesize", 1024, TYPE_INT,
        PROPERTY_HINT_ENUM, "512,1024,2048,4096,8192"
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
        MaszynaSkyEnvironment.SHADOW_CABIN_MODE_SETTING, DirectionalLight3D.SHADOW_PARALLEL_2_SPLITS,
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
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_CURVE_SETTING, MaszynaSkyEnvironment.FOG_CURVE_DEFAULT, TYPE_FLOAT,
        PROPERTY_HINT_EXP_EASING, "positive_only"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_SKY_HEIGHT_SETTING, MaszynaSkyEnvironment.FOG_SKY_HEIGHT_DEFAULT,
        TYPE_FLOAT, PROPERTY_HINT_RANGE, "10.0,5000.0,10.0,or_greater,suffix:m"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_AERIAL_PERSPECTIVE_SETTING,
        MaszynaSkyEnvironment.FOG_AERIAL_PERSPECTIVE_DEFAULT, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.0,1.0,0.01"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.RAIN_FOG_DISTANCE_SETTING, MaszynaSkyEnvironment.RAIN_FOG_DISTANCE_DEFAULT,
        TYPE_FLOAT, PROPERTY_HINT_RANGE, "10.0,5000.0,10.0,or_greater,suffix:m"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.RAIN_FOG_DENSITY_SETTING, MaszynaSkyEnvironment.RAIN_FOG_DENSITY_DEFAULT,
        TYPE_FLOAT, PROPERTY_HINT_RANGE, "0.0,1.0,0.01"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_VOLUMETRIC_FAR_FALLOFF_SETTING,
        MaszynaSkyEnvironment.FOG_VOLUMETRIC_FAR_FALLOFF_DEFAULT, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "1.0,4.0,0.05"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_SCENERY_DISTANCE_FACTOR_SETTING,
        MaszynaSkyEnvironment.FOG_SCENERY_DISTANCE_FACTOR_DEFAULT, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.1,4.0,0.01,or_greater"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_DAY_DISTANCE_FACTOR_SETTING,
        MaszynaSkyEnvironment.FOG_DAY_DISTANCE_FACTOR_DEFAULT, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.01,4.0,0.0001,or_greater"
    )
    add_custom_project_setting(
        MaszynaSkyEnvironment.FOG_NIGHT_DISTANCE_FACTOR_SETTING,
        MaszynaSkyEnvironment.FOG_NIGHT_DISTANCE_FACTOR_DEFAULT, TYPE_FLOAT,
        PROPERTY_HINT_RANGE, "0.01,4.0,0.0001,or_greater"
    )
    add_import_plugin(fiz_import_plugin)

func _exit_tree():
    remove_import_plugin(fiz_import_plugin)
    print_verbose("Libmaszyna.gd _exit_tree finished!")

func add_custom_project_setting(name: String, default_value, type: int, hint: int = PROPERTY_HINT_NONE, hint_string: String = "") -> void:
    var setting_info: Dictionary = {
        "name": name,
        "type": type,
        "hint": hint,
        "hint_string": hint_string
    }

    # project.godot keeps only the value - the hint and the initial value are gone with every
    # editor restart, so a setting saved there still needs them registered again
    if not ProjectSettings.has_setting(name):
        ProjectSettings.set_setting(name, default_value)
    ProjectSettings.add_property_info(setting_info)
    ProjectSettings.set_initial_value(name, default_value)
