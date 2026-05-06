@tool
extends Object


func import(p:MaszynaParser, context: MaszynaImporterContext) -> MaszynaTrack3D:
    var obj := MaszynaTrack3D.new()

    var type = p.next_token()
    if type not in ["switch", "normal"]:
        return null
        
    obj.length = float(p.next_token())
    obj.rail_spacing = float(p.next_token())  # width
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
        obj.ballast_material = p.next_token().to_lower()  # material1
        var ballast_length_tiling = float(p.next_token())  # tex_length ??
        var material2 = p.next_token().to_lower()  # material2
        var tex_height = float(p.next_token())  # tex_height ???
        var ballast_width_tiling = float(p.next_token())  # tex_width  ???
        var tex_slope = float(p.next_token())
        #obj.ballast_offset += tex_height
        #obj.sleeper_height += tex_height
        obj.position.y = tex_height
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
        # parameters
        var params = {}
        var key = nt
        while true:
            nt = p.next_token()
            if nt == "endtrack":
                break
            if not key:
                key = nt
            else:
                params[key] = nt
                key = null
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
