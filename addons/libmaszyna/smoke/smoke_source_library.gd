@tool
extends Node

## Reads the particle emitter templates the original keeps in the game's [code]data/[/code]
## directory and turns them into what [E3DRenderingServer] needs to render an emitter.
##
## An emitter is a model submodel named [code]smokesource_<template>[/code]; the whole submodel
## name is the file name of its template ([code]particle_manager::find()[/code],
## [code]particles.cpp:465[/code]). There are twelve of them in the data set, so they are memoised
## in memory and not cached on disk.


## The original's gfx.smoke.fidelity caps a source at 500 * fidelity particles (particles.cpp:128)
const MAX_PARTICLES_SETTING:String = "maszyna/rendering/smoke_max_particles"
const DEFAULT_MAX_PARTICLES:int = 500
## Multiplies how many particles an emitter spawns per second, over what its template asks for.
## Each one is made correspondingly fainter, so a denser plume is smoother rather than darker -
## the original's gfx.smoke.fidelity does the same (particles.cpp:73, :128, :165).
const DENSITY_SETTING:String = "maszyna/rendering/smoke_density"
const DEFAULT_DENSITY:float = 1.0
## The one smoke texture the original uses for every emitter (opengl33renderer.cpp:105)
const SMOKE_TEXTURE:String = "fx/smoke"

## Which sprite a particle is drawn with. ORIGINAL is the single round blob the original binds for
## every emitter; MODERN walks a flipbook over the particle's lifetime, so a puff wells up, breaks
## into wisps and dissolves on its own.
enum GeneratorMode {ORIGINAL, MODERN}

const GENERATOR_MODE_SETTING:String = "maszyna/rendering/smoke_generator_mode"
## Path of the flipbook MODERN uses. Empty in the addon - the slot is filled by the project that
## ships the asset (the demo sets it to res://vfx/smoke_atlas.png).
const ATLAS_SETTING:String = "maszyna/rendering/smoke_atlas"
## Columns and rows of that flipbook
const ATLAS_FRAMES_SETTING:String = "maszyna/rendering/smoke_atlas_frames"

## cParser stop characters of the template grammar (particles.cpp:24); the braces are tokens of
## their own and must not be listed here
const _STOP_CHARS:Array = [" ", "\t", "\n", "\r", ";", ",", "[", "]"]

## Block key -> property of [MaszynaSmokeSource], the shape the original's own variablemaps have
## (particles.cpp:24-33, particles.h:222-226)
const _INITIALIZER_FIELDS:Dictionary = {
    "min_inclination:": "inclination_min",
    "max_inclination:": "inclination_max",
    "min_velocity:": "velocity_min",
    "max_velocity:": "velocity_max",
    "min_size:": "size_min",
    "max_size:": "size_max",
    "min_opacity:": "opacity_min",
    "max_opacity:": "opacity_max",
}
const _SIZE_CHANGE_FIELDS:Dictionary = {
    "step:": "size_step",
    "min:": "size_limit_min",
    "max:": "size_limit_max",
}
const _OPACITY_CHANGE_FIELDS:Dictionary = {
    "step:": "opacity_step",
    "min:": "opacity_limit_min",
    "max:": "opacity_limit_max",
}

var _sources:Dictionary[String, MaszynaSmokeSource] = {}
var _render_data:Dictionary[String, Dictionary] = {}


## E3DRenderingServer builds the emitters of an instance through this
func _ready() -> void:
    E3DRenderingServer.set_smoke_source_resolver(build_render_data)


## Everything E3DRenderingServer needs for one emitter: process_material, mesh, amount, lifetime,
## spawn_rate and the local aabb of the plume. Empty for a template that cannot be used.
func build_render_data(template_name:String) -> Dictionary:
    if _render_data.has(template_name):
        return _render_data[template_name]

    var data:Dictionary = {}
    var source:MaszynaSmokeSource = get_source(template_name)
    if source:
        data = _build_render_data(source, template_name)
    _render_data[template_name] = data
    return data


func get_source(template_name:String) -> MaszynaSmokeSource:
    if _sources.has(template_name):
        return _sources[template_name]

    var path:String = UserSettings.get_maszyna_game_dir().path_join("data").path_join(template_name + ".txt")
    var source:MaszynaSmokeSource = parse_file(path)
    _sources[template_name] = source
    return source


