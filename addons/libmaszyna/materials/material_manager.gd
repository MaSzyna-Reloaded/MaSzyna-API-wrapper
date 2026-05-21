@tool
extends Node

enum Weather {WEATHER_CLEAR, WEATHER_CLOUDY, WEATHER_RAIN, WEATHER_SNOW}
enum Season {SEASON_WINTER, SEASON_SPRING, SEASON_SUMMER, SEASON_AUTUMN}

static var UNKNOWN_MATERIAL = preload("res://addons/libmaszyna/materials/unknown.material")
static var UNKNOWN_TEXTURE = preload("res://addons/libmaszyna/materials/missing_texture.png")

var _materials_cache = ResourceCache.create("materials")
var _managed_materials: Dictionary = {}

enum Transparency { Disabled, Alpha, AlphaScissor }

const _transparency_codes = {
    Transparency.Disabled: "0",
    Transparency.Alpha: "a",
    Transparency.AlphaScissor: "s",
}

@export var season: Season = Season.SEASON_SUMMER:
    set(x):
        if not x == season:
            season = x
            _refresh_managed_materials()

@export var weather: Weather = Weather.WEATHER_CLEAR:
    set(x):
        if not x == weather:
            weather = x
            _refresh_managed_materials()


func clear_cache() -> void:
    _materials_cache.clear()
    _refresh_managed_materials()

func load_material(model_path:String, material_name:String) -> MaszynaMaterial:
    return MaterialParser.parse(model_path, material_name)

func get_material(
    model_path:String,
    material_path:String,
    transparent:Transparency = Transparency.Disabled,
    diffuse_color: Color = Color(1.0, 1.0, 1.0)
) -> Material:
    var cache_hash: String = _compute_cache_hash(model_path, material_path, transparent, diffuse_color)
    var managed_material: Dictionary = _managed_materials.get(cache_hash, {})
    if managed_material:
        var material_ref: WeakRef = managed_material.get("material_ref") as WeakRef
        var material: ShaderMaterial = material_ref.get_ref() as ShaderMaterial if material_ref else null
        if material:
            return material
        _managed_materials.erase(cache_hash)
    var force_transparent := not transparent == Transparency.Disabled  # TODO: ALPHA
    var output: ShaderMaterial = _materials_cache.get(cache_hash) as ShaderMaterial
    if not output:
        var mmat: MaszynaMaterial = load_material(model_path, material_path)
        output = MaterialFactory.create(mmat, model_path, season, weather, force_transparent, diffuse_color)
    else:
        var mmat: MaszynaMaterial = load_material(model_path, material_path)
        MaterialFactory.apply(output, mmat, model_path, season, weather, force_transparent, diffuse_color)
    _managed_materials[cache_hash] = {
        "material_ref": weakref(output),
        "model_path": model_path,
        "material_path": material_path,
        "force_transparent": force_transparent,
        "diffuse_color": diffuse_color,
    }
    _materials_cache.set(cache_hash, output)
    return output

func get_texture(texture_path:String) -> Texture:
    return load_texture("", texture_path)

func load_texture(model_path:String, material_name:String, normal:bool = false) -> Texture:
    var project_data_dir:String = UserSettings.get_maszyna_game_dir()
    if (project_data_dir.ends_with("\\")):
        project_data_dir = project_data_dir.trim_suffix("\\")
    if (project_data_dir.ends_with("/")):
        project_data_dir = project_data_dir.trim_suffix("/")

    var possible_paths:Array[String] = [
        model_path.path_join(material_name+".dds"),
        "textures".path_join(model_path.path_join(material_name+".dds")),
        material_name+".dds",
        "textures".path_join(material_name+".dds"),
    ]

    var final_path:String = ""
    for p:String in possible_paths:
        if FileAccess.file_exists(project_data_dir.path_join(p)):
            final_path = p
            break

    if not final_path:
        return UNKNOWN_TEXTURE

    if FileAccess.file_exists(project_data_dir.path_join(final_path)):
        var texture:Texture2D = load(project_data_dir.path_join(final_path)) as Texture2D
        if texture:
            return texture
        return UNKNOWN_TEXTURE
    return UNKNOWN_TEXTURE


func _compute_cache_hash(
    model_path: String,
    material_path: String,
    transparent: Transparency,
    diffuse_color: Color,
) -> String:
    return model_path.path_join(("%s_t%s_%s.res" % [
        material_path,
        _transparency_codes[transparent],
        "%x%x%x" % [diffuse_color.r8, diffuse_color.g8, diffuse_color.b8],
    ]))

func _refresh_managed_materials() -> void:
    var cache_hashes: Array = _managed_materials.keys()
    for cache_hash: String in cache_hashes:
        _refresh_managed_material(cache_hash)

func _refresh_managed_material(cache_hash: String) -> void:
    var managed_material: Dictionary = _managed_materials.get(cache_hash, {})
    if not managed_material:
        return
    var material_ref: WeakRef = managed_material.get("material_ref") as WeakRef
    var material: ShaderMaterial = material_ref.get_ref() as ShaderMaterial if material_ref else null
    if not material:
        _managed_materials.erase(cache_hash)
        return
    var model_path: String = managed_material.get("model_path", "")
    var material_path: String = managed_material.get("material_path", "")
    var force_transparent: Transparency = managed_material.get("force_transparent", false)
    var diffuse_color: Color = managed_material.get("diffuse_color", Color.WHITE)
    var mmat: MaszynaMaterial = load_material(model_path, material_path)
    mmat.transparent = mmat.transparent or force_transparent
    MaterialFactory.apply(material, mmat, model_path, season, weather, force_transparent, diffuse_color)
    _materials_cache.set(cache_hash, material)
