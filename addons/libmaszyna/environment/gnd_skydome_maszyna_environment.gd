@tool
extends MaszynaSkyEnvironment
class_name GndSkydomeMaszynaEnvironment

const SKYDOME_NAME: StringName = &"Skydome"
const SUN_LIGHT_NAME: StringName = &"SunLight"
const WEATHER_NAME: StringName = &"Weather"
const WIND_TURBULENCE_SETTING: StringName = &"maszyna/weather/wind_turbulence"
# Weather response to rain and wind strength, as in forest-test-scene ui/WeatherControlsCanvas.gd.
const WIND_SPEED_MIN: float = 0.15
const WIND_SPEED_MAX: float = 3.0
const WIND_STRENGTH_MIN: float = 0.4
const WIND_STRENGTH_MAX: float = 5.0
const STORM_RAIN_START: float = 0.4
# Precipitation the rain starts to bring its own fog from (RAIN_FOG_DISTANCE_SETTING,
# RAIN_FOG_DENSITY_SETTING), in full at 1.0
const RAIN_FOG_START: float = 0.6
# Skydome's day/night densities are tuned for exponential fog and barely show in depth fog, so the
# visible fog comes from Skydome's fog_density boost, which is the fog opacity at the fog distance.
# The volumetric fog and the clouds and rain Skydome blends on top are tuned for this opacity
# (0.15 in forest-test-scene) and this day distance, and follow the node proportionally.
const FOG_REFERENCE_DENSITY: float = 0.15
const FOG_REFERENCE_DISTANCE_PROPERTY: StringName = &"day_fog_distance"
const FOG_DENSITY_PROPERTIES: Array[StringName] = [&"day_fog_density", &"night_fog_density"]
# Volumetric fog is an extinction per metre over a volume of Skydome's own length in front of the
# camera, not an opacity at a distance: it has to thin out as the fog distance grows, or it fills
# that volume up while the depth fog recedes. The volume length stays Skydome's.
const FOG_VOLUMETRIC_DENSITY_PROPERTIES: Array[StringName] = [&"day_vol_fog_density", &"night_vol_fog_density"]
const FOG_RANGE_PROPERTIES: Array[StringName] = [&"day_fog_distance_begin", &"night_fog_distance_begin"]
# How much of the depth fog reaches the sky follows the fog distance (FOG_SKY_HEIGHT_SETTING);
# Skydome's own pull of it towards 1.0 with a growing fog is switched off.
const FOG_SKY_AFFECT_PROPERTIES: Array[StringName] = [&"day_fog_sky_affect", &"night_fog_sky_affect"]
# The volumetric fog is a volume right in front of the camera, so it stands in front of the sky as
# much as in front of anything else: left out of the sky, fog lit by the headlights ends along the
# silhouette of whatever stands against it.
const FOG_VOLUMETRIC_SKY_AFFECT_PROPERTIES: Array[StringName] = [
    &"day_vol_fog_sky_affect", &"night_vol_fog_sky_affect",
]
const RAINBOW_INTENSITY_MAX: float = 0.12
const RAINBOW_RAIN_START: float = 0.2
const RAINBOW_RAIN_END: float = 0.5
const RAINBOW_CLOUD_FADE_START: float = 0.2
const RAINBOW_CLOUD_FADE_END: float = 0.5
const SECONDS_PER_HOUR: float = 3600.0
const SECONDS_PER_DAY: int = 86400
const MAXIMUM_DAY_OF_YEAR: int = 365
# Time is pushed to Skydome at a fixed rate and Skydome interpolates in between
# (same as WorldTimer in forest-test-scene levels/test_biomes.gd:227).
const TIME_UPDATE_INTERVAL: float = 0.1

var skydome: Skydome
var sun_light: DirectionalLight3D
var weather: WeatherNode

var _year: int = 2026
var _month: int = 1
var _day: int = 1
var _current_time: float = 0.0
var _time_update_elapsed: float = 0.0


func create_sky() -> Sky:
    # Skydome replaces the sky with its own shader material in _init_sky() (Skydome.gd:794).
    return Sky.new()


func create_nodes(world_environment: WorldEnvironment, _environment: Environment) -> void:
    # Skydome switches this single light between sun and moon by itself.
    sun_light = DirectionalLight3D.new()
    sun_light.name = SUN_LIGHT_NAME
    sun_light.shadow_reverse_cull_face = ProjectSettings.get_setting("maszyna/rendering/lights_shadow_reverse_cull_face", true)
    world_environment.add_child(sun_light, false, Node.INTERNAL_MODE_BACK)

    skydome = Skydome.new()
    skydome.name = SKYDOME_NAME
    skydome.world_environment_path = NodePath("..")
    skydome.directional_light_path = NodePath("../%s" % SUN_LIGHT_NAME)
    skydome.fog_mode = Skydome.FogModeOverride.DEPTH
    # Sky look comes from the gnd_skydome/* project settings (SkydomeSettings).
    skydome.apply_project_settings = true
    world_environment.add_child(skydome, false, Node.INTERNAL_MODE_BACK)

    # Weather drives Skydome clouds, wind, fog density and lightning on its own.
    weather = WeatherNode.new()
    weather.name = WEATHER_NAME
    weather.skydome_path = NodePath("../%s" % SKYDOME_NAME)
    weather.world_environment_path = NodePath("..")
    # Rain look comes from the gnd_weather/* project settings (WeatherSettings).
    weather.apply_project_settings = true
    world_environment.add_child(weather, false, Node.INTERNAL_MODE_BACK)


