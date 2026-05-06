@tool
extends Object


func import(p:MaszynaParser, context: MaszynaImporterContext) -> E3DModelInstance:
    var loc_x = p.next_token()
    var loc_y = p.next_token()
    var loc_z = p.next_token()
    var rot_y = p.next_token()
    var filename:String = p.next_token().to_lower()
    var data_path:String = filename.get_base_dir()

    var obj := E3DModelInstance.new()
    obj.model_filename = filename.get_file().get_basename()
    var data_path_array = data_path.split("/")
    if not data_path_array or not data_path_array[0] == "dynamic":
        data_path_array.insert(0, "models")
        
    obj.data_path = "/".join(data_path_array)
    obj.instancer = E3DModelInstance.Instancer.OPTIMIZED

    obj.position = Vector3(float(loc_x), float(loc_y), float(loc_z))
    obj.rotate_object_local(Vector3.UP, deg_to_rad(float(rot_y)))
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

                var _lights = []
                var _lightcolors = []
                var _lt = _lights
                var _notransition = false

                while true:
                    var x = p.next_token()
                    match x.to_lower():
                        "lightcolors":
                            _lt = _lightcolors
                        "notransition":
                            _notransition = true
                        "endmodel":
                            _endmodel = true
                            break
                        _:
                            _lt.append(x)
                if obj:
                    #push_warning("Node model lights aren't supported yet")
                    pass
                    # obj.lights = _lights
                    # obj.light_colors = _lightcolors
                    # obj.light_transition = not _notransition
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
