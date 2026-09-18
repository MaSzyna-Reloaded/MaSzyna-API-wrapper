extends CanvasLayer

## Weather and time bar driving MaszynaEnvironmentNode, toggled with toggle_weather_controls (F9)
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

@onready var _weather_panel: PanelContainer = $WeatherControlsRoot/WeatherPanel
@onready var _wind_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/WindGroup/Row/WindValueLabel
@onready var _rain_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/RainGroup/Row/RainValueLabel
@onready var _cloud_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/CloudGroup/Row/CloudValueLabel
@onready var _fog_density_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/FogDensityGroup/Row/FogDensityValueLabel
@onready var _fog_range_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/FogRangeGroup/Row/FogRangeValueLabel
@onready var _time_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/TimeGroup/Row/TimeValueLabel
@onready var _time_scale_value_label: Label = $WeatherControlsRoot/WeatherPanel/Row/TimeScaleGroup/Row/TimeScaleValueLabel
@onready var _wind_strength_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/WindGroup/Row/WindSlider
@onready var _wind_direction_button: OptionButton = $WeatherControlsRoot/WeatherPanel/Row/WindDirectionGroup/WindDirectionButton
@onready var _rain_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/RainGroup/Row/RainSlider
@onready var _cloud_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/CloudGroup/Row/CloudSlider
@onready var _fog_density_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/FogDensityGroup/Row/FogDensitySlider
@onready var _fog_range_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/FogRangeGroup/Row/FogRangeSlider
@onready var _time_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/TimeGroup/Row/TimeSlider
@onready var _time_scale_slider: HSlider = $WeatherControlsRoot/WeatherPanel/Row/TimeScaleGroup/Row/TimeScaleSlider


func _ready() -> void:
    _environment_node = get_node(environment_node_path) as MaszynaEnvironmentNode
    _weather_panel.add_theme_stylebox_override("panel", _make_panel_style())
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


func _unhandled_input(event: InputEvent) -> void:
    if event.is_action_pressed("toggle_weather_controls"):
        visible = not visible


# Mirrors the environment node, so presets and changes made elsewhere show up in the bar.
func _process(_delta: float) -> void:
    if not visible:
        return

    if not _time_slider_dragging:
        _time_slider.set_value_no_signal(_environment_node.current_time)
        _time_value_label.text = _format_time_label(_environment_node.current_time)
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


func _make_panel_style() -> StyleBoxFlat:
    var panel_style: StyleBoxFlat = StyleBoxFlat.new()
    panel_style.bg_color = Color(0.05, 0.055, 0.06, 0.74)
    panel_style.set_corner_radius_all(14)
    panel_style.set_border_width_all(1)
    panel_style.border_color = Color(0.7, 0.74, 0.68, 0.18)
    panel_style.content_margin_left = 12.0
    panel_style.content_margin_top = 8.0
    panel_style.content_margin_right = 12.0
    panel_style.content_margin_bottom = 8.0
    return panel_style
