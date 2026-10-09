@tool
extends Node
class_name MaszynaEnvironmentNode

## Group the player sets cabin_view on when switching between the cabin and the exterior view
const GROUP: StringName = &"maszyna_environment"
## Render layer of the cab interior - with maszyna/cabin/improve_shadows_quality the cab is lit by a
## sun of its own that reaches only this layer (MaszynaSkyEnvironment._create_cabin_light())
const CABIN_RENDER_LAYER: int = 1 << 18
## How often the time of day, the light level and the wind are pushed to E3DRenderingServer, which
## decides from the first two which scenery lights are lit (see _push_environment_state())
const LIGHT_STATE_UPDATE_INTERVAL: float = 1.0
## How often the running time is taken from SimulationServer's clock
const TIME_UPDATE_INTERVAL: float = 0.1
const SECONDS_PER_HOUR: float = 3600.0
const SECONDS_PER_DAY: int = 86400
const MONTHS_PER_YEAR: int = 12
## Sun altitude (degrees) between which get_light_level() ramps from night to full day. The
## original lights a scenery light set to "on when dark" below a light level of 0.325
## (AnimModel.cpp:598), which on this ramp falls at about 1.4 degrees below the horizon. Both ends
## have to stay clear of a winter noon - at 50 N the sun peaks at 16-19 degrees in January, so a
## day threshold anywhere near that would light the whole town at midday (see FINDINGS.md).
const LIGHT_LEVEL_NIGHT_ALTITUDE_SETTING: StringName = &"maszyna/lights/night_altitude"
const LIGHT_LEVEL_DAY_ALTITUDE_SETTING: StringName = &"maszyna/lights/day_altitude"
const LIGHT_LEVEL_NIGHT_ALTITUDE: float = -6.0
const LIGHT_LEVEL_DAY_ALTITUDE: float = 6.0
## Overcast dims the key light in the original by this much at full cover
## (simulationenvironment.cpp:163)
const LIGHT_LEVEL_OVERCAST_FACTOR: float = 0.65
## Surface pressure of the sun's refraction, millibars (sun.cpp:14)
const SUN_SURFACE_PRESSURE: float = 1013.0
## Wind speed the 0-1 wind_strength maps onto, m/s
const WIND_SPEED_MIN: float = 0.15
const WIND_SPEED_MAX: float = 3.0
## The original's Global.Overcast runs 0-1 for the cloud cover and on up to 2 for precipitation
## (simulationenvironment.cpp:64-71); MaszynaScenery turns its heaviest step into a precipitation
## of 0.4 (maszyna_scenery.gd PRECIPITATION_MEDIUM), which here is an overcast of 2 again
const OVERCAST_MAX: float = 2.0
const OVERCAST_FULL_PRECIPITATION: float = 0.4
## The original's fog range with the fog switched off - the longest a scenery may declare
## (simulationstateserializer.cpp:216)
const FOG_RANGE_MAX: float = 25000.0
const WEATHER_PRESETS: Dictionary = {
    MaszynaEnvironment.Weather.WEATHER_CLEAR: {
        "precipitation": 0.0, "cloudiness": 0.1, "fog_density": 0.075, "wind_strength": 0.2,
    },
    MaszynaEnvironment.Weather.WEATHER_CLOUDY: {
        "precipitation": 0.0, "cloudiness": 0.7, "fog_density": 0.15, "wind_strength": 0.4,
    },
    MaszynaEnvironment.Weather.WEATHER_RAIN: {
        "precipitation": 0.8, "cloudiness": 0.9, "fog_density": 0.3, "wind_strength": 0.6,
    },
    MaszynaEnvironment.Weather.WEATHER_SNOW: {
        "precipitation": 0.0, "cloudiness": 0.9, "fog_density": 0.3, "wind_strength": 0.5,
    },
}

## The environment applied a change of its configuration - a preset, a scenery's own declarations,
## the cabin view or a property written from anywhere. Whoever shows this state (a
## MaszynaSkyEnvironment) reacts to this instead of reading the node every frame; the running clock
## is deliberately not announced here (see _process()).
signal configuration_changed

@export_category("Time")
@export var use_system_time: bool = false:
    set(value):
        use_system_time = value
        _dirty_time = true