func bind_nodes(world_environment: WorldEnvironment) -> void:
    sun_light = world_environment.get_node_or_null(NodePath(SUN_LIGHT_NAME)) as DirectionalLight3D
    skydome = world_environment.get_node_or_null(NodePath(SKYDOME_NAME)) as Skydome
    weather = world_environment.get_node_or_null(NodePath(WEATHER_NAME)) as WeatherNode


func apply_visual_configuration() -> void:
    if not skydome:
        return

    var precipitation: float = environment_node.precipitation
    weather.cloud_density = environment_node.cloudiness
    weather.precipitation_intensity = precipitation
    weather.cloud_overcast_intensity = precipitation
    weather.storm_intensity = clampf(inverse_lerp(STORM_RAIN_START, 1.0, precipitation), 0.0, 1.0)
    var rain_fog: float = clampf(inverse_lerp(RAIN_FOG_START, 1.0, precipitation), 0.0, 1.0)
    var fog_density: float = lerpf(environment_node.fog_density, maxf(environment_node.fog_density, float(
        ProjectSettings.get_setting(RAIN_FOG_DENSITY_SETTING, RAIN_FOG_DENSITY_DEFAULT))), rain_fog)
    var fog_distance: float = lerpf(environment_node.fog_distance, minf(environment_node.fog_distance, float(
        ProjectSettings.get_setting(RAIN_FOG_DISTANCE_SETTING, RAIN_FOG_DISTANCE_DEFAULT))), rain_fog)
    weather.global_wind_direction = Vector2.from_angle(environment_node.wind_direction)
    weather.global_wind_speed = lerpf(WIND_SPEED_MIN, WIND_SPEED_MAX, environment_node.wind_strength)
    weather.global_wind_strength = lerpf(
        WIND_STRENGTH_MIN, WIND_STRENGTH_MAX, environment_node.wind_strength
    )
    weather.global_wind_turbulence = float(
        ProjectSettings.get_setting(WIND_TURBULENCE_SETTING, 1.0)
    )
    skydome.rainbow_intensity = (
        RAINBOW_INTENSITY_MAX
        * smoothstep(RAINBOW_RAIN_START, RAINBOW_RAIN_END, precipitation)
        * (1.0 - smoothstep(RAINBOW_CLOUD_FADE_START, RAINBOW_CLOUD_FADE_END, environment_node.cloudiness))
    )

    # Skydome adds its storm fog boost on top of its own day/night fog density (Skydome.gd:1265)
    # and that sum is the opacity the depth fog reaches at fog_distance. Godot does not clamp it:
    # above 1.0 a fully fogged object comes out as opacity * fog colour - (opacity - 1) * its own
    # colour - brighter than the fogged sky and still carrying its own silhouette. So the day/night
    # density stays Skydome's own haze and the boost carries only the rest of the wanted opacity.
    var density_scale: float = fog_density / FOG_REFERENCE_DENSITY
    var base_density: float = 0.0
    for property: StringName in FOG_DENSITY_PROPERTIES:
        var density: float = float(SkydomeSettings.get_value(property))
        skydome.set(property, density)
        base_density = maxf(base_density, density)
    weather.storm_fog_intensity = clampf(fog_density - base_density, 0.0, 1.0)
    var range_scale: float = (
        fog_distance / float(SkydomeSettings.get_value(FOG_REFERENCE_DISTANCE_PROPERTY)))
    for property: StringName in FOG_RANGE_PROPERTIES:
        skydome.set(property, float(SkydomeSettings.get_value(property)) * range_scale)
    var fog_curve: float = maxf(FOG_CURVE_MIN, float(ProjectSettings.get_setting(
        FOG_CURVE_SETTING, FOG_CURVE_DEFAULT)))
    var sky_affect: float = clampf(pow(float(ProjectSettings.get_setting(
        FOG_SKY_HEIGHT_SETTING, FOG_SKY_HEIGHT_DEFAULT)) / fog_distance, fog_curve), 0.0, 1.0)
    # The sky takes fog_sky_affect of the fog colour whatever the density is, while geometry at
    # fog_distance takes the density itself - a sky fogged harder than the terrain in front of it
    # cuts every distant silhouette back out of the fog, so the height share carries the opacity.
    sky_affect = lerpf(sky_affect, 1.0, rain_fog) * clampf(fog_density, 0.0, 1.0)
    for property: StringName in FOG_SKY_AFFECT_PROPERTIES:
        skydome.set(property, sky_affect)
    skydome.fog_sky_affect_intensity = 0.0
    for property: StringName in FOG_VOLUMETRIC_SKY_AFFECT_PROPERTIES:
        skydome.set(property, 1.0)
    # closer than the reference distance the volumetric fog thickens in proportion; further away
    # it dies out faster (FOG_VOLUMETRIC_FAR_FALLOFF_SETTING). Skydome scales its own boost of that
    # fog by the plain proportion, so the boost gets the rest of the falloff.
    var distance_ratio: float = 1.0 / range_scale
    var volumetric_scale: float = distance_ratio
    if distance_ratio < 1.0:
        volumetric_scale = pow(distance_ratio, float(ProjectSettings.get_setting(
            FOG_VOLUMETRIC_FAR_FALLOFF_SETTING, FOG_VOLUMETRIC_FAR_FALLOFF_DEFAULT)))
    for property: StringName in FOG_VOLUMETRIC_DENSITY_PROPERTIES:
        skydome.set(property, float(SkydomeSettings.get_value(property)) * density_scale * volumetric_scale)
    skydome.vol_fog_density_boost = (
        float(SkydomeSettings.get_value(&"vol_fog_density_boost")) * volumetric_scale / distance_ratio)
    skydome.day_fog_distance = fog_distance * float(ProjectSettings.get_setting(
        FOG_DAY_DISTANCE_FACTOR_SETTING, FOG_DAY_DISTANCE_FACTOR_DEFAULT))
    skydome.night_fog_distance = fog_distance * float(ProjectSettings.get_setting(
        FOG_NIGHT_DISTANCE_FACTOR_SETTING, FOG_NIGHT_DISTANCE_FACTOR_DEFAULT))


