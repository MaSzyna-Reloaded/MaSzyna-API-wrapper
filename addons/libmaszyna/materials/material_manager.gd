@tool
extends Node

var _unknown_material = preload("res://addons/libmaszyna/materials/unknown.material")
var _unknown_texture = preload("res://addons/libmaszyna/materials/missing_texture.png")

var _materials_cache = ResourceCache.create("materials")

enum Transparency { Disabled, Alpha, AlphaScissor }

const _transparency_codes = {
    Transparency.Disabled: "0",
    Transparency.Alpha: "a",
    Transparency.AlphaScissor: "s",
}

var use_alpha_transparency:bool = false

func clear_cache():
    _materials_cache.clear()
    use_alpha_transparency = UserSettings.get_setting("e3d", "use_alpha_transparency", false)

func load_material(model_path, material_name) -> MaszynaMaterial:
    return MaterialParser.parse(model_path, material_name)

func get_material(
    model_path:String,
    material_path:String,
    transparent:Transparency = Transparency.Disabled,
    is_sky:bool = false,
    diffuse_color: Color = Color(1.0, 1.0, 1.0)
) -> StandardMaterial3D:
    var _code = model_path.path_join(("%s_t%s_s%s_%s.res" % [
        material_path,
        _transparency_codes[transparent],
        "1" if is_sky else "0",
        "%x%x%x" % [diffuse_color.r8, diffuse_color.g8, diffuse_color.b8],
    ]))
    var output:StandardMaterial3D
    
    output = _materials_cache.get(_code) as StandardMaterial3D
    if output:
        return output
    
    var _m:StandardMaterial3D = StandardMaterial3D.new()

    var project_data_dir = UserSettings.get_maszyna_game_dir()
    var _mmat = load_material(model_path, material_path)

    if _mmat.albedo_texture_path:
        _m.albedo_texture = load_texture(model_path, _mmat.albedo_texture_path)
    else:
        # possibly "COLORED" material
        _m.albedo_color = diffuse_color

    if _mmat.normal_texute_path:
        _m.normal_texture = load_texture(model_path, _mmat.normal_texute_path, true)
        _m.normal_enabled = true
        _m.normal_scale = -5.0
    _mmat.apply_to_material(_m)

    _m.alpha_scissor_threshold = 0.5  # default

    # DETECT ALPHA FROM TEXTURE
    var texture_alpha:bool = false
    if _m.albedo_texture:
        var img:Image = _m.albedo_texture.get_image()
        if img:
            texture_alpha = not img.detect_alpha() == Image.ALPHA_NONE

    if texture_alpha:
        # FIXME: the legacy exe uses alpha channel mostly for rendering
        # windows, so ALPHA or ALPHA_DEPTH_PRE_PASS should be enabled
        # here. But both causes issues with rendering (priorirty and
        # other artifcats).
        #
        # The safest mode is AlsphaScissor, but windows aren't properly
        # rendered. Another approach is to use ALPHA_DEPTH_PRE_PASS
        # and adjust sorting depth on meshinstances.

        if use_alpha_transparency:
            transparent = Transparency.Alpha
        else:
            transparent = Transparency.AlphaScissor
            _m.alpha_scissor_threshold = 0.80  # Who knows...

    # End DETECT ALPHA FROM TEXTURE

    # force transparency and shading
    match transparent:
        Transparency.AlphaScissor:
            _m.transparency = StandardMaterial3D.TRANSPARENCY_ALPHA_SCISSOR
        Transparency.Alpha:
            _m.transparency = StandardMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS


    if is_sky:
        _m.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
        _m.albedo_color = Color(0.9, 0.9, 0.9, 0.8)
        _m.disable_receive_shadows = true
        _m.disable_ambient_light = true

    output = _m
    _materials_cache.set(_code, output)
    return _materials_cache.get(_code)  # Make sure a proper reference is returned


func get_texture(texture_path):
    return load_texture("", texture_path)


func load_texture(model_path, material_name, normal:bool=false) -> Texture:
    #var output:Texture
    
    var project_data_dir = UserSettings.get_maszyna_game_dir()

    var data_dir = project_data_dir if project_data_dir else ""
    if (project_data_dir.ends_with("\\")):
        project_data_dir = project_data_dir.trim_suffix("\\")
    if (project_data_dir.ends_with("/")):
        project_data_dir = project_data_dir.trim_suffix("/")

    var possible_paths = [
        model_path.path_join(material_name+".dds"),
        "textures".path_join(model_path.path_join(material_name+".dds")),
        material_name+".dds",
        "textures".path_join(material_name+".dds"),
    ]

    var final_path = ""
    for p in possible_paths:
        if FileAccess.file_exists(project_data_dir.path_join(p)):
            final_path = p
            break

    if not final_path:
        return _unknown_texture
    
    if FileAccess.file_exists(project_data_dir.path_join(final_path)):
        var texture:Texture2D = load(project_data_dir.path_join(final_path)) as Texture2D
        if texture:
            return texture
        else:
            return _unknown_texture
    else:
        return _unknown_texture