@export_range(0.0, 23.9998) var current_time: float = 8.0:
    set(value):
        if not value == current_time:
            current_time = value
            _dirty_time = true

@export_range(1, 31) var day: int = 3:
    set(value):
        if not value == day:
            day = value
            _dirty_time = true

@export_range(1, 12) var month: int = 5:
    set(value):
        if not value == month:
            month = value
            _dirty_time = true

@export_range(0, 9999) var year: int = 2026:
    set(value):
        if not value == year:
            year = value
            _dirty_time = true

@export_range(-12, 14, 1) var timezone_offset: int = 1:
    set(value):
        timezone_offset = value
        _dirty_time = true

## The fastest the simulation runs, as many times the wall clock
const MAX_SIMULATION_SPEED: float = 100.0

## How many times the wall clock the simulation runs - SimulationServer's; the node only sets it,
## from the scene and the inspector
@export_range(0.0, MAX_SIMULATION_SPEED) var simulation_speed: float = 1.0:
    set(value):
        SimulationServer.simulation_speed = clampf(value, 0.0, MAX_SIMULATION_SPEED)
    get:
        return SimulationServer.simulation_speed

@export_category("Location")
@export_range(-90.0, 90.0, 0.001, "suffix:°") var latitude: float = 50.271:
    set(value):
        if not value == latitude:
            latitude = value
            _dirty_time = true

@export_range(-180.0, 180.0, 0.001, "suffix:°") var longitude: float = 19.04:
    set(value):
        if not value == longitude:
            longitude = value
            _dirty_time = true

@export_category("Weather")
## Preset: changing it after the node is ready sets precipitation, cloudiness, fog density and
## wind strength (WEATHER_PRESETS); loading a scene keeps the saved values.
@export var weather: MaszynaEnvironment.Weather = MaszynaEnvironment.Weather.WEATHER_CLEAR:
    set(value):
        if not value == weather:
            weather = value
            _dirty_weather_preset = is_node_ready()
            _dirty_visuals = true

@export_range(0.0, 1.0, 0.01) var cloudiness: float = 0.5:
    set(value):
        cloudiness = value
        _dirty_visuals = true

## Compass bearing the wind blows towards, in degrees. A plain angle rather than a vector: the
## weather backends and the particle emitters only ever need a horizontal direction.
@export_range(0.0, 360.0, 0.1, "suffix:°") var wind_direction: float = 135.0:
    set(value):
        wind_direction = wrapf(value, 0.0, 360.0)
        _dirty_visuals = true

@export_range(0.0, 10.0, 0.01) var wind_strength: float = 0.3:
    set(value):
        wind_strength = value
        _dirty_visuals = true

@export_range(0.0, 1.0, 0.01) var precipitation: float = 0.0:
    set(value):
        precipitation = value
        _dirty_visuals = true

## Air temperature, published to SimulationServer
@export_range(-15.0, 45.0, 0.1, "suffix:°C") var temperature: float = 15.0

@export_group("Fog")
@export var fog_enabled: bool = true:
    set(value):
        fog_enabled = value
        _dirty_visuals = true

## How much of the view the fog covers at fog_distance: 0 - none, 1 - fully opaque. Whoever draws
## the sky may add its own share on top (day/night base fog, rain), so this is the scenery's part.
@export_range(0.0, 1.0, 0.001) var fog_density: float = 0.15:
    set(value):
        fog_density = value
        _dirty_visuals = true

## Distance the fog reaches fog_density at, growing linearly up to it. Whoever draws the sky may
## tell day from night (maszyna/weather/fog/day_distance_factor, night_distance_factor).
@export_range(10.0, 25000.0, 1.0, "suffix:m") var fog_distance: float = 470.0:
    set(value):
        fog_distance = value
        _dirty_visuals = true

var season: MaszynaEnvironment.Season = MaszynaEnvironment.Season.SEASON_SUMMER:
    set(value):
        if not value == season:
            season = value
            MaterialManager.season = season

var _dirty_time: bool = true
var _dirty_visuals: bool = true
var _dirty_weather_preset: bool = false
var _dirty_view: bool = false
var _light_state_elapsed: float = 0.0
var _time_update_elapsed: float = 0.0

