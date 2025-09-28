@tool
extends Node
class_name MaszynaEnvironmentNode

@export_node_path("WorldEnvironment") var world_environment:NodePath = NodePath("")

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
    if not is_node_ready():
        return

    var shader_texture
    if sky_texture:
        shader_texture = MaterialManager.new().get_texture(sky_texture)
    else:
        shader_texture = null
    RenderingServer.global_shader_parameter_set("sky_texture", shader_texture)


    var world:WorldEnvironment = get_node(world_environment)

    if world:
        var env:Environment = world.environment
        var sky:ShaderMaterial = env.sky.sky_material
        if sky:
            sky.set_shader_parameter("sky_offset", sky_texture_offset)
            sky.set_shader_parameter("exposure", sky_energy)
            sky.set_shader_parameter("sky_scale", sky_texture_scale)

func _ready():
    UserSettings.config_changed.connect(_update_sky_shader)
    _update_sky_shader()
