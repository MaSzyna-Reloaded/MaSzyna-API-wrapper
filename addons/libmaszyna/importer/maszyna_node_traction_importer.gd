@tool
extends Object


func import(p:MaszynaParser, context: MaszynaImporterContext) -> MaszynaTraction3D:
    var obj = MaszynaTraction3D.new()
    obj.power_supply_name = p.next_token()
    obj.nominal_voltage = float(p.next_token())
    obj.max_current = float(p.next_token())
    obj.resistivity = float(p.next_token())
    obj.material = p.next_token()
    obj.wire_thickness = float(p.next_token())
    obj.damage_flag = int(p.next_token())
    obj.curve1 = get_curve_from_tokens(p.get_tokens(6).map(func(x): return float(x)))
    obj.curve2 = get_curve_from_tokens(p.get_tokens(6).map(func(x): return float(x)))
    obj.min_height = float(p.next_token())
    obj.segment_length = float(p.next_token())
    obj.wires = int(p.next_token())
    if not obj.wires == MaszynaTraction3D.Wires.JEZDNY:
        obj.wire_offset = float(p.next_token())
    obj.visible = p.as_bool(p.next_token())
    var nt = p.next_token().to_lower()
    if nt == "parallel":
        obj.parallel = p.next_token()
        nt = p.next_token()
    if not nt == "endtraction":
        push_error("Invalid node::traction")
    return obj


func get_curve_from_tokens(points) -> MaszynaTrackCurve:
    var c = MaszynaTrackCurve.new()
    c.p1 = Vector3(points[0], points[1], points[2])
    c.p2 = Vector3(points[3], points[4], points[5])
    return c