## Cabin view (the player in a cab) - whoever draws the environment lets the cab light cast its
## shadows then (MaszynaSkyEnvironment)
var cabin_view: bool = false:
    set(value):
        if not value == cabin_view:
            cabin_view = value
            _dirty_view = true


func _ready() -> void:
    update()
    _process_dirty()


func _enter_tree() -> void:
    add_to_group(GROUP)
    # the time of day passes while the environment is there (SimulationServer's clock)
    if not Engine.is_editor_hint():
        SimulationServer.clock_hold()
    ProjectSettings.settings_changed.connect(_on_project_settings_changed)


func _exit_tree() -> void:
    if not Engine.is_editor_hint():
        SimulationServer.clock_release()
    ProjectSettings.settings_changed.disconnect(_on_project_settings_changed)


func _process(delta: float) -> void:
    var time_set: bool = _dirty_time
    _process_dirty()
    _time_update_elapsed += delta
    if not Engine.is_editor_hint() and _time_update_elapsed >= TIME_UPDATE_INTERVAL:
        _time_update_elapsed = 0.0
        if use_system_time:
            _read_system_time()
        else:
            # the time the simulation's clock ran to; past midnight it is the next day
            var now: float = SimulationServer.time_of_day
            if now < current_time:
                _set_normalized_date(year, month, day + 1)
            current_time = now
        season = _season_from_year_day(_get_year_day(day, month, year))
    _push_environment_state(delta, time_set)
    # Running time is only mirrored here; it must not be re-applied as a configuration change.
    _dirty_time = false


func update() -> void:
    _dirty_time = true
    _dirty_visuals = true


func set_date(next_year: int, next_month: int, next_day: int) -> void:
    _set_normalized_date(next_year, next_month, next_day)


## How bright the scene is, the equivalent of the original's Global.fLuminance
## (simulationenvironment.cpp:184): daylight from the sun's altitude, dimmed by the overcast. It is
## what decides whether a scenery light that is set to come on automatically is on: the original
## compares it against DefaultDarkThresholdLevel of 0.325 (AnimModel.cpp:598).
##
## The sun's refracted altitude is cSun::move() and cSun::refract() (sun.cpp:104-240), with the
## location in decimal degrees (the original reads its own minutes-as-fraction notation).
func get_light_level() -> float:
    var local_time: float = current_time
    # days from 2000-01-01, the original's integer arithmetic included (sun.cpp:122-128)
    var day_number: float = (
        367 * year - 7 * (year + (month + 9) / MONTHS_PER_YEAR) / 4 + 275 * month / 9 + day - 730530
        + local_time / 24.0)
    var universal_time: float = local_time - timezone_offset
    var perihelion_longitude: float = 282.9404 + 4.70935e-5 * day_number
    var eccentricity: float = 0.016709 - 1.151e-9 * day_number
    var mean_anomaly: float = fposmod(356.0470 + 0.9856002585 * day_number, 360.0)
    var obliquity: float = 23.4393 - 3.563e-7 * day_number
    var eccentric_anomaly: float = mean_anomaly + rad_to_deg(
        eccentricity * sin(deg_to_rad(mean_anomaly)) * (1.0 + eccentricity * cos(deg_to_rad(mean_anomaly))))
    var xv: float = cos(deg_to_rad(eccentric_anomaly)) - eccentricity
    var yv: float = sin(deg_to_rad(eccentric_anomaly)) * sqrt(1.0 - eccentricity * eccentricity)
    var ecliptic_longitude: float = fposmod(rad_to_deg(atan2(yv, xv)) + perihelion_longitude, 360.0)
    var declination: float = asin(sin(deg_to_rad(obliquity)) * sin(deg_to_rad(ecliptic_longitude)))
    var right_ascension: float = fposmod(rad_to_deg(atan2(
        cos(deg_to_rad(obliquity)) * sin(deg_to_rad(ecliptic_longitude)), cos(deg_to_rad(ecliptic_longitude)))), 360.0)
    var sidereal_time: float = fposmod(6.697375 + 0.0657098242 * day_number + universal_time, 24.0)
    var hour_angle: float = wrapf(fposmod(sidereal_time * 15.0 + longitude, 360.0) - right_ascension, -180.0, 180.0)
    var zenith_cosine: float = clampf(
        sin(declination) * sin(deg_to_rad(latitude))
        + cos(declination) * cos(deg_to_rad(latitude)) * cos(deg_to_rad(hour_angle)), -1.0, 1.0)
    var elevation: float = 90.0 - rad_to_deg(acos(zenith_cosine))
    var refraction: float = 0.0
    if elevation <= 85.0:
        var elevation_tangent: float = tan(deg_to_rad(elevation))
        if elevation >= 5.0:
            refraction = (58.1 / elevation_tangent - 0.07 / pow(elevation_tangent, 3)
                + 0.000086 / pow(elevation_tangent, 5))
        elif elevation >= -0.575:
            refraction = 1735.0 + elevation * (-518.2 + elevation * (
                103.4 + elevation * (-12.79 + elevation * 0.711)))
        else:
            refraction = -20.774 / elevation_tangent
        refraction *= (SUN_SURFACE_PRESSURE * 283.0) / (SUN_SURFACE_PRESSURE * (273.0 + temperature)) / SECONDS_PER_HOUR
    var daylight: float = smoothstep(
        float(ProjectSettings.get_setting(LIGHT_LEVEL_NIGHT_ALTITUDE_SETTING, LIGHT_LEVEL_NIGHT_ALTITUDE)),
        float(ProjectSettings.get_setting(LIGHT_LEVEL_DAY_ALTITUDE_SETTING, LIGHT_LEVEL_DAY_ALTITUDE)),
        elevation + refraction)
    return daylight * (1.0 - clampf(cloudiness, 0.0, 1.0) * LIGHT_LEVEL_OVERCAST_FACTOR)


