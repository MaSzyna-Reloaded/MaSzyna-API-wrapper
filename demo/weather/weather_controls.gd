extends VBoxContainer

## Weather and time controls driving MaszynaEnvironmentNode, content of the "Weather and Time" HUD window
## (ported from forest-test-scene ui/WeatherControlsCanvas.gd).

const WIND_DIRECTION_OPTIONS: Array[Dictionary] = [
    {"label": "N", "value": Vector2(0.0, -1.0)},
    {"label": "NE", "value": Vector2(0.70710677, -0.70710677)},
    {"label": "E", "value": Vector2(1.0, 0.0)},
    {"label": "SE", "value": Vector2(0.70710677, 0.70710677)},
    {"label": "S", "value": Vector2(0.0, 1.0)},
    {"label": "SW", "value": Vector2(-0.70710677, 0.70710677)},
    {"label": "W", "value": Vector2(-1.0, 0.0)},
    {"label": "NW", "value": Vector2(-0.70710677, -0.70710677)},
]

@export var environment_node_path: NodePath

var _environment_node: MaszynaEnvironmentNode
var _time_slider_dragging: bool = false

@onready var _wind_value_label: Label = $WeatherRow/WindGroup/Row/WindValueLabel
@onready var _rain_value_label: Label = $WeatherRow/RainGroup/Row/RainValueLabel
@onready var _cloud_value_label: Label = $WeatherRow/CloudGroup/Row/CloudValueLabel
@onready var _fog_density_value_label: Label = $WeatherRow/FogDensityGroup/Row/FogDensityValueLabel
@onready var _fog_range_value_label: Label = $WeatherRow/FogRangeGroup/Row/FogRangeValueLabel
@onready var _time_value_label: Label = $TimeRow/TimeGroup/Row/TimeValueLabel
@onready var _day_value_label: Label = $TimeRow/DayGroup/Row/DayValueLabel
@onready var _month_value_label: Label = $TimeRow/MonthGroup/Row/MonthValueLabel
@onready var _year_value_label: Label = $TimeRow/YearGroup/Row/YearValueLabel
@onready var _day_slider: HSlider = $TimeRow/DayGroup/Row/DaySlider
@onready var _month_slider: HSlider = $TimeRow/MonthGroup/Row/MonthSlider
@onready var _year_slider: HSlider = $TimeRow/YearGroup/Row/YearSlider
@onready var _time_scale_value_label: Label = $TimeRow/TimeScaleGroup/Row/TimeScaleValueLabel
@onready var _wind_strength_slider: HSlider = $WeatherRow/WindGroup/Row/WindSlider
@onready var _wind_direction_button: OptionButton = $WeatherRow/WindDirectionGroup/WindDirectionButton
@onready var _rain_slider: HSlider = $WeatherRow/RainGroup/Row/RainSlider
@onready var _cloud_slider: HSlider = $WeatherRow/CloudGroup/Row/CloudSlider
@onready var _fog_density_slider: HSlider = $WeatherRow/FogDensityGroup/Row/FogDensitySlider
@onready var _fog_range_slider: HSlider = $WeatherRow/FogRangeGroup/Row/FogRangeSlider
@onready var _time_slider: HSlider = $TimeRow/TimeGroup/Row/TimeSlider
@onready var _time_scale_slider: HSlider = $TimeRow/TimeScaleGroup/Row/TimeScaleSlider


func _ready() -> void:
    _environment_node = get_node(environment_node_path) as MaszynaEnvironmentNode
    for option: Dictionary in WIND_DIRECTION_OPTIONS:
        _wind_direction_button.add_item(option["label"])

    _wind_strength_slider.value_changed.connect(_on_wind_strength_changed)
    _wind_direction_button.item_selected.connect(_on_wind_direction_selected)
    _rain_slider.value_changed.connect(_on_rain_changed)
    _cloud_slider.value_changed.connect(_on_cloud_changed)
    _fog_density_slider.value_changed.connect(_on_fog_density_changed)
    _fog_range_slider.value_changed.connect(_on_fog_range_changed)
    _time_slider.value_changed.connect(_on_time_changed)
    _time_slider.drag_started.connect(_on_time_drag_started)
    _time_slider.drag_ended.connect(_on_time_drag_ended)
    _time_scale_slider.value_changed.connect(_on_time_scale_changed)
    _day_slider.value_changed.connect(_on_day_changed)
    _month_slider.value_changed.connect(_on_month_changed)
    _year_slider.value_changed.connect(_on_year_changed)


