@tool
@abstract
extends RefCounted
class_name MaszynaSkyEnvironment

const SHADOW_ENABLED_SETTING: StringName = &"maszyna/rendering/shadow_enabled"
const SHADOW_MODE_SETTING: StringName = &"maszyna/rendering/shadow_mode"
## The cabin view fills the screen with close surfaces, where every PSSM split costs a full
## screen of filtering - fewer splits than outside are enough for the few metres it needs
const SHADOW_CABIN_MODE_SETTING: StringName = &"maszyna/rendering/shadow_cabin_mode"
const SHADOW_BLUR_SETTING: StringName = &"maszyna/rendering/shadow_blur"
const SHADOW_OPACITY_SETTING: StringName = &"maszyna/rendering/shadow_opacity"
const SHADOW_BIAS_SETTING: StringName = &"maszyna/rendering/shadow_bias"
const SHADOW_NORMAL_BIAS_SETTING: StringName = &"maszyna/rendering/shadow_normal_bias"
const SHADOW_EXTERIOR_MAX_DISTANCE_SETTING: StringName = &"maszyna/rendering/shadow_exterior_max_distance"
const SHADOW_CABIN_MAX_DISTANCE_SETTING: StringName = &"maszyna/rendering/shadow_cabin_max_distance"
const SHADOW_BLEND_SPLITS_SETTING: StringName = &"maszyna/rendering/shadow_blend_splits"
const SHADOW_EXTERIOR_SPLIT_SETTINGS: Array[StringName] = [
    &"maszyna/rendering/shadow_exterior_split_1",
    &"maszyna/rendering/shadow_exterior_split_2",
    &"maszyna/rendering/shadow_exterior_split_3",
]
const SHADOW_CABIN_SPLIT_SETTINGS: Array[StringName] = [
    &"maszyna/rendering/shadow_cabin_split_1",
    &"maszyna/rendering/shadow_cabin_split_2",
    &"maszyna/rendering/shadow_cabin_split_3",
]
## Godot's DirectionalLight3D defaults for the exterior, the cabin keeps most of the map up close
const SHADOW_EXTERIOR_SPLITS: Array[float] = [0.1, 0.2, 0.5]
const SHADOW_CABIN_SPLITS: Array[float] = [0.01, 0.02, 0.2]
const VOLUMETRIC_FOG_ENERGY_SETTING: StringName = &"maszyna/rendering/volumetric_fog_energy"

var environment_node: Node


func _init(node: Node) -> void:
    environment_node = node


@abstract func create_sky() -> Sky


@abstract func create_nodes(
    world_environment: WorldEnvironment, environment: Environment
) -> void


@abstract func bind_nodes(world_environment: WorldEnvironment) -> void


@abstract func apply_visual_configuration() -> void


## Applies the maszyna/rendering/* light settings to the backend's directional lights.
@abstract func apply_light_configuration() -> void


@abstract func set_date(year: int, month: int, day: int) -> Vector3i


@abstract func apply_time_configuration() -> void


@abstract func get_date() -> Vector3i


@abstract func get_current_time() -> float


func process(_delta: float) -> void:
    pass


func _apply_directional_light_settings(light: DirectionalLight3D) -> void:
    light.shadow_enabled = bool(ProjectSettings.get_setting(SHADOW_ENABLED_SETTING, true))
    light.shadow_blur = float(ProjectSettings.get_setting(SHADOW_BLUR_SETTING, 1.0))
    light.shadow_opacity = float(ProjectSettings.get_setting(SHADOW_OPACITY_SETTING, 1.0))
    light.shadow_bias = float(ProjectSettings.get_setting(SHADOW_BIAS_SETTING, 0.1))
    light.shadow_normal_bias = float(ProjectSettings.get_setting(SHADOW_NORMAL_BIAS_SETTING, 2.0))
    # the cabin view needs sharp near shadows only, the exterior view far ones
    var cabin_view: bool = (environment_node as MaszynaEnvironmentNode).cabin_view
    light.directional_shadow_mode = int(
        ProjectSettings.get_setting(SHADOW_CABIN_MODE_SETTING, DirectionalLight3D.SHADOW_PARALLEL_2_SPLITS)
        if cabin_view
        else ProjectSettings.get_setting(SHADOW_MODE_SETTING, DirectionalLight3D.SHADOW_PARALLEL_4_SPLITS)
    ) as DirectionalLight3D.ShadowMode
    light.directional_shadow_max_distance = (
        float(ProjectSettings.get_setting(SHADOW_CABIN_MAX_DISTANCE_SETTING, 150.0))
        if cabin_view
        else float(ProjectSettings.get_setting(SHADOW_EXTERIOR_MAX_DISTANCE_SETTING, 100.0))
    )
    var split_settings: Array[StringName] = (
        SHADOW_CABIN_SPLIT_SETTINGS if cabin_view else SHADOW_EXTERIOR_SPLIT_SETTINGS
    )
    var split_defaults: Array[float] = SHADOW_CABIN_SPLITS if cabin_view else SHADOW_EXTERIOR_SPLITS
    light.directional_shadow_split_1 = float(ProjectSettings.get_setting(split_settings[0], split_defaults[0]))
    light.directional_shadow_split_2 = float(ProjectSettings.get_setting(split_settings[1], split_defaults[1]))
    light.directional_shadow_split_3 = float(ProjectSettings.get_setting(split_settings[2], split_defaults[2]))
    light.directional_shadow_blend_splits = bool(ProjectSettings.get_setting(SHADOW_BLEND_SPLITS_SETTING, true))
    light.light_volumetric_fog_energy = float(
        ProjectSettings.get_setting(VOLUMETRIC_FOG_ENERGY_SETTING, 1.0)
    )
