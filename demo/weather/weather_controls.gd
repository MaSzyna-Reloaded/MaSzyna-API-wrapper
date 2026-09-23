extends VBoxContainer

## Weather and time controls driving MaszynaEnvironmentNode, content of the "Weather and Time" HUD window
## (ported from forest-test-scene ui/WeatherControlsCanvas.gd).

@export var environment_node_path: NodePath

## How often the running clock is read back. The environment announces every other change, so
## this is the only thing left that has to be looked at repeatedly - and a label showing hours and
## minutes gains nothing from being rewritten 60 times a second.
const CLOCK_REFRESH_INTERVAL: float = 0.1

var _environment_node: MaszynaEnvironmentNode
var _time_slider_dragging: bool = false
var _dirty: bool = true
var _refresh_timer: Timer

@onready var _wind_value_label: Label = $WeatherRow/WindGroup/Row/WindValueLabel
@onready var _rain_value_label: Label = $WeatherRow/RainGroup/Row/RainValueLabel
@onready var _cloud_value_label: Label = $WeatherRow/CloudGroup/Row/CloudValueLabel
@onready var _fog_density_value_label: Label = $WeatherRow/FogDensityGroup/Row/FogDensityValueLabel
@onready var _fog_distance_value_label: Label = $WeatherRow/FogDistanceGroup/Row/FogDistanceValueLabel
@onready var _time_value_label: Label = $TimeRow/TimeGroup/Row/TimeValueLabel
@onready var _day_value_label: Label = $TimeRow/DayGroup/Row/DayValueLabel
@onready var _month_value_label: Label = $TimeRow/MonthGroup/Row/MonthValueLabel
@onready var _year_value_label: Label = $TimeRow/YearGroup/Row/YearValueLabel
@onready var _day_slider: HSlider = $TimeRow/DayGroup/Row/DaySlider
@onready var _month_slider: HSlider = $TimeRow/MonthGroup/Row/MonthSlider
@onready var _year_slider: HSlider = $TimeRow/YearGroup/Row/YearSlider
@onready var _time_scale_value_label: Label = $TimeRow/TimeScaleGroup/Row/TimeScaleValueLabel
@onready var _wind_strength_slider: HSlider = $WeatherRow/WindGroup/Row/WindSlider
@onready var _wind_direction_slider: HSlider = $WeatherRow/WindDirectionGroup/Row/WindDirectionSlider
@onready var _wind_direction_value_label: Label = $WeatherRow/WindDirectionGroup/Row/WindDirectionValueLabel
@onready var _rain_slider: HSlider = $WeatherRow/RainGroup/Row/RainSlider
@onready var _cloud_slider: HSlider = $WeatherRow/CloudGroup/Row/CloudSlider
@onready var _fog_density_slider: HSlider = $WeatherRow/FogDensityGroup/Row/FogDensitySlider
@onready var _fog_distance_slider: HSlider = $WeatherRow/FogDistanceGroup/Row/FogDistanceSlider
@onready var _time_slider: HSlider = $TimeRow/TimeGroup/Row/TimeSlider
@onready var _time_scale_slider: HSlider = $TimeRow/TimeScaleGroup/Row/TimeScaleSlider
@onready var _system_time_check_box: CheckBox = $TimeRow/SystemTimeGroup/Row/SystemTimeCheckBox


func _ready() -> void:
    _environment_node = get_node(environment_node_path) as MaszynaEnvironmentNode
    _wind_strength_slider.value_changed.connect(_on_wind_strength_changed)
    _wind_direction_slider.value_changed.connect(_on_wind_direction_changed)
    _rain_slider.value_changed.connect(_on_rain_changed)
    _cloud_slider.value_changed.connect(_on_cloud_changed)
    _fog_density_slider.value_changed.connect(_on_fog_density_changed)
    _fog_distance_slider.value_changed.connect(_on_fog_distance_changed)
    _time_slider.value_changed.connect(_on_time_changed)
    _time_slider.drag_started.connect(_on_time_drag_started)
    _time_slider.drag_ended.connect(_on_time_drag_ended)
    _time_scale_slider.value_changed.connect(_on_time_scale_changed)
    _day_slider.value_changed.connect(_on_day_changed)
    _month_slider.value_changed.connect(_on_month_changed)
    _year_slider.value_changed.connect(_on_year_changed)
    _system_time_check_box.toggled.connect(_on_system_time_toggled)
    _environment_node.configuration_changed.connect(_on_environment_configuration_changed)
    _refresh_timer = Timer.new()
    _refresh_timer.wait_time = CLOCK_REFRESH_INTERVAL
    add_child(_refresh_timer)
    _refresh_timer.timeout.connect(_on_refresh_timeout)
    _refresh_timer.start()


func _exit_tree() -> void:
    _environment_node.configuration_changed.disconnect(_on_environment_configuration_changed)


## The environment applied a change, so everything the window shows is out of date.
func _on_environment_configuration_changed() -> void:
    _dirty = true


