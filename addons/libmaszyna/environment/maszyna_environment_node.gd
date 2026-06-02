@tool
extends Node
class_name MaszynaEnvironmentNode

@export_category("Current Time")
@export var use_system_time: bool = false:
    set(x):
        use_system_time = x
        update()

@export_range(0.0, 23.9998) var current_time: float = 8.0:
    set(x):
        if not x == current_time:
            current_time = x
            _dirty_lat_lng = true

@export_range(1, 31) var day: int = 3:
    set(x):
        if not x == day:
            day = x
            _dirty_lat_lng = true

@export_range(1, 12) var month: int = 5:
    set(x):
        if not x == month:
            month = x
            _dirty_lat_lng = true

@export_range(0, 9999) var year: int = 2026:
    set(x):
        if not x == year:
            year = x
            _dirty_lat_lng = true

@export var weather: MaszynaEnvironment.Weather = MaszynaEnvironment.Weather.WEATHER_CLEAR:
    set(x):
        if not x == weather:
            weather = x
            MaterialManager.weather = weather

@export_category("Location")
@export var latitude: float = 50.271:
    set(x):
        if not x == latitude:
            latitude = x
            _dirty_lat_lng = true

@export var longitude: float = 19.04:
    set(x):
        if not x == longitude:
            longitude = x
            _dirty_lat_lng = true

@export_category("Configuration")
@export var timezone_offset: int = 1:
    set(x):
        timezone_offset = x
        update()

@export_range(0.0, 1000.0) var simulation_speed: float = 1.0:
    set(x):
        simulation_speed = x
        update()

@export var time_in_editor: bool = true:
    set(x):
        time_in_editor = x
        update()

@export_node_path("TimeOfDay") var time_of_day_path: NodePath:
    set(x):
        time_of_day_path = x
        update()

var season: MaszynaEnvironment.Season = MaszynaEnvironment.Season.SEASON_SUMMER:
    set(x):
        if not x == season:
            season = x
            MaterialManager.season = season

var _time_of_day: TimeOfDay
var _dirty_lat_lng: bool = false


func _ready() -> void:
    update()


func set_date(next_year: int, next_month: int, next_day: int) -> void:
    if _time_of_day:
        # The order of date initialization matters!
        # TimeOfDay from Sky3D addon implements some cleanups in setters
        # but it should expose public interface to set full date instead.

        _time_of_day.year = next_year    # first, update the year
        _time_of_day.month = next_month  # second, update the month
        _time_of_day.day = next_day      # then update the day

        # because date could be cleaned (i.e. 31.04.2025 -> 1.05.2025)
        # we shold fix should store here fixed properties
        year = _time_of_day.year
        month = _time_of_day.month
        day = _time_of_day.day
    else:
        # FIXME: the date logic should be implemented in our code.
        # We should not rely on the addon!
        #
        # This will be probably addressed later in Simulation/Environment server

        year = next_year
        month = next_month
        day = next_day


func _process(delta: float) -> void:
    if _dirty_lat_lng:
        _dirty_lat_lng = false
        if _time_of_day:
            _time_of_day.latitude = latitude
            _time_of_day.longitude = longitude

            if use_system_time:
                _time_of_day.set_from_datetime_dict(Time.get_datetime_dict_from_system())
            else:
                set_date(year, month, day)
                _time_of_day.current_time = current_time

            day = _time_of_day.day
            month = _time_of_day.month
            year = _time_of_day.year
            current_time = _time_of_day.current_time
            season = _season_from_year_day(_get_year_day(day, month, year))

func update() -> void:
    _time_of_day = null

    if is_inside_tree():
        _time_of_day = get_node_or_null(time_of_day_path)
        if _time_of_day:
            _time_of_day.editor_time_enabled = time_in_editor
            _time_of_day.system_sync = use_system_time
            _time_of_day.minutes_per_day = 1440 / simulation_speed if simulation_speed > 0.0 else 0.0
            _time_of_day.update_interval = _time_of_day.minutes_per_day * 0.001
            _dirty_lat_lng = true

func _season_from_year_day(year_day: int) -> MaszynaEnvironment.Season:
    # thresholds are taken from the original simulator code
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
