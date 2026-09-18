@tool
@abstract
extends RefCounted
class_name MaszynaSkyEnvironment

const SHADOW_ENABLED_SETTING: StringName = &"maszyna/rendering/shadow_enabled"
const SHADOW_MODE_SETTING: StringName = &"maszyna/rendering/shadow_mode"
const SHADOW_BLUR_SETTING: StringName = &"maszyna/rendering/shadow_blur"
const SHADOW_OPACITY_SETTING: StringName = &"maszyna/rendering/shadow_opacity"
const SHADOW_BIAS_SETTING: StringName = &"maszyna/rendering/shadow_bias"
const SHADOW_NORMAL_BIAS_SETTING: StringName = &"maszyna/rendering/shadow_normal_bias"
const SHADOW_MAX_DISTANCE_SETTING: StringName = &"maszyna/rendering/shadow_max_distance"
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
    light.directional_shadow_mode = int(ProjectSettings.get_setting(
        SHADOW_MODE_SETTING, DirectionalLight3D.SHADOW_PARALLEL_4_SPLITS
    )) as DirectionalLight3D.ShadowMode
    light.shadow_blur = float(ProjectSettings.get_setting(SHADOW_BLUR_SETTING, 1.0))
    light.shadow_opacity = float(ProjectSettings.get_setting(SHADOW_OPACITY_SETTING, 1.0))
    light.shadow_bias = float(ProjectSettings.get_setting(SHADOW_BIAS_SETTING, 0.1))
    light.shadow_normal_bias = float(ProjectSettings.get_setting(SHADOW_NORMAL_BIAS_SETTING, 2.0))
    light.directional_shadow_max_distance = float(
        ProjectSettings.get_setting(SHADOW_MAX_DISTANCE_SETTING, 100.0)
    )
    light.light_volumetric_fog_energy = float(
        ProjectSettings.get_setting(VOLUMETRIC_FOG_ENERGY_SETTING, 1.0)
    )
