@tool
extends Node

class TextureMap:
    var albedo: String
    var normalmap: String
    var detail_normalmap: String
    var dudvmap: String
    var specgloss: String

    func _init(albedo: String, normalmap: String, detail_normalmap: String = ""):
        self.albedo = albedo
        self.normalmap = normalmap
        self.detail_normalmap = detail_normalmap
        self.dudvmap = "dudvmap"
        self.specgloss = "specgloss"

class MaszynaShaderMeta:
    var base_material: Material
    var factory: Callable
    var texture_map: TextureMap

    func _init(factory: Callable, base_material: Material, texture_map: TextureMap = null) -> void:
        self.base_material = base_material
        self.factory = factory
        if texture_map:
            self.texture_map = texture_map
        else:
            self.texture_map = TextureMap.new("diffuse", "normalmap")

var DEFAULT_SHADER:MaszynaShaderMeta = MaszynaShaderMeta.new(
    _apply_default_material,
    preload("res://addons/libmaszyna/materials/types/default.tres"),
    TextureMap.new("diffuse", "normalmap"),
)

var MATERIAL_SHADER_FACTORIES: Dictionary[String, MaszynaShaderMeta] = {
    "shadowlessnormalmap": MaszynaShaderMeta.new(
        _apply_default_material,
        preload("./types/shadowlessnormalmap.tres"),
        TextureMap.new("diffuse", "normalmap"),
    ),
    "sunlessnormalmap": MaszynaShaderMeta.new(
        _apply_default_material,
        preload("./types/sunlessnormalmap.tres"),
        TextureMap.new("diffuse", "normalmap"),
    ),
    "normalmap": MaszynaShaderMeta.new(
        _apply_default_material,
        preload("./types/normalmap.tres"),
        TextureMap.new("diffuse", "normalmap"),
    ),
    "normalmap_specgloss": MaszynaShaderMeta.new(
        _apply_default_material,
        preload("./types/normalmap_specgloss.tres"),
        TextureMap.new("diffuse", "normalmap"),
    ),
    "parallax": MaszynaShaderMeta.new(
        _apply_parallax,
        preload("./types/parallax.tres"),
        TextureMap.new("diffuse", "normalmap"),
    ),
    "parallax_specgloss": MaszynaShaderMeta.new(
        _apply_parallax,
        preload("./types/parallax_specgloss.tres"),
        TextureMap.new("diffuse", "normalmap"),
    ),
    "water": MaszynaShaderMeta.new(
        _apply_water,
        preload("./types/water.tres"),
    ),
    "water_specgloss": MaszynaShaderMeta.new(
        _apply_water,
        preload("./types/water_specgloss.tres"),
    ),
}


func create(
    mmat: MaszynaMaterial,
    model_path: String = "",
    season: MaterialManager.Season = MaterialManager.Season.SEASON_SUMMER,
    weather: MaterialManager.Weather = MaterialManager.Weather.WEATHER_CLEAR,
    options: MaterialManager.MaterialOptions = MaterialManager.MaterialOptions.new(),
) -> Material:
    var variant: MaszynaMaterial.MaszynaMaterialVariant = mmat.get_variant(season, weather)
    var shader_meta:MaszynaShaderMeta = _get_shader_meta(variant.shader)
    var material: Material = shader_meta.base_material.duplicate(true)
    _apply(
        material,
        mmat,
        variant,
        shader_meta,
        shader_meta.texture_map,
        model_path,
        options,
    )
    return material


func apply(
    material: Material,
    mmat: MaszynaMaterial,
    model_path: String,
    season: MaterialManager.Season,
    weather: MaterialManager.Weather,
    options: MaterialManager.MaterialOptions = MaterialManager.MaterialOptions.new(),
) -> void:
    var variant: MaszynaMaterial.MaszynaMaterialVariant = mmat.get_variant(season, weather)
    var shader_meta:MaszynaShaderMeta = _get_shader_meta(variant.shader)
    _apply(
        material,
        mmat,
        variant,
        shader_meta,
        shader_meta.texture_map,
        model_path,
        options,
    )


func _get_shader_meta(shader_name: String) -> MaszynaShaderMeta:
    if not shader_name:
        return DEFAULT_SHADER
    if not shader_name in MATERIAL_SHADER_FACTORIES:
        push_warning("Shader is not supported: " + shader_name)
    return MATERIAL_SHADER_FACTORIES.get(shader_name, DEFAULT_SHADER)


