@tool
extends Node

var _unknown_material = preload("res://addons/libmaszyna/materials/unknown.material")
var _unknown_texture = preload("res://addons/libmaszyna/materials/missing_texture.png")

var _textures = {}
var _materials = {}

enum Transparency { Disabled, Alpha, AlphaScissor }

const _transparency_codes = {
    Transparency.Disabled: "0",
    Transparency.Alpha: "a",
    Transparency.AlphaScissor: "s",
}

var use_alpha_transparency:bool = false

func clear_cache():
    _textures.clear()
    _materials.clear()
    use_alpha_transparency = UserSettings.get_setting("e3d", "use_alpha_transparency", false)

func _ready():
    UserSettings.config_changed.connect(clear_cache)
    UserSettings.cache_clear_requested.connect(clear_cache)

func load_material(model_path, material_name) -> MaszynaMaterial:
    return MaterialParser.parse(model_path, material_name)

func get_material(
    model_path:String,
    material_path:String,
    transparent:Transparency = Transparency.Disabled,
    is_sky:bool = false,
    diffuse_color: Color = Color(1.0, 1.0, 1.0)
) -> StandardMaterial3D:
    var _code = "%s/%s:t=%s:s=%s" % [
        model_path,
        material_path,
        _transparency_codes[transparent],
        "1" if is_sky else "0",
    ]

    if not _materials.has(_code):
        var _m = StandardMaterial3D.new()

        var project_data_dir = UserSettings.get_maszyna_game_dir()
        var _mmat = load_material(model_path, material_path)

        if _mmat.albedo_texture_path:
            _m.albedo_texture = load_texture(model_path, _mmat.albedo_texture_path)
        else:
            # possibly "COLORED" material
            _m.albedo_color = diffuse_color

        if _mmat.normal_texute_path:
            _m.normal_texture = load_texture(model_path, _mmat.normal_texute_path)
            var im:Image = _m.normal_texture.get_image()
            im.decompress()
            im.normal_map_to_xy()
            im.compress(Image.COMPRESS_S3TC, Image.COMPRESS_SOURCE_NORMAL)
            _m.normal_texture = ImageTexture.create_from_image(im)
            _m.normal_enabled = true
            _m.normal_scale = -5.0
        _mmat.apply_to_material(_m)

        _m.alpha_scissor_threshold = 0.5  # default

        # DETECT ALPHA FROM TEXTURE
        var texture_alpha:bool = false
        if _m.albedo_texture:
            var img:Image = _m.albedo_texture.get_image()
            texture_alpha = not img.detect_alpha() == Image.ALPHA_NONE

        if texture_alpha or  _m.albedo_texture.has_alpha():
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

        _materials[_code] = _m
    return _materials[_code]


func get_texture(texture_path):
    return load_texture("", texture_path)


func load_texture(model_path, material_name, global:bool=true) -> Texture:
    var project_data_dir = UserSettings.get_maszyna_game_dir()
    var data_dir = project_data_dir if project_data_dir else ""

    var im = Image.new()

    var possible_paths = [
        project_data_dir+"/"+model_path+"/"+material_name+".dds",
        project_data_dir+"/textures/"+model_path+"/"+material_name+".dds",
        project_data_dir+"/"+material_name+".dds",
        project_data_dir+"/"+"textures/"+material_name+".dds",
    ]

    var final_path = ""
    for p in possible_paths:
        if FileAccess.file_exists(p):
            final_path = p
            break

    if not final_path:
        return _unknown_texture

    if _textures.has(final_path):
        return _textures.get(final_path)

    var res = ResourceLoader.load(final_path)
    if res:
        return res
    else:
        push_error("Error loading texture \"" + final_path + "\" ")
        return _unknown_texture


func load_submodel_texture(model_path, material_name) -> ImageTexture:
    return load_texture(model_path, material_name, "/" in material_name)
