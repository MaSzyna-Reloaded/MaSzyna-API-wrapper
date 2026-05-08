@tool
extends RefCounted


func import(p:MaszynaParser, context: MaszynaImporterContext) -> MaszynaTrack3D:
    var obj := MaszynaTrack3D.new()

    var type = p.next_token()
    if type not in ["switch", "normal"]:
        return null
        
    obj.length = float(p.next_token())
    obj.width = float(p.next_token())
    obj.friction = float(p.next_token())
    var sound_distance = float(p.next_token())
    obj.quality_flag = int(p.next_token())
    obj.damage_flag = int(p.next_token())
    obj.environment = p.next_token()
    obj.visible = p.as_bool(p.next_token())
    obj.sleeper_model_name = "models/quark/podklad1"  # FIXME: hardcoded
    obj.sleeper_model_skin = "quark/podklad_betonowy1"  # FIXME: hardcoded
    obj.ballast_offset = -0.25  # FIXME: hardcoded
    obj.ballast_height = 0.4  # FIXME: hardcoded
    obj.sleeper_height = -0.25  # FIXME: hardcoded
    if obj.visible:
        # FIXME: this is probably wrongly mapped
        obj.ballast_enabled = true
        obj.material1 = p.next_token().to_lower()
        obj.tex_length = float(p.next_token())
        obj.material2 = p.next_token().to_lower()
        obj.tex_height = float(p.next_token())
        obj.tex_width = float(p.next_token())
        obj.tex_slope = float(p.next_token())
    var curve_tokens = p.get_tokens(15)
    obj.curve = get_curve_from_tokens(curve_tokens)

    var nt = p.next_token()
    if nt.is_valid_float():
        # second curve
        curve_tokens = [nt] + p.get_tokens(14)
        #obj.additional_curve = true
        #obj.curve2 = get_curve_from_tokens(curve_tokens)
        push_warning("Node track: curve2 is not supported yet")
        
    elif not nt == "endtrack":
        var params: Dictionary = {}
        var key: String = nt
        while key != "endtrack":
            var value: String = p.next_token()
            if value == "endtrack":
                break

            if key == "railprofile":
                obj.railprofile = value
            else:
                params[key] = value

            key = p.next_token()
        obj.parameters = params
    return obj


func get_curve_from_tokens(points) -> MaszynaTrackCurve:
    var c = MaszynaTrackCurve.new()
    c.p1 = Vector3(float(points[0]), float(points[1]), float(points[2]))
    c.c1 = Vector3(float(points[4]), float(points[5]), float(points[6]))
    c.c2 = Vector3(float(points[7]), float(points[8]), float(points[9]))
    c.p2 = Vector3(float(points[10]), float(points[11]), float(points[12]))
    c.roll1 = float(points[3])
    c.roll2 = float(points[13])
    c.radius = float(points[14])

    return c