## Unit vector the wind blows along - horizontal, from the compass bearing. The original keeps one
## wind for the whole simulation (simulationenvironment.cpp:255-268) and the smoke emitters drift
## with it.
func get_wind_direction() -> Vector3:
    var bearing: float = deg_to_rad(wind_direction)
    return Vector3(cos(bearing), 0.0, sin(bearing))


## Wind speed in metres per second, wind_strength mapped onto WIND_SPEED_MIN..WIND_SPEED_MAX
func get_wind_speed() -> float:
    return lerpf(WIND_SPEED_MIN, WIND_SPEED_MAX, wind_strength)


func _process_dirty() -> void:
    var applied: bool = false

    if _dirty_weather_preset:
        _dirty_weather_preset = false
        var preset: Dictionary = WEATHER_PRESETS[weather]
        precipitation = preset["precipitation"]
        cloudiness = preset["cloudiness"]
        fog_density = preset["fog_density"]
        wind_strength = preset["wind_strength"]
        applied = true

    if _dirty_visuals:
        _dirty_visuals = false
        # Any precipitation switches the materials to their "rain" variant. It was blocked for a
        # while as bad looking: the wet texture ("rain: { texture2: ... }") is a reflection map and
        # was bound as a normal map back then (FINDINGS.md, "texture2: is not always the normal map").
        MaterialManager.weather = (
            MaszynaEnvironment.Weather.WEATHER_RAIN if precipitation > 0.0 else weather
        )
        # rain_params of the "rain_windscreen" materials (opengl33renderer.cpp:752-754): the share of
        # active droplets and the time they take to return after a wiper pass
        RenderingServer.global_shader_parameter_set("maszyna_rain_intensity", precipitation)
        RenderingServer.global_shader_parameter_set("maszyna_wiper_regen_time", lerpf(15.0, 1.0, precipitation))
        # the free spotlights' points and glare (types/free_spotlight*.gdshader) follow the
        # original's Global.Overcast and m_fogrange (opengl33renderer.cpp:5009), which fog_distance
        # is a multiple of
        RenderingServer.global_shader_parameter_set(
            "maszyna_overcast", clampf(cloudiness + precipitation / OVERCAST_FULL_PRECIPITATION, 0.0, OVERCAST_MAX))
        RenderingServer.global_shader_parameter_set(
            "maszyna_fog_range",
            fog_distance / float(ProjectSettings.get_setting(
                MaszynaSkyEnvironment.FOG_SCENERY_DISTANCE_FACTOR_SETTING,
                MaszynaSkyEnvironment.FOG_SCENERY_DISTANCE_FACTOR_DEFAULT))
            if fog_enabled
            else FOG_RANGE_MAX)
        applied = true

    if _dirty_view:
        _dirty_view = false
        applied = true

    if _dirty_time:
        _dirty_time = false
        if use_system_time:
            _read_system_time()
        else:
            _set_normalized_date(year, month, day)
        season = _season_from_year_day(_get_year_day(day, month, year))
        # a time set, not run: the clock jumps to it
        SimulationServer.time_of_day = current_time
        applied = true

    if applied:
        configuration_changed.emit()