func clear_cache() -> void:
    _sources.clear()
    _render_data.clear()


## Parses one template file. Returns null when it cannot be read.
static func parse_file(abs_path:String) -> MaszynaSmokeSource:
    var file:FileAccess = FileAccess.open(abs_path, FileAccess.READ)
    if not file:
        push_error("[SmokeSourceLibrary] Particle source template not found: %s" % abs_path)
        return null

    var parser:MaszynaParser = MaszynaParser.new()
    parser.initialize(file.get_buffer(file.get_length()), _STOP_CHARS)
    var tokens:Array[String] = []
    while not parser.eof_reached():
        var token:String = parser.next_token()
        if token:
            tokens.append(token)
    return _parse_tokens(tokens)


static func _parse_tokens(tokens:Array[String]) -> MaszynaSmokeSource:
    var source:MaszynaSmokeSource = MaszynaSmokeSource.new()
    var i:int = 0
    while i < tokens.size():
        var key:String = tokens[i].to_lower()
        i += 1
        match key:
            "spawn_rate:":
                if i < tokens.size():
                    source.spawn_rate = float(tokens[i])
                    i += 1
            "initializer:":
                i = _parse_block(tokens, i, source, _INITIALIZER_FIELDS, true)
            "size_change:":
                i = _parse_block(tokens, i, source, _SIZE_CHANGE_FIELDS, false)
            "opacity_change:":
                i = _parse_block(tokens, i, source, _OPACITY_CHANGE_FIELDS, false)
    return source


## Reads a `{ key: value ... }` block into the properties [param fields] maps the keys to.
## Returns the index just past the closing brace.
static func _parse_block(
        tokens:Array[String], start:int, source:MaszynaSmokeSource,
        fields:Dictionary, with_color:bool) -> int:
    var i:int = start
    if i < tokens.size() and tokens[i] == "{":
        i += 1

    while i < tokens.size() and not tokens[i] == "}":
        var key:String = tokens[i].to_lower()
        i += 1
        if with_color and key == "color:" and i + 2 < tokens.size():
            source.color = Color8(int(tokens[i]), int(tokens[i + 1]), int(tokens[i + 2]))
            i += 3
        elif fields.has(key) and i < tokens.size():
            source.set(fields[key], float(tokens[i]))
            i += 1
    return i + 1


func _build_render_data(source:MaszynaSmokeSource, template_name:String) -> Dictionary:
    var density:float = maxf(ProjectSettings.get_setting(DENSITY_SETTING, DEFAULT_DENSITY), 0.0)
    var lifetime:float = source.get_particle_lifetime()
    var amount:int = source.get_particle_amount(
        ProjectSettings.get_setting(MAX_PARTICLES_SETTING, DEFAULT_MAX_PARTICLES), density)
    if not lifetime or not amount:
        # the original divides the spawn rate by the fade step and lands on infinity here
        push_warning("[SmokeSourceLibrary] Template emits nothing: %s" % template_name)
        return {}

    var terminal_size:float = source.get_terminal_size()
    var reach:float = source.velocity_max * lifetime + terminal_size

    return {
        "process_material": _build_process_material(source, lifetime, density),
        "mesh": _build_mesh(),
        "amount": amount,
        "lifetime": lifetime,
        "spawn_rate": source.spawn_rate * density,
        "aabb": AABB(
            Vector3(-reach, -terminal_size, -reach),
            Vector3(reach * 2.0, reach + terminal_size * 2.0, reach * 2.0)),
    }


