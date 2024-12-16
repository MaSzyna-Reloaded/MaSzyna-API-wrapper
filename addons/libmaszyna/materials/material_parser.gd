@tool
extends Node

const STOP_KEY = [" ", ";", char(9), char(10), char(13)]
const STOP_VALUE = [" ", ";", char(9), char(10), char(13)]


func parse(model_path, material_file) -> MaszynaMaterial:
    var final_path = ""
    var project_data_dir = UserSettings.get_maszyna_game_dir()
    var material_name = material_file

    # FIXME: move this to MaterialManager
    var possible_paths = [
        project_data_dir+"/"+model_path+"/"+material_name+".mat",
        project_data_dir+"/textures/"+model_path+"/"+material_name+".mat",
        project_data_dir+"/"+material_name+".mat",
        project_data_dir+"/"+"textures/"+material_name+".mat",
    ]
    for p in possible_paths:
        if FileAccess.file_exists(p):
            final_path = p
            break

    var file = FileAccess.open(final_path, FileAccess.READ) as FileAccess
    var mat = MaszynaMaterial.new()

    if file:
        var p = MaszynaParser.new()
        p.initialize(file.get_buffer(file.get_length()))

        var data = {}
        var current = [data]
        var key = ""
        var value = ""

        while not p.eof_reached():
            var token = p.next_token(STOP_VALUE)

            if token.ends_with(":"):
                key = token.split(":")[0]
            else:
                match token:
                    "}":
                        current.pop_front()
                        key = ""
                    "{":
                        var sub = {}
                        current[0][key] = sub
                        key = ""
                        current.push_front(sub)
                    _:
                        if token:
                            if current[0].has(key):
                                if not current[0][key] is Array:
                                    current[0][key] = [current[0][key]]
                                current[0][key].push_back(token)
                            else:
                                current[0][key] = token

        mat.shader = data.get("shader", "")
        var _t1 = data.get("texture_diffuse", data.get("texture1", ""))
        var _t2 = data.get("texture_normalmap", data.get("texture2", ""))

        if _t1 is Array:
            push_warning("Material: " + final_path +": more than 1 texture parameters aren't supported!")
            _t1 = _t1[0]
        if _t2 is Array:
            push_warning("Material: " + final_path +": more than 1 texture parameters aren't supported!")
            _t2 = _t2[0]
        mat.albedo_texture_path = _t1
        mat.normal_texute_path = _t2
    else:
        # Material (.mat) file not found
        # Assuming there is only DDS texture
        mat.albedo_texture_path = material_file

    return mat