## The fog follows its project settings while the scenery runs
func _on_project_settings_changed() -> void:
    _dirty_visuals = true


## Scenery lights set to come on automatically are decided by E3DRenderingServer out of the time of
## day and the light level, and the particle emitters drift with the wind; SimulationServer carries
## the time, the light level and the temperature for everything else (a cab screen's clock, once
## a second, is exactly the resolution it shows). None of the three
## changes fast enough to be worth pushing every frame - a whole scenery is re-resolved on each
## push - so they go at a fixed interval, and at once when the time was set rather than merely
## running.
func _push_environment_state(delta: float, time_set: bool) -> void:
    _light_state_elapsed += delta
    if _light_state_elapsed < LIGHT_STATE_UPDATE_INTERVAL and not time_set:
        return
    _light_state_elapsed = 0.0
    var light_level: float = get_light_level()
    E3DRenderingServer.environment_set_time(current_time)
    E3DRenderingServer.environment_set_light_level(light_level)
    # Global.fLuminance of the free spotlights' glare (types/free_spotlight_glare.gdshader)
    RenderingServer.global_shader_parameter_set("maszyna_light_level", light_level)
    SimulationServer.light_level = light_level
    SimulationServer.air_temperature = temperature
    E3DRenderingServer.environment_set_wind(get_wind_speed(), get_wind_direction())


## A date past the end of its month or year carries over (the 32nd of January is the 1st of February)
func _set_normalized_date(next_year: int, next_month: int, next_day: int) -> void:
    var normalized_year: int = next_year + floori((next_month - 1) / float(MONTHS_PER_YEAR))
    var normalized_month: int = posmod(next_month - 1, MONTHS_PER_YEAR) + 1
    var unix_time: int = Time.get_unix_time_from_datetime_dict(
        {"year": normalized_year, "month": normalized_month, "day": 1}
    ) + (next_day - 1) * SECONDS_PER_DAY
    var date: Dictionary = Time.get_date_dict_from_unix_time(unix_time)
    year = date["year"]
    month = date["month"]
    day = date["day"]


func _read_system_time() -> void:
    var datetime: Dictionary = Time.get_datetime_dict_from_system()
    current_time = datetime["hour"] + datetime["minute"] / 60.0 + datetime["second"] / SECONDS_PER_HOUR
    _set_normalized_date(datetime["year"], datetime["month"], datetime["day"])


func _season_from_year_day(year_day: int) -> MaszynaEnvironment.Season:
    # Thresholds are taken from the original simulator code.
    if year_day <= 65:
        return MaszynaEnvironment.Season.SEASON_WINTER
    if year_day <= 158:
        return MaszynaEnvironment.Season.SEASON_SPRING
    if year_day <= 252:
        return MaszynaEnvironment.Season.SEASON_SUMMER
    if year_day <= 341:
        return MaszynaEnvironment.Season.SEASON_AUTUMN
    return MaszynaEnvironment.Season.SEASON_WINTER


func _get_year_day(current_day: int, current_month: int, current_year: int) -> int:
    var month_lengths: Array[int] = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    var year_day: int = current_day
    var month_index: int = 0

    if _is_leap_year(current_year):
        month_lengths[1] = 29

    while month_index < current_month - 1:
        year_day += month_lengths[month_index]
        month_index += 1

    return year_day


func _is_leap_year(current_year: int) -> bool:
    return ((current_year % 4) == 0 and not (current_year % 100) == 0) or (current_year % 400) == 0
