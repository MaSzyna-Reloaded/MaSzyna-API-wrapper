@tool
extends Node

static var UNKNOWN_MATERIAL = preload("res://addons/libmaszyna/materials/unknown.material")
static var UNKNOWN_TEXTURE = preload("res://addons/libmaszyna/materials/missing_texture.png")
const DDSTextureLoader = preload("res://addons/libmaszyna/materials/dds_texture_loader.gd")
const COLORED_MATERIAL: Material = preload("res://addons/libmaszyna/e3d/colored.material")

## The cache key cannot see changes to MaterialFactory's own code - bump this whenever that code
## changes what a built material holds. v2: normal_scale 1.0 like the original.
const CACHE_VERSION: int = 3

var _materials_cache = ResourceCache.create("materials")
var _managed_materials: Dictionary = {}
var _dds_cache: Dictionary = {}

enum Transparency { Disabled, Alpha, AlphaScissor }


class MaterialOptions:
    var diffuse_color: Color = Color.WHITE
    var selfillum_color: Color = Color.WHITE
    var selfillum_energy: float = 1.0
    var selfillum_enabled: bool = false
    # E3D translucent submodels are rendered in a separate alpha-blended pass.
    var force_transparent: bool = false
    var alpha_scissor_threshold: float = 0.5


@export var season := MaszynaEnvironment.Season.SEASON_SUMMER:
    set(x):
        if not x == season:
            season = x
            _refresh_managed_materials()

@export var weather := MaszynaEnvironment.Weather.WEATHER_CLEAR:
    set(x):
        if not x == weather:
            weather = x
            _refresh_managed_materials()


func _ready() -> void:
    E3DRenderingServer.set_material_resolver(get_submodel_material)


func clear_cache() -> void:
    _materials_cache.clear()
    _dds_cache.clear()
    _refresh_managed_materials()

func load_material(model_path:String, material_name:String) -> MaszynaMaterial:
    return MaterialParser.parse(model_path, material_name)

func get_material(
    model_path:String,
    material_path:String,
    options: MaterialOptions = MaterialOptions.new(),
) -> Material:
    var cache_hash: String = _compute_cache_hash(model_path, material_path, options)
    var managed_material: Dictionary = _managed_materials.get(cache_hash, {})
    if managed_material:
        var material_ref: WeakRef = managed_material.get("material_ref") as WeakRef
        var material: ShaderMaterial = material_ref.get_ref() as ShaderMaterial if material_ref else null
        if material:
            return material
        _managed_materials.erase(cache_hash)
    var force_transparent = options.force_transparent  # TODO: ALPHA
    var output: ShaderMaterial = _materials_cache.get(cache_hash) as ShaderMaterial
    var is_newly_created:bool = not output
    var mmat: MaszynaMaterial = load_material(model_path, material_path)
    if is_newly_created:
        output = MaterialFactory.create(mmat, model_path, season, weather, options)
    else:
        MaterialFactory.apply(output, mmat, model_path, season, weather, options)
    _managed_materials[cache_hash] = {
        "material_ref": weakref(output),
        # most materials declare no season or weather variant and never change with them
        "has_variants": mmat.variants.size() > 0,
        "model_path": model_path,
        "material_path": material_path,
        "options": options,
    }
    # _materials_cache.set() writes a resource file to disk (see ResourceCache::set() in
    # src/core/ResourceCache.cpp) - only actually needed the first time this hash is seen, not
    # on every lookup. A scenery with hundreds of track/model segments sharing the same handful
    # of materials was otherwise doing hundreds of redundant disk writes per load.
    if is_newly_created:
        _materials_cache.set(cache_hash, output)
    return output

## Material override of an E3D submodel - the material resolver of [E3DRenderingServer].
## The first segment of [param data_path] is dropped from the material search path
## (see maszyna_rail_vehicle_3d_instancer.gd's _build_structure()).
func get_submodel_material(
    submodel: E3DSubModel,
    data_path: String,
    skins: PackedStringArray,
    force_alpha: bool,
) -> Material:
    var unprefixed_model_path: String = "/".join(data_path.split("/").slice(1))
    var options: MaterialOptions = MaterialOptions.new()

    # TODO: handle more material options here (selfillum, etc)
    options.force_transparent = force_alpha
    options.diffuse_color = submodel.diffuse_color
    options.selfillum_color = (
        submodel.self_illumination
        if submodel.self_illumination and not submodel.self_illumination == Color.BLACK
        else Color.WHITE
    )
    options.selfillum_energy = options.selfillum_color.a  # legacy renderer
    options.selfillum_enabled = options.selfillum_energy > 0.0 and submodel.lights_on_threshold >= 1.0  # legacy renderer logic

    if submodel.dynamic_material:
        if skins.size() < submodel.dynamic_material_index + 1:
            push_warning(
                "Model %s has no skins set, but submodel requires material #%s"
                % [data_path, submodel.dynamic_material_index]
            )
            return null
        return get_material(unprefixed_model_path, skins[submodel.dynamic_material_index], options)

    if submodel.material_colored:
        return COLORED_MATERIAL

    if submodel.material_name:
        return get_material(unprefixed_model_path, submodel.material_name, options)

    return null


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

    var full_path:String = project_data_dir.path_join(final_path)
    var max_size:int = int(ProjectSettings.get_setting("maszyna/dds_maxtexturesize", 1024))
    var texture:Texture2D = _load_dds_clamped(full_path, max_size)
    if not texture:
        texture = load(full_path) as Texture2D
    if texture:
        return texture
    return UNKNOWN_TEXTURE


## Loads a .dds with its top mipmap levels discarded down to max_size - port of the original
## engine's iMaxTextureSize/maxtexturesize clamp (Texture.cpp), which this wrapper had no
## equivalent of. Cached by path+max_size since this bypasses Godot's own load() resource
## cache, and the same texture file is commonly referenced by several distinct materials
## (e.g. a shared normal map across dynamic skin slots).
func _load_dds_clamped(full_path:String, max_size:int) -> Texture2D:
    var cache_key:String = "%s:%d" % [full_path, max_size]
    if _dds_cache.has(cache_key):
        return _dds_cache[cache_key]
    var texture:Texture2D = DDSTextureLoader.load_texture(full_path, max_size)
    if texture:
        _dds_cache[cache_key] = texture
    return texture


func _compute_cache_hash(
    model_path: String,
    material_path: String,
    options: MaterialOptions,
) -> String:
    var options_hash = ":".join([
        CACHE_VERSION,
        options.force_transparent,
        options.diffuse_color.to_html(true),
        options.alpha_scissor_threshold,
        options.selfillum_enabled,
        options.selfillum_color.to_html(true),
        options.selfillum_energy,
    ].map(str)).md5_text()
    return model_path.path_join("%s_%s.res" % [material_path, options_hash])


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
    if not managed_material.get("has_variants", true):
        return
    var model_path: String = managed_material.get("model_path", "")
    var material_path: String = managed_material.get("material_path", "")
    var options:MaterialOptions = managed_material.get("options")
    var mmat: MaszynaMaterial = load_material(model_path, material_path)
    # not written back to the disk cache: its key knows neither the season nor the weather, a
    # loaded material gets its variant applied anyway (get_material()), and the write - a resource
    # with its textures embedded, for every material at once - is what froze the game
    MaterialFactory.apply(material, mmat, model_path, season, weather, options)