func _on_refresh_timeout() -> void:
    if not is_visible_in_tree():
        return
    if _dirty:
        _dirty = false
        _process_dirty()
        return
    _refresh_clock()


## Mirrors the whole environment node, so presets and changes made elsewhere show up in the
## controls.
func _process_dirty() -> void:
    # the system clock drives the time and the date, the sliders only show them
    var editable: bool = not _environment_node.use_system_time
    _system_time_check_box.set_pressed_no_signal(_environment_node.use_system_time)
    _time_slider.editable = editable
    _day_slider.editable = editable
    _month_slider.editable = editable
    _year_slider.editable = editable
    _refresh_clock()
    _day_slider.set_value_no_signal(_environment_node.day)
    _day_value_label.text = str(_environment_node.day)
    _month_slider.set_value_no_signal(_environment_node.month)
    _month_value_label.text = str(_environment_node.month)
    _year_slider.set_value_no_signal(_environment_node.year)
    _year_value_label.text = str(_environment_node.year)
    _wind_strength_slider.set_value_no_signal(_environment_node.wind_strength)
    _wind_value_label.text = _format_percent(_environment_node.wind_strength)
    _wind_direction_slider.set_value_no_signal(_environment_node.wind_direction)
    _wind_direction_value_label.text = _format_degrees(_environment_node.wind_direction)
    _rain_slider.set_value_no_signal(_environment_node.precipitation)
    _rain_value_label.text = _format_percent(_environment_node.precipitation)
    _cloud_slider.set_value_no_signal(_environment_node.cloudiness)
    _cloud_value_label.text = _format_percent(_environment_node.cloudiness)
    _fog_density_slider.set_value_no_signal(_environment_node.fog_density)
    _fog_density_value_label.text = _format_percent(_environment_node.fog_density)
    _fog_distance_slider.set_value_no_signal(_environment_node.fog_distance)
    _fog_distance_value_label.text = _format_meters(_environment_node.fog_distance)
    _time_scale_slider.set_value_no_signal(_environment_node.simulation_speed)
    _time_scale_value_label.text = "%dx" % _environment_node.simulation_speed


## The only part of the state that moves on its own, so the only part read on a timer.
func _refresh_clock() -> void:
    if _time_slider_dragging:
        return
    _time_slider.set_value_no_signal(_environment_node.current_time)
    _time_value_label.text = _format_time_label(_environment_node.current_time)


func _on_wind_strength_changed(value: float) -> void:
    _wind_value_label.text = _format_percent(value)
    _environment_node.wind_strength = value


func _on_wind_direction_changed(value: float) -> void:
    _wind_direction_value_label.text = _format_degrees(value)
    _environment_node.wind_direction = value


func _on_rain_changed(value: float) -> void:
    _rain_value_label.text = _format_percent(value)
    _environment_node.precipitation = value


func _on_cloud_changed(value: float) -> void:
    _cloud_value_label.text = _format_percent(value)
    _environment_node.cloudiness = value


func _on_fog_density_changed(value: float) -> void:
    _fog_density_value_label.text = _format_percent(value)
    _environment_node.fog_density = value


func _on_fog_distance_changed(value: float) -> void:
    _fog_distance_value_label.text = _format_meters(value)
    _environment_node.fog_distance = value


func _on_time_changed(value: float) -> void:
    _time_value_label.text = _format_time_label(value)
    _environment_node.current_time = value


# set_date() normalizes the date (e.g. 31.02 -> 03.03), the bar follows it in _process.
func _on_day_changed(value: float) -> void:
    _environment_node.set_date(_environment_node.year, _environment_node.month, int(value))


func _on_month_changed(value: float) -> void:
    _environment_node.set_date(_environment_node.year, int(value), _environment_node.day)


func _on_year_changed(value: float) -> void:
    _environment_node.set_date(int(value), _environment_node.month, _environment_node.day)


func _on_time_drag_started() -> void:
    _time_slider_dragging = true


func _on_time_drag_ended(_value_changed: bool) -> void:
    _time_slider_dragging = false


func _on_system_time_toggled(pressed: bool) -> void:
    _environment_node.use_system_time = pressed


func _on_time_scale_changed(value: float) -> void:
    _time_scale_value_label.text = "%dx" % value
    _environment_node.simulation_speed = value


func _format_percent(value: float) -> String:
    return "%d%%" % int(round(value * 100.0))


func _format_degrees(value: float) -> String:
    return "%d°" % roundi(value)


func _format_meters(value: float) -> String:
    return "%d m" % roundi(value)


func _format_time_label(value: float) -> String:
    var wrapped: float = wrapf(value, 0.0, 24.0)
    var hours: int = int(floor(wrapped))
    var minutes: int = int(round((wrapped - float(hours)) * 60.0))
    if minutes >= 60:
        hours = (hours + 1) % 24
        minutes = 0
    return "%02d:%02d" % [hours, minutes]

