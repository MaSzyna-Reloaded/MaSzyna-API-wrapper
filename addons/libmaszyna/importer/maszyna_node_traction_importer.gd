@tool
extends RefCounted


const WIRES_INT_TO_ENUM = {
    0: TractionRenderingServer.Wires.CONTACT,
    1: TractionRenderingServer.Wires.CONTACT_SUPPORT,
    2: TractionRenderingServer.Wires.TWO_CONTACT_ONE_SUPPORT,
    3: TractionRenderingServer.Wires.TWO_CONTACT_TWO_SUPPORT,
}


func import(p:MaszynaParser, context: MaszynaImporterContext) -> MaszynaTraction3D:
    var obj = MaszynaTraction3D.new()
    obj.power_supply_name = p.next_token()
    obj.nominal_voltage = float(p.next_token())
    obj.max_current = float(p.next_token())
    obj.resistivity = float(p.next_token())
    obj.material = (
        MaszynaTraction3D.TractionMaterial.COPPER 
        if p.next_token().to_lower() == "cu"
        else MaszynaTraction3D.TractionMaterial.ALUMINIUM
    )
    obj.wire_thickness = float(p.next_token())
    obj.damage_flag = int(p.next_token())
    obj.contact_line = get_line_from_tokens(p.get_tokens(6).map(func(x): return float(x)))
    obj.support_line = get_line_from_tokens(p.get_tokens(6).map(func(x): return float(x)))
    obj.min_height = float(p.next_token())
    obj.segment_length = float(p.next_token())
    obj.wires = WIRES_INT_TO_ENUM[int(p.next_token())]
    if not obj.wires == TractionRenderingServer.Wires.CONTACT:
        obj.wire_offset = float(p.next_token())
    obj.visible = p.as_bool(p.next_token())
    var nt = p.next_token().to_lower()
    if nt == "parallel":
        obj.parallel = p.next_token()
        nt = p.next_token()
    if not nt == "endtraction":
        push_error("Invalid node::traction")
    return obj


func get_line_from_tokens(points) -> MaszynaTractionLine:
    var c = MaszynaTractionLine.new()
    c.p1 = Vector3(points[0], points[1], points[2])
    c.p2 = Vector3(points[3], points[4], points[5])
    return c
