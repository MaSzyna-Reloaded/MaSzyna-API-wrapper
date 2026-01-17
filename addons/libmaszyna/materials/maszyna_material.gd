#class_name MaszynaMaterial

func _parse_texture_path(path:String):
    var _parts = path.split(":")
    var texture = _parts[0]
    if "t" in _parts:
        transparency = StandardMaterial3D.TRANSPARENCY_ALPHA_SCISSOR
    return texture

var transparency:StandardMaterial3D.Transparency = StandardMaterial3D.TRANSPARENCY_DISABLED

var albedo_texture_path:String = "":
    set(x):
        albedo_texture_path = _parse_texture_path(x)
var normal_texute_path:String = ""

var winter_albedo_texture_path:String = "":
    set(x):
        winter_albedo_texture_path = _parse_texture_path(x)
var winter_normal_texute_path:String = ""

var autumn_albedo_texture_path:String = "":
    set(x):
        autumn_albedo_texture_path = _parse_texture_path(x)

var shader:String = ""
var shadow_rank: int


func apply_to_material(mat:StandardMaterial3D):
    mat.transparency = transparency

    match transparency:
        StandardMaterial3D.TRANSPARENCY_ALPHA_SCISSOR:
            mat.alpha_scissor_threshold = 0.1

    match shader:
        "shadowlessnormalmap":
            #mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
            mat.specular_mode = BaseMaterial3D.SPECULAR_DISABLED
            mat.disable_receive_shadows = true
        "sunlessnormalmap":
            #mat.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
            mat.specular_mode = BaseMaterial3D.SPECULAR_DISABLED
