extends Node

## Rain and thunder sounds driven by the WeatherNode of MaszynaEnvironmentNode's Skydome backend
## (ported from forest-test-scene levels/test_biomes.gd).

const RAIN_BUS_NAME: StringName = &"Rain"
const RAIN_LP_OPEN_CUTOFF_HZ: float = 20500.0
const RAIN_LP_OCCLUDED_CUTOFF_HZ: float = 3000.0
const RAIN_LP_TWEEN_DURATION: float = 0.12
const THUNDER_HEAVY_THRESHOLD_CALM: float = 0.9
const THUNDER_HEAVY_THRESHOLD_STORM: float = 0.72
const THUNDER_VOLUME_DB_MIN: float = -5.0
const THUNDER_VOLUME_DB_MAX: float = 1.5

## Internal WeatherNode created by MaszynaEnvironmentNode, e.g.
## "../MaszynaEnvironmentNode/_WorldEnvironment/Weather". Must be ready before this node.
@export var weather_path: NodePath

var _weather: WeatherNode
var _rain_bus_index: int
var _rain_low_pass_filter: AudioEffectLowPassFilter
var _rain_low_pass_tween: Tween

@onready var _rain_player: SfxPlayer = $RainSfxPlayer
@onready var _thunder_light_player: AudioStreamPlayer = $ThunderLight
@onready var _thunder_heavy_player: AudioStreamPlayer = $ThunderHeavy


func _ready() -> void:
    _rain_bus_index = AudioServer.get_bus_index(RAIN_BUS_NAME)
    _rain_low_pass_filter = AudioServer.get_bus_effect(_rain_bus_index, 0) as AudioEffectLowPassFilter
    _weather = get_node(weather_path) as WeatherNode
    _weather.thunder.connect(_on_weather_thunder)
    _weather.rain_strength_changed.connect(_on_weather_rain_strength_changed)
    _weather.rain_local_strength_changed.connect(_on_weather_rain_local_strength_changed)
    MaszynaRuntime.paused.connect(_on_runtime_paused)
    MaszynaRuntime.unpaused.connect(_on_runtime_unpaused)
    # The weather state was already applied while MaszynaEnvironmentNode got ready.
    _on_weather_rain_strength_changed(_weather.precipitation_intensity)


func _exit_tree() -> void:
    _weather.thunder.disconnect(_on_weather_thunder)
    _weather.rain_strength_changed.disconnect(_on_weather_rain_strength_changed)
    _weather.rain_local_strength_changed.disconnect(_on_weather_rain_local_strength_changed)
    MaszynaRuntime.paused.disconnect(_on_runtime_paused)
    MaszynaRuntime.unpaused.disconnect(_on_runtime_unpaused)


## The rain and the thunder go silent while the world is paused
func _on_runtime_paused() -> void:
    AudioServer.set_bus_mute(_rain_bus_index, true)


func _on_runtime_unpaused() -> void:
    AudioServer.set_bus_mute(_rain_bus_index, false)


func _on_weather_thunder(strength: float) -> void:
    var thunder_strength: float = clampf(strength, 0.0, 1.0)
    var storm_factor: float = clampf(_weather.get_storm_factor(), 0.0, 1.0)
    var heavy_threshold: float = lerpf(
        THUNDER_HEAVY_THRESHOLD_CALM, THUNDER_HEAVY_THRESHOLD_STORM, storm_factor
    )
    var player: AudioStreamPlayer = (
        _thunder_heavy_player if thunder_strength >= heavy_threshold else _thunder_light_player
    )
    player.volume_db = lerpf(THUNDER_VOLUME_DB_MIN, THUNDER_VOLUME_DB_MAX, thunder_strength)
    player.play()


func _on_weather_rain_strength_changed(strength: float) -> void:
    _rain_player.play_automation(&"rain", &"strength", strength)


## Rain heard from a shelter (e.g. a cab inside a vehicle's RainExclusion) is muffled.
func _on_weather_rain_local_strength_changed(strength: float) -> void:
    var global_strength: float = clampf(_weather.precipitation_intensity, 0.0, 1.0)
    var shelter_factor: float = 0.0
    if global_strength > 0.0001:
        shelter_factor = clampf((global_strength - strength) / global_strength, 0.0, 1.0)

    if _rain_low_pass_tween:
        _rain_low_pass_tween.kill()
    _rain_low_pass_tween = create_tween()
    _rain_low_pass_tween.tween_property(
        _rain_low_pass_filter,
        "cutoff_hz",
        lerpf(RAIN_LP_OPEN_CUTOFF_HZ, RAIN_LP_OCCLUDED_CUTOFF_HZ, shelter_factor),
        RAIN_LP_TWEEN_DURATION
    ).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN_OUT)