## Skydome later overrides shadow opacity from cloud coverage.
func apply_light_configuration() -> void:
    _apply_directional_light_settings(sun_light)


func set_date(year: int, month: int, day: int) -> Vector3i:
    _normalize_date(year, month, day)
    _push_time(true)
    return get_date()


func apply_time_configuration() -> void:
    if environment_node.use_system_time:
        _read_system_time()
    else:
        _current_time = environment_node.current_time
        _normalize_date(environment_node.year, environment_node.month, environment_node.day)
    _push_time(true)


func get_date() -> Vector3i:
    return Vector3i(_year, _month, _day)


func get_current_time() -> float:
    return _current_time


func process(delta: float) -> void:
    if Engine.is_editor_hint():
        return

    _time_update_elapsed += delta
    if _time_update_elapsed < TIME_UPDATE_INTERVAL:
        return

    var elapsed: float = _time_update_elapsed
    _time_update_elapsed = 0.0

    if environment_node.use_system_time:
        _read_system_time()
    else:
        _current_time += elapsed * environment_node.simulation_speed / SECONDS_PER_HOUR
        if _current_time >= 24.0:
            _current_time -= 24.0
            _normalize_date(_year, _month, _day + 1)
    _push_time(false)


func _normalize_date(year: int, month: int, day: int) -> void:
    var normalized_year: int = year + floori((month - 1) / 12.0)
    var normalized_month: int = posmod(month - 1, 12) + 1
    var unix_time: int = Time.get_unix_time_from_datetime_dict(
        {"year": normalized_year, "month": normalized_month, "day": 1}
    ) + (day - 1) * SECONDS_PER_DAY
    var date: Dictionary = Time.get_date_dict_from_unix_time(unix_time)

    _year = date["year"]
    _month = date["month"]
    _day = date["day"]


func _read_system_time() -> void:
    var datetime: Dictionary = Time.get_datetime_dict_from_system()
    _current_time = (
        datetime["hour"] + datetime["minute"] / 60.0 + datetime["second"] / SECONDS_PER_HOUR
    )
    _normalize_date(datetime["year"], datetime["month"], datetime["day"])


# Configuration changes snap immediately; only running time uses Skydome's transition.
func _push_time(snap: bool) -> void:
    if not skydome:
        return

    var transition_duration: float = skydome.time_transition_duration
    if snap:
        skydome.time_transition_duration = 0.0

    var year_start: int = Time.get_unix_time_from_datetime_dict(
        {"year": _year, "month": 1, "day": 1}
    )
    var date_time: int = Time.get_unix_time_from_datetime_dict(
        {"year": _year, "month": _month, "day": _day}
    )
    var day_of_year: int = (date_time - year_start) / SECONDS_PER_DAY + 1

    skydome.latitude = environment_node.latitude
    skydome.day_of_year = mini(day_of_year, MAXIMUM_DAY_OF_YEAR)
    # Skydome treats time_of_day as local solar time (Skydome.gd:1052) and has no longitude
    # nor timezone, so the local clock time is converted here.
    skydome.time_of_day = wrapf(
        _current_time - environment_node.timezone_offset + environment_node.longitude / 15.0,
        0.0,
        24.0
    )
    skydome.time_transition_duration = transition_duration
