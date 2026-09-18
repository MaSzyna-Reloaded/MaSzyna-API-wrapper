@tool
extends MaszynaSkyEnvironment
class_name TokisanSky3DMaszynaEnvironment

const SKY_SHADER: Shader = preload("res://addons/sky_3d/shaders/SkyMaterial.gdshader")
const SKY_DOME_NAME: StringName = &"SkyDome"
const TIME_OF_DAY_NAME: StringName = &"TimeOfDay"
const CELESTIAL_FADE_CLOUDINESS_START: float = 0.5
const SUN_DISK_INTENSITY: float = 30.0
const MOON_COLOR: Color = Color.WHITE
const STARMAP_COLOR: Color = Color(0.709804, 0.709804, 0.709804, 0.854902)
const STAR_FIELD_COLOR: Color = Color.WHITE
# Fog baseline scaled by MaszynaEnvironmentNode.fog_density / fog_range.
const FOG_DENSITY: float = 0.2
const FOG_RANGE_START: float = 200.0
const FOG_RANGE_END: float = 1000.0
const MINIMUM_VOLUMETRIC_FOG_LENGTH: float = 64.0
const MAXIMUM_FOG_OPACITY: float = 0.999
const VOLUMETRIC_FOG_OPACITY_SCALE: float = 0.1

var sky_dome: SkyDome
var time_of_day: TimeOfDay
var sun_light: DirectionalLight3D
var moon_light: DirectionalLight3D
var environment: Environment


func create_sky() -> Sky:
    var sky_material: ShaderMaterial = ShaderMaterial.new()
    sky_material.shader = SKY_SHADER

    var sky: Sky = Sky.new()
    sky.sky_material = sky_material
    return sky


func create_nodes(world_environment: WorldEnvironment, p_environment: Environment) -> void:
    environment = p_environment

    sun_light = DirectionalLight3D.new()
    sun_light.name = &"SunLight"
    sun_light.shadow_enabled = true
    world_environment.add_child(sun_light, false, Node.INTERNAL_MODE_BACK)

    moon_light = DirectionalLight3D.new()
    moon_light.name = &"MoonLight"
    moon_light.shadow_enabled = true
    world_environment.add_child(moon_light, false, Node.INTERNAL_MODE_BACK)

    sky_dome = SkyDome.new()
    sky_dome.name = SKY_DOME_NAME
    sky_dome.environment = environment
    sky_dome.fog_visible = false
    world_environment.add_child(sky_dome, false, Node.INTERNAL_MODE_BACK)
    sky_dome.sun_light_path = NodePath("../SunLight")
    sky_dome.moon_light_path = NodePath("../MoonLight")

    time_of_day = TimeOfDay.new()
    time_of_day.name = TIME_OF_DAY_NAME
    time_of_day.editor_time_enabled = false
    world_environment.add_child(time_of_day, false, Node.INTERNAL_MODE_BACK)
    time_of_day.dome_path = NodePath("../SkyDome")


func bind_nodes(world_environment: WorldEnvironment) -> void:
    environment = world_environment.environment
    sun_light = world_environment.get_node_or_null("SunLight") as DirectionalLight3D
    moon_light = world_environment.get_node_or_null("MoonLight") as DirectionalLight3D
    sky_dome = world_environment.get_node_or_null(NodePath(SKY_DOME_NAME)) as SkyDome
    time_of_day = world_environment.get_node_or_null(NodePath(TIME_OF_DAY_NAME)) as TimeOfDay


func apply_visual_configuration() -> void:
    if not sky_dome:
        return

    var clouds_visible: bool = environment_node.cloudiness > 0.0
    var celestial_visibility: float = 1.0 - smoothstep(
        CELESTIAL_FADE_CLOUDINESS_START, 1.0, environment_node.cloudiness
    )

    sky_dome.cirrus_visible = clouds_visible
    sky_dome.cumulus_visible = clouds_visible
    sky_dome.cirrus_coverage = environment_node.cloudiness
    sky_dome.cumulus_coverage = clampf(environment_node.cloudiness * 1.1, 0.0, 1.0)
    sky_dome.wind_direction = environment_node.wind_direction

    # Tokisan's screen-space fog bypasses Godot's Environment fog.
    sky_dome.fog_visible = false
    _apply_fog_configuration()

    sky_dome.sun_disk_intensity = SUN_DISK_INTENSITY * celestial_visibility
    sky_dome.moon_color = MOON_COLOR.lerp(
        Color(0.0, 0.0, 0.0, 1.0), 1.0 - celestial_visibility
    )
    sky_dome.starmap_color = STARMAP_COLOR.lerp(
        Color(0.0, 0.0, 0.0, 1.0), 1.0 - celestial_visibility
    )
    sky_dome.star_field_color = STAR_FIELD_COLOR.lerp(
        Color(0.0, 0.0, 0.0, 1.0), 1.0 - celestial_visibility
    )


func _apply_fog_configuration() -> void:
    var fog_density: float = clampf(FOG_DENSITY * environment_node.fog_density, 0.0, 1.0)
    var fog_end: float = FOG_RANGE_END * environment_node.fog_range
    var volumetric_fog_length: float = maxf(fog_end, MINIMUM_VOLUMETRIC_FOG_LENGTH)

    environment.fog_density = fog_density
    environment.fog_depth_begin = FOG_RANGE_START * environment_node.fog_range
    environment.fog_depth_end = fog_end
    environment.fog_sky_affect = fog_density
    environment.volumetric_fog_density = _fog_opacity_to_exponential_density(
        fog_density * VOLUMETRIC_FOG_OPACITY_SCALE, volumetric_fog_length
    )
    environment.volumetric_fog_length = volumetric_fog_length
    environment.volumetric_fog_sky_affect = 1.0


func _fog_opacity_to_exponential_density(opacity: float, distance: float) -> float:
    var effective_opacity: float = clampf(opacity, 0.0, MAXIMUM_FOG_OPACITY)
    return -log(1.0 - effective_opacity) / distance


func apply_light_configuration() -> void:
    _apply_directional_light_settings(sun_light)
    _apply_directional_light_settings(moon_light)


func set_date(year: int, month: int, day: int) -> Vector3i:
    if not time_of_day:
        return Vector3i(year, month, day)

    # The order matters because TimeOfDay normalizes each component in its setter.
    time_of_day.year = year
    time_of_day.month = month
    time_of_day.day = day
    return Vector3i(time_of_day.year, time_of_day.month, time_of_day.day)


func apply_time_configuration() -> void:
    if not time_of_day:
        return

    time_of_day.editor_time_enabled = false
    time_of_day.system_sync = environment_node.use_system_time
    time_of_day.minutes_per_day = (
        1440.0 / environment_node.simulation_speed
        if environment_node.simulation_speed > 0.0
        else 0.0
    )
    time_of_day.update_interval = time_of_day.minutes_per_day * 0.001
    time_of_day.latitude = deg_to_rad(environment_node.latitude)
    time_of_day.longitude = deg_to_rad(environment_node.longitude)
    time_of_day.utc = float(environment_node.timezone_offset)

    if environment_node.use_system_time:
        time_of_day.set_from_datetime_dict(Time.get_datetime_dict_from_system())
    else:
        set_date(environment_node.year, environment_node.month, environment_node.day)
        time_of_day.current_time = environment_node.current_time


func get_date() -> Vector3i:
    if not time_of_day:
        return Vector3i.ZERO
    return Vector3i(time_of_day.year, time_of_day.month, time_of_day.day)


func get_current_time() -> float:
    return time_of_day.current_time if time_of_day else 0.0
