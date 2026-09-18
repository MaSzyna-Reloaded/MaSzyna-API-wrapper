@tool
extends MaszynaSkyEnvironment
class_name GndSkydomeMaszynaEnvironment

const SKYDOME_NAME: StringName = &"Skydome"
const SUN_LIGHT_NAME: StringName = &"SunLight"
const WEATHER_NAME: StringName = &"Weather"
const WIND_SPEED_SETTING: StringName = &"maszyna/weather/wind_speed"
const WIND_STRENGTH_SETTING: StringName = &"maszyna/weather/wind_strength"
const WIND_TURBULENCE_SETTING: StringName = &"maszyna/weather/wind_turbulence"
# Rain specular range as in forest-test-scene levels/test_biomes.tscn.
const RAIN_SPECULAR_FADE_START: float = 0.1
const RAIN_SPECULAR_FADE_END: float = 3.0
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
    world_environment.add_child(sun_light, false, Node.INTERNAL_MODE_BACK)

    skydome = Skydome.new()
    skydome.name = SKYDOME_NAME
    skydome.world_environment_path = NodePath("..")
    skydome.directional_light_path = NodePath("../%s" % SUN_LIGHT_NAME)
    skydome.fog_mode = Skydome.FogModeOverride.DEPTH
    world_environment.add_child(skydome, false, Node.INTERNAL_MODE_BACK)

    # Weather drives Skydome clouds, wind, fog density and lightning on its own.
    weather = WeatherNode.new()
    weather.name = WEATHER_NAME
    weather.skydome_path = NodePath("../%s" % SKYDOME_NAME)
    weather.world_environment_path = NodePath("..")
    weather.rain_specular_fade_start = RAIN_SPECULAR_FADE_START
    weather.rain_specular_fade_end = RAIN_SPECULAR_FADE_END
    world_environment.add_child(weather, false, Node.INTERNAL_MODE_BACK)


func bind_nodes(world_environment: WorldEnvironment) -> void:
    sun_light = world_environment.get_node_or_null(NodePath(SUN_LIGHT_NAME)) as DirectionalLight3D
    skydome = world_environment.get_node_or_null(NodePath(SKYDOME_NAME)) as Skydome
    weather = world_environment.get_node_or_null(NodePath(WEATHER_NAME)) as WeatherNode


func apply_visual_configuration() -> void:
    if not skydome:
        return

    weather.cloud_density = environment_node.cloudiness
    weather.storm_fog_intensity = (
        environment_node.fog_density if environment_node.fog_enabled else 0.0
    )
    weather.precipitation_intensity = (
        1.0 if environment_node.weather == MaszynaEnvironment.Weather.WEATHER_RAIN else 0.0
    )
    weather.global_wind_direction = Vector2.from_angle(environment_node.wind_direction)
    weather.global_wind_speed = float(ProjectSettings.get_setting(WIND_SPEED_SETTING, 1.0))
    weather.global_wind_strength = float(ProjectSettings.get_setting(WIND_STRENGTH_SETTING, 4.0))
    weather.global_wind_turbulence = float(
        ProjectSettings.get_setting(WIND_TURBULENCE_SETTING, 1.0)
    )

    # Skydome owns the Environment fog; only its general sliders are driven from here.
    skydome.day_fog_distance_begin = environment_node.fog_range_start
    skydome.day_fog_distance = environment_node.fog_range_end
    skydome.night_fog_distance_begin = environment_node.fog_range_start
    skydome.night_fog_distance = environment_node.fog_range_end

    # Shadow opacity is left to Skydome, which derives it from cloud coverage.
    sun_light.shadow_enabled = environment_node.day_light_shadow_enabled
    sun_light.directional_shadow_mode = environment_node.day_light_shadow_mode
    sun_light.shadow_blur = environment_node.day_light_shadow_blur
    sun_light.shadow_bias = environment_node.day_light_shadow_bias
    sun_light.shadow_normal_bias = environment_node.day_light_shadow_normal_bias
    sun_light.directional_shadow_max_distance = environment_node.day_light_shadow_max_distance
    sun_light.light_volumetric_fog_energy = environment_node.day_light_volumetric_fog_energy


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
