@tool
extends Node
class_name MaszynaEnvironmentNode

@export_node_path("WorldEnvironment") var world_environment:NodePath = NodePath("")
@export var season: MaterialManager.Season = MaterialManager.Season.SEASON_SUMMER:
    set(x):
        if not x == season:
            season = x
            MaterialManager.season = season

@export var weather: MaterialManager.Weather = MaterialManager.Weather.WEATHER_CLEAR:
    set(x):
        if not x == weather:
            weather = x
            MaterialManager.weather = weather

@export var sky_texture = "sky/sky_altostratus015":
    set(x):
        if not x == sky_texture:
            sky_texture = x
            _update_sky_shader()

@export var sky_texture_offset = Vector2(0.0, 0.0):
    set(x):
        sky_texture_offset = x
        _update_sky_shader()

@export var sky_texture_scale = Vector2(1.0, 1.0):
    set(x):
        sky_texture_scale = x
        _update_sky_shader()

@export var sky_energy: float = 0.75:
    set(x):
        sky_energy = x
        _update_sky_shader()


func _update_sky_shader():
    # workaround for broken Godot nodes/plugin exiting order
    if not is_node_ready() or not is_inside_tree():
        return

    var shader_texture
    if sky_texture:
        shader_texture = MaterialManager.get_texture(sky_texture)
    else:
        shader_texture = null
    RenderingServer.global_shader_parameter_set("sky_texture", shader_texture)


    var world: WorldEnvironment = get_node_or_null(world_environment)

    if world:
        var env:Environment = world.environment
        var sky:ShaderMaterial = env.sky.sky_material
        if sky:
            sky.set_shader_parameter("sky_offset", sky_texture_offset)
            sky.set_shader_parameter("exposure", sky_energy)
            sky.set_shader_parameter("sky_scale", sky_texture_scale)


func _ready():
    _update_sky_shader()


func _enter_tree() -> void:
    UserSettings.config_changed.connect(_update_sky_shader)

func _exit_tree() -> void:
    UserSettings.config_changed.disconnect(_update_sky_shader)
