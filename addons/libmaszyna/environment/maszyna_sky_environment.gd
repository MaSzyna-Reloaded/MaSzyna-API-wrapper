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
## MaszynaEnvironmentNode.fog_distance is scaled by these for the day and for the night fog of a
## sky backend that tells them apart; the night default keeps Skydome's own 200 m to 470 m ratio
const FOG_DAY_DISTANCE_FACTOR_SETTING: StringName = &"maszyna/rendering/fog_day_distance_factor"
const FOG_NIGHT_DISTANCE_FACTOR_SETTING: StringName = &"maszyna/rendering/fog_night_distance_factor"
## How the fog builds up with the distance z from the camera: opacity = fog_density *
## (z / fog_distance) ^ exponent (Environment.fog_depth_curve). 1 - evenly, above 1 - clear near the
## camera and thickening further away, below 1 - thick right away. It has nothing to do with how
## fog_density itself is scaled. The original's fog is 1 - exp(-(z / range)^2) (apply_fog.glsl:16);
## an exponent of 1.5 with the fog complete at 1.5 of that range starts as slowly as it does
## (19/54/100% against 22/63/90% at 0.5/1/1.5 of the range) - hence the two defaults.
## How the fog builds up from the camera to MaszynaEnvironmentNode.fog_distance
## (Environment.fog_depth_curve), drawn by the editor as it works: x is the distance (0 at the
## camera, 1 at fog_distance), y the share of fog_density reached there - share = x ^ curve.
## 1 - evenly, above 1 - clear near the camera and thickening further away, below 1 - thick right
## away; FOG_CURVE_MIN keeps it from becoming a wall of fog in front of the camera. The original's
## fog is 1 - exp(-(z / range)^2) (apply_fog.glsl:16); a curve of 1.5 with the fog complete at 1.5
## of that range starts as slowly as it does (19/54/100% against 22/63/90% at 0.5/1/1.5 of the
## range) - hence the two defaults.
const FOG_CURVE_SETTING: StringName = &"maszyna/rendering/fog_curve"
const FOG_CURVE_DEFAULT: float = 1.5
const FOG_CURVE_MIN: float = 0.5
## How much fog there is looking up, as a distance: the sky takes the share of the fog a terrain
## that far away would (share = (height / fog_distance) ^ fog_curve, at most 1). A fog reaching
## kilometres is a thin layer and leaves the stars, the moon and the clouds alone; a fog of a
## hundred metres hides them. Rain is no thin layer - it fills the air all the way up - so the fog
## a downpour brings reaches the sky in full.
const FOG_SKY_HEIGHT_SETTING: StringName = &"maszyna/rendering/fog_sky_height"
const FOG_SKY_HEIGHT_DEFAULT: float = 1000.0
## How much the fog takes the colour of the sky's radiance instead of its own
## (Environment.fog_aerial_perspective). Off by default: at dusk that radiance is as dark as the
## scene itself, so a fully fogged object stays a dark silhouette against a brighter sky instead of
## fading out - the fog stops reading as fog.
const FOG_AERIAL_PERSPECTIVE_SETTING: StringName = &"maszyna/rendering/fog_aerial_perspective"
const FOG_AERIAL_PERSPECTIVE_DEFAULT: float = 0.0
## A downpour brings a fog of its own, whatever fog is set: towards a full precipitation the fog
## distance shortens to this visibility and the fog density rises to this opacity (never the other
## way - a fog already closer or thicker stays as it is).
const RAIN_FOG_DISTANCE_SETTING: StringName = &"maszyna/rendering/rain_fog_distance"
const RAIN_FOG_DISTANCE_DEFAULT: float = 800.0
const RAIN_FOG_DENSITY_SETTING: StringName = &"maszyna/rendering/rain_fog_density"
const RAIN_FOG_DENSITY_DEFAULT: float = 0.6
## How fast the fog right in front of the camera (the volumetric layer) dies out once fog_distance
## grows past the distance the sky backend is tuned for: its density follows
## (reference / fog_distance) ^ falloff there. 1 - as below that distance; 2 - a fog reaching
## kilometres leaves the view in front of the camera as crisp as no fog at all.
const FOG_VOLUMETRIC_FAR_FALLOFF_SETTING: StringName = &"maszyna/rendering/fog_volumetric_far_falloff"
const FOG_VOLUMETRIC_FAR_FALLOFF_DEFAULT: float = 2.0
## fog_distance of a scenery that declares its fog, as a multiple of the original's fog range
const FOG_SCENERY_DISTANCE_FACTOR_SETTING: StringName = &"maszyna/rendering/fog_scenery_distance_factor"
const FOG_SCENERY_DISTANCE_FACTOR_DEFAULT: float = 1.5
const FOG_DAY_DISTANCE_FACTOR_DEFAULT: float = 1.0
const FOG_NIGHT_DISTANCE_FACTOR_DEFAULT: float = 0.4255

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