# Mirrors the environment node, so presets and changes made elsewhere show up in the controls.
func _process(_delta: float) -> void:
    if not is_visible_in_tree():
        return

    if not _time_slider_dragging:
        _time_slider.set_value_no_signal(_environment_node.current_time)
        _time_value_label.text = _format_time_label(_environment_node.current_time)
    _day_slider.set_value_no_signal(_environment_node.day)
    _day_value_label.text = str(_environment_node.day)
    _month_slider.set_value_no_signal(_environment_node.month)
    _month_value_label.text = str(_environment_node.month)
    _year_slider.set_value_no_signal(_environment_node.year)
    _year_value_label.text = str(_environment_node.year)
    _wind_strength_slider.set_value_no_signal(_environment_node.wind_strength)
    _wind_value_label.text = _format_percent(_environment_node.wind_strength)
    _wind_direction_button.select(_find_closest_direction_index(_environment_node.wind_direction))
    _rain_slider.set_value_no_signal(_environment_node.precipitation)
    _rain_value_label.text = _format_percent(_environment_node.precipitation)
    _cloud_slider.set_value_no_signal(_environment_node.cloudiness)
    _cloud_value_label.text = _format_percent(_environment_node.cloudiness)
    _fog_density_slider.set_value_no_signal(_environment_node.fog_density)
    _fog_density_value_label.text = _format_multiplier(_environment_node.fog_density)
    _fog_range_slider.set_value_no_signal(_environment_node.fog_range)
    _fog_range_value_label.text = _format_multiplier(_environment_node.fog_range)
    _time_scale_slider.set_value_no_signal(_environment_node.simulation_speed)
    _time_scale_value_label.text = "%dx" % _environment_node.simulation_speed


func _on_wind_strength_changed(value: float) -> void:
    _wind_value_label.text = _format_percent(value)
    _environment_node.wind_strength = value


func _on_wind_direction_selected(index: int) -> void:
    var direction: Vector2 = WIND_DIRECTION_OPTIONS[index]["value"]
    _environment_node.wind_direction = direction.angle()


func _on_rain_changed(value: float) -> void:
    _rain_value_label.text = _format_percent(value)
    _environment_node.precipitation = value


func _on_cloud_changed(value: float) -> void:
    _cloud_value_label.text = _format_percent(value)
    _environment_node.cloudiness = value


func _on_fog_density_changed(value: float) -> void:
    _fog_density_value_label.text = _format_multiplier(value)
    _environment_node.fog_density = value


func _on_fog_range_changed(value: float) -> void:
    _fog_range_value_label.text = _format_multiplier(value)
    _environment_node.fog_range = value


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


func _on_time_scale_changed(value: float) -> void:
    _time_scale_value_label.text = "%dx" % value
    _environment_node.simulation_speed = value


func _find_closest_direction_index(wind_direction: float) -> int:
    var direction: Vector2 = Vector2.from_angle(wind_direction)
    var best_index: int = 0
    var best_dot: float = -INF
    for index: int in range(WIND_DIRECTION_OPTIONS.size()):
        var candidate: Vector2 = WIND_DIRECTION_OPTIONS[index]["value"]
        var score: float = direction.dot(candidate)
        if score > best_dot:
            best_dot = score
            best_index = index
    return best_index


func _format_percent(value: float) -> String:
    return "%d%%" % int(round(value * 100.0))


func _format_multiplier(value: float) -> String:
    return "x%.2f" % value


func _format_time_label(value: float) -> String:
    var wrapped: float = wrapf(value, 0.0, 24.0)
    var hours: int = int(floor(wrapped))
    var minutes: int = int(round((wrapped - float(hours)) * 60.0))
    if minutes >= 60:
        hours = (hours + 1) % 24
        minutes = 0
    return "%02d:%02d" % [hours, minutes]