func _build_process_material(
        source:MaszynaSmokeSource, lifetime:float, density:float) -> ParticleProcessMaterial:
    var material:ParticleProcessMaterial = ParticleProcessMaterial.new()
    # the original launches every particle along the owner's up axis, within the inclination cone
    # (particles.cpp:58-76)
    material.direction = Vector3.UP
    material.spread = source.inclination_max
    material.initial_velocity_min = source.velocity_min
    material.initial_velocity_max = source.velocity_max
    material.angle_min = 0.0
    material.angle_max = 360.0
    material.scale_min = source.size_min
    material.scale_max = source.size_max
    material.scale_curve = _build_curve(1.0, source.get_terminal_size() / maxf(source.get_mean_size(), 0.001))
    material.color = source.color
    material.color_initial_ramp = _build_opacity_ramp(source, density)
    material.alpha_curve = _build_curve(1.0, 0.0)
    # the flipbook is walked exactly once over the particle's lifetime
    material.anim_speed_min = 1.0
    material.anim_speed_max = 1.0
    # Neither engine applies gravity. E3DRenderingServer overwrites this with the wind drift
    # (set_wind()); the vertical decay of particles.cpp:365-380 is still not ported (see TODO.md).
    material.gravity = Vector3.ZERO
    return material


func _build_mesh() -> QuadMesh:
    var material:StandardMaterial3D = StandardMaterial3D.new()
    material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    material.blend_mode = BaseMaterial3D.BLEND_MODE_MIX
    material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
    material.billboard_mode = BaseMaterial3D.BILLBOARD_PARTICLES
    material.billboard_keep_scale = true
    material.vertex_color_use_as_albedo = true
    material.cull_mode = BaseMaterial3D.CULL_DISABLED
    # the original draws the particles with depth writes off (openglrenderer.cpp:3315)
    material.depth_draw_mode = BaseMaterial3D.DEPTH_DRAW_DISABLED
    var atlas:Texture2D = get_atlas()
    if atlas:
        var frames:Vector2i = ProjectSettings.get_setting(ATLAS_FRAMES_SETTING, Vector2i(4, 4))
        material.albedo_texture = atlas
        material.particles_anim_h_frames = maxi(frames.x, 1)
        material.particles_anim_v_frames = maxi(frames.y, 1)
        # one pass over the flipbook per particle, not a loop
        material.particles_anim_loop = false
    else:
        material.albedo_texture = MaterialManager.load_texture("", SMOKE_TEXTURE)

    var mesh:QuadMesh = QuadMesh.new()
    mesh.size = Vector2.ONE
    mesh.material = material
    return mesh


## The flipbook of the MODERN generator mode, or null when the mode is ORIGINAL or the project
## filled in no atlas - in which case a particle is drawn with the original's single sprite.
func get_atlas() -> Texture2D:
    if not ProjectSettings.get_setting(GENERATOR_MODE_SETTING, GeneratorMode.ORIGINAL) == GeneratorMode.MODERN:
        return null
    var path:String = ProjectSettings.get_setting(ATLAS_SETTING, "")
    if not path:
        push_warning("[SmokeSourceLibrary] Modern smoke needs %s; falling back to the original sprite"
            % ATLAS_SETTING)
        return null
    return load(path) as Texture2D


## Random initial opacity per particle, the original's LocalRandom(opacity[min], opacity[max])
## (particles.cpp:73)
## Each particle is divided by the density, so twice as many of them add up to the same plume
## instead of twice the soot (particles.cpp:73)
func _build_opacity_ramp(source:MaszynaSmokeSource, density:float) -> GradientTexture1D:
    var scale:float = 1.0 / maxf(density, 0.001)
    var gradient:Gradient = Gradient.new()
    gradient.set_color(0, Color(1.0, 1.0, 1.0, source.opacity_min * scale))
    gradient.set_color(1, Color(1.0, 1.0, 1.0, source.opacity_max * scale))
    var texture:GradientTexture1D = GradientTexture1D.new()
    texture.gradient = gradient
    return texture


## Both the size growth and the opacity fade are linear in the original (particles.h:207-220)
func _build_curve(from:float, to:float) -> CurveTexture:
    var curve:Curve = Curve.new()
    curve.min_value = minf(from, to)
    curve.max_value = maxf(from, to)
    curve.add_point(Vector2(0.0, from), 0.0, 0.0, Curve.TANGENT_LINEAR, Curve.TANGENT_LINEAR)
    curve.add_point(Vector2(1.0, to), 0.0, 0.0, Curve.TANGENT_LINEAR, Curve.TANGENT_LINEAR)
    var texture:CurveTexture = CurveTexture.new()
    texture.curve = curve
    return texture