func _apply(
    material: Material,
    mmat: MaszynaMaterial,
    variant: MaszynaMaterial.MaszynaMaterialVariant,
    shader_meta: MaszynaShaderMeta,
    texture_map: TextureMap,
    model_path: String,
    options: MaterialManager.MaterialOptions,
) -> void:

    if not material is ShaderMaterial or not shader_meta.base_material is ShaderMaterial:
        return
    var target_shader_material: ShaderMaterial = material as ShaderMaterial
    var source_shader_material: ShaderMaterial = shader_meta.base_material as ShaderMaterial
    for property: Dictionary in source_shader_material.get_property_list():
        var property_name: String = property.get("name", "")
        if property_name == "shader" or property_name == "render_priority" or property_name.begins_with("shader_parameter/"):
            target_shader_material.set(property_name, source_shader_material.get(property_name))

    shader_meta.factory.call(mmat, variant, material, texture_map, model_path, options)
    var transparency: MaterialManager.Transparency = MaterialManager.Transparency.Disabled
    if mmat.transparent or options.force_transparent:
        transparency = MaterialManager.Transparency.AlphaScissor
    target_shader_material.set_shader_parameter("transparency", transparency)
    target_shader_material.set_shader_parameter("alpha_scissor_threshold", 0.5)
    target_shader_material.set_shader_parameter("emission_enabled", options.selfillum_enabled)
    target_shader_material.set_shader_parameter("emission_color", options.selfillum_color if options.selfillum_color else Color(1.0, 1.0, 1.0, 1.0))
    target_shader_material.set_shader_parameter("emission_energy", options.selfillum_energy)


func _apply_default_material(
    mmat: MaszynaMaterial,
    variant: MaszynaMaterial.MaszynaMaterialVariant,
    material: ShaderMaterial,
    texture_map: TextureMap,
    model_path: String,
    options: MaterialManager.MaterialOptions,
) -> void:
    var diffuse_texture: String = variant.get_texture_path(texture_map.albedo)
    var normalmap_texture: String = variant.get_texture_path(texture_map.normalmap)

    if diffuse_texture:
        var albedo_texture: Texture = MaterialManager.load_texture(model_path, diffuse_texture)
        if albedo_texture is Texture2D and _texture_has_alpha(albedo_texture):
            mmat.transparent = true
        material.set_shader_parameter("texture_albedo", albedo_texture)
        if variant.has_parameter("diffuse"):
            material.set_shader_parameter("albedo", Color(
                variant.get_parameter("diffuse"),
                variant.get_parameter("diffuse"),
                variant.get_parameter("diffuse"),
                1.0
            ))
    else:
        material.set_shader_parameter("albedo", options.diffuse_color)

    if normalmap_texture:
        material.set_shader_parameter("texture_normal", MaterialManager.load_texture(model_path, normalmap_texture, true))
        material.set_shader_parameter("normal_scale", -5.0)

    if variant.has_parameter("specular"):
        material.set_shader_parameter("specular", variant.get_parameter("specular"))

    if variant.has_parameter("glossiness"):
        material.set_shader_parameter("roughness", _roughness_from_glossiness(variant.get_parameter("glossiness")))

    if variant.has_parameter("reflection"):
        material.set_shader_parameter("metallic", variant.get_parameter("reflection"))
    material.set_shader_parameter("emission_enabled", options.selfillum_enabled)
    material.set_shader_parameter("emission_color", options.selfillum_color if options.selfillum_color else Color(1.0, 1.0, 1.0, 1.0))
    material.set_shader_parameter("emission_energy", options.selfillum_energy)

func _apply_parallax(
    mmat: MaszynaMaterial,
    variant: MaszynaMaterial.MaszynaMaterialVariant,
    material: ShaderMaterial,
    texture_map: TextureMap,
    model_path: String,
    options: MaterialManager.MaterialOptions,
) -> void:
    var diffuse_texture_path: String = variant.get_texture_path(texture_map.albedo)
    var normalmap_texture_path: String = variant.get_texture_path(texture_map.normalmap)
    var detail_normalmap_texture_path: String = variant.get_texture_path(texture_map.detail_normalmap)

    var albedo_texture:Texture = MaterialManager.UNKNOWN_TEXTURE
    if diffuse_texture_path:
        albedo_texture = MaterialManager.load_texture(model_path, diffuse_texture_path)
        if albedo_texture is Texture2D and _texture_has_alpha(albedo_texture):
            mmat.transparent = true

    var normal_texture:Texture = MaterialManager.UNKNOWN_TEXTURE
    if normalmap_texture_path:
        normal_texture = MaterialManager.load_texture(model_path, normalmap_texture_path, true)

    var detail_normal_texture:Texture = MaterialManager.UNKNOWN_TEXTURE
    var use_detail_normal:bool = false
    if detail_normalmap_texture_path:
        detail_normal_texture = MaterialManager.load_texture(model_path, detail_normalmap_texture_path, true)
        use_detail_normal = true

    var albedo_multiplier:Color = Color(1.0, 1.0, 1.0, 1.0)
    if not diffuse_texture_path:
        albedo_multiplier = options.diffuse_color

    if variant.has_parameter("diffuse"):
        albedo_multiplier = Color(
            variant.get_parameter("diffuse"),
            variant.get_parameter("diffuse"),
            variant.get_parameter("diffuse"),
            1.0
        )

    var alpha_scissor_threshold:float = 0.5
    var use_alpha_scissor := false

    if mmat.transparent:
        alpha_scissor_threshold = 0.70
        use_alpha_scissor = true

    material.set_shader_parameter("diffuse_texture", albedo_texture)
    material.set_shader_parameter("normal_texture", normal_texture)
    material.set_shader_parameter("detail_normal_texture", detail_normal_texture)
    material.set_shader_parameter("use_detail_normal", use_detail_normal)
    material.set_shader_parameter("albedo_multiplier", albedo_multiplier)
    material.set_shader_parameter("specular_strength", variant.get_parameter("specular", 0.5))
    material.set_shader_parameter("reflection_strength", variant.get_parameter("reflection", 0.0))
    material.set_shader_parameter("glossiness", variant.get_parameter("glossiness", 10.0))
    material.set_shader_parameter("detail_scale", variant.get_parameter("detail_scale", 1.0))
    material.set_shader_parameter("detail_height_scale", variant.get_parameter("detail_height_scale", 1.0))
    material.set_shader_parameter("height_scale", variant.get_parameter("height_scale", 0.0))
    material.set_shader_parameter("height_offset", variant.get_parameter("height_offset", 0.0))
    material.set_shader_parameter("texture_size", mmat.size)
    material.set_shader_parameter("use_alpha_scissor", use_alpha_scissor)
    material.set_shader_parameter("alpha_scissor_threshold", alpha_scissor_threshold)

