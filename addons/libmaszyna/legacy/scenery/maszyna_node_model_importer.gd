@tool
extends RefCounted

## Where a model's file is looked for, in this order for each extension: its path as given, from
## the game directory ("models\linia053\peron_sandomierz.t3d", "dynamic\pkp\..."), then under
## models/ ("bud\dombale.t3d") - TModelsManager::find_on_disk(), MdlMngr.cpp:146-150
const MODEL_EXTENSIONS:Array[String] = ["e3d", "t3d"]
const MODELS_DIRECTORY:String = "models"


func import(p:MaszynaParser, context: MaszynaImporterContext) -> MaszynaModelData:
    var loc_x = p.next_token()
    var loc_y = p.next_token()
    var loc_z = p.next_token()
    var rot_y = p.next_token()
    var filename:String = p.next_token().replace("\\", "/")
    var data_path:String = filename.get_base_dir()

    var obj := MaszynaModelData.new()
    obj.model_filename = filename.get_file().get_basename()
    var game_dir:String = UserSettings.get_maszyna_game_dir()
    var lookups:Array[String] = []
    for extension:String in MODEL_EXTENSIONS:
        for directory:String in [data_path, MODELS_DIRECTORY.path_join(data_path)]:
            lookups.append(directory.path_join(obj.model_filename + "." + extension))
    var found:String = ""
    for lookup:String in lookups:
        var file:String = MaszynaDataPath.resolve(game_dir, lookup)
        if FileAccess.file_exists(game_dir.path_join(file)):
            found = file
            break
    # a model found nowhere keeps the models/ path, for the loader to report
    obj.data_path = (
        found.get_base_dir() if found else MaszynaDataPath.resolve(game_dir, MODELS_DIRECTORY.path_join(data_path))
    )

    obj.position = Vector3(float(loc_x), float(loc_y), float(loc_z))
    obj.rotation = Vector3(0.0, deg_to_rad(float(rot_y)), 0.0)
    var skins = p.next_token()
    if not skins.to_lower() == "none":
        obj.skins = skins.split("|")

    var _endmodel = false

    while not _endmodel:
        if p.eof_reached():
            break
        var nt = p.next_token().to_lower()

        match nt:
            "lights":
                # `lights <mode> ... [lightcolors <hex> ...] [notransition]`, one value per light
                # in Light_On00..07 order (TAnimModel::Load(), AnimModel.cpp:335-361). A mode is
                # ls_Off/ls_On/ls_Blink/ls_Dark/ls_Home plus an optional fraction; a colour is an
                # RGB hex literal, with -1 meaning "leave the model's own colour alone".
                var modes:PackedFloat32Array = []
                var colors:PackedColorArray = []
                var reading_colors:bool = false

                while true:
                    var x:String = p.next_token()
                    match x.to_lower():
                        "lightcolors":
                            reading_colors = true
                        "notransition":
                            pass
                        "endmodel":
                            _endmodel = true
                            break
                        _:
                            if reading_colors:
                                colors.append(_parse_light_color(x))
                            else:
                                modes.append(float(x))
                obj.lights = modes
                obj.light_colors = colors
            "angles":
                var _rot = p.get_tokens(3)
                if obj:
                    obj.rotation = Vector3(
                        deg_to_rad(float(_rot[0])),
                        deg_to_rad(float(_rot[1])),
                        deg_to_rad(float(_rot[2])),
                    )
            "endmodel":
                _endmodel = true
                break
            _:
                pass

    return obj


## A `lightcolors` entry is an RGB hex literal; -1 keeps the colour the model carries
## (AnimModel.cpp:356). That "no override" is passed on as a negative colour.
func _parse_light_color(token:String) -> Color:
    if token == "-1":
        return Color(-1.0, -1.0, -1.0)
    var value:int = token.hex_to_int()
    return Color8((value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff)