func _apply_water(
    mmat: MaszynaMaterial,
    variant: MaszynaMaterial.MaszynaMaterialVariant,
    material: ShaderMaterial,
    texture_map: TextureMap,
    model_path: String,
    options: MaterialManager.MaterialOptions,
) -> void:
    var diffuse_texture_path: String = variant.get_texture_path(texture_map.albedo)
    var normalmap_texture_path: String = variant.get_texture_path(texture_map.normalmap)
    var dudvmap_texture_path: String = variant.get_texture_path(texture_map.dudvmap)
    var specgloss_texture_path: String = variant.get_texture_path(texture_map.specgloss)

    var diffuse_texture: Texture = MaterialManager.UNKNOWN_TEXTURE
    if diffuse_texture_path:
        diffuse_texture = MaterialManager.load_texture(model_path, diffuse_texture_path)
        if diffuse_texture is Texture2D and _texture_has_alpha(diffuse_texture):
            mmat.transparent = true

    var normal_texture: Texture = MaterialManager.UNKNOWN_TEXTURE
    if normalmap_texture_path:
        normal_texture = MaterialManager.load_texture(model_path, normalmap_texture_path, true)

    var dudv_texture: Texture = MaterialManager.UNKNOWN_TEXTURE
    if dudvmap_texture_path:
        dudv_texture = MaterialManager.load_texture(model_path, dudvmap_texture_path)

    var material_color := Vector4(1.0, 1.0, 1.0, 0.0)
    if variant.has_parameter_vec4("color"):
        material_color = variant.get_parameter_vec4("color")

    material.set_shader_parameter("diffuse_texture", diffuse_texture)
    material.set_shader_parameter("normal_texture", normal_texture)
    material.set_shader_parameter("dudv_texture", dudv_texture)
    material.set_shader_parameter("material_color", Color(
        material_color.x,
        material_color.y,
        material_color.z,
        material_color.w
    ))
    material.set_shader_parameter("reflection_strength", variant.get_parameter("reflection", 0.0))
    material.set_shader_parameter("glossiness", variant.get_parameter("glossiness", 10.0))
    material.set_shader_parameter("wave_strength", variant.get_parameter("wave_strength", 0.05))
    material.set_shader_parameter("wave_speed", variant.get_parameter("wave_speed", 0.02))

    if variant.shader == "water_specgloss":
        var specgloss_texture: Texture = MaterialManager.UNKNOWN_TEXTURE
        if specgloss_texture_path:
            specgloss_texture = MaterialManager.load_texture(model_path, specgloss_texture_path, true)
        material.set_shader_parameter("specgloss_texture", specgloss_texture)


func _roughness_from_glossiness(glossiness: float) -> float:
    if glossiness == NAN:
        return 1.0

    var g := abs(glossiness)

    if g < 0.0001:
        return 1.0

    var max_glossiness := 3500.0
    var min_roughness := 0.18
    var max_roughness := 0.75

    var t := log(g + 1.0) / log(max_glossiness + 1.0)

    return lerp(max_roughness, min_roughness, clamp(t, 0.0, 1.0))

func _texture_has_alpha(texture:Texture2D) -> bool:
    var img:Image = texture.get_image()
    if not img:
        return false
    return not img.detect_alpha() == Image.ALPHA_NONE


func _apply_alpha_override_from_texture(
    material:BaseMaterial3D,
) -> MaterialManager.Transparency:
    var texture_alpha:bool = false
    if material.albedo_texture:
        texture_alpha = _texture_has_alpha(material.albedo_texture)

    if not texture_alpha:
        return MaterialManager.Transparency.Disabled

    if MaterialManager.use_alpha_transparency:
        return MaterialManager.Transparency.Alpha
    return MaterialManager.Transparency.AlphaScissor


func _apply_base_material_transparency(material:BaseMaterial3D, transparent:MaterialManager.Transparency) -> void:
    match transparent:
        MaterialManager.Transparency.AlphaScissor:
            material.transparency = StandardMaterial3D.TRANSPARENCY_ALPHA_SCISSOR
        MaterialManager.Transparency.Alpha:
            material.transparency = StandardMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS
