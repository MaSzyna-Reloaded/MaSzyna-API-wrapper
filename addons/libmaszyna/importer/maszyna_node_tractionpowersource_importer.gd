@tool
extends RefCounted

## Plain data holder - mirrors maszyna_node_traction_importer.gd's TractionData doc comment:
## built directly against TractionPowerServer's RID-based API
## (scenery_instancer.gd's _build_power_source()) instead of a Node3D.
class PowerSourceData extends RefCounted:
    var name:String = ""
    var position:Vector3 = Vector3.ZERO
    var nominal_voltage:float = 0.0
    var voltage_frequency:float = 0.0
    var internal_resistance:float = 0.0
    var max_output_current:float = 0.0
    var fast_fuse_timeout:float = 0.0
    var fast_fuse_repetition:float = 0.0
    var slow_fuse_timeout:float = 0.0
    var is_section:bool = false
    var recuperation:bool = false


func import(p:MaszynaParser, context:MaszynaImporterContext) -> PowerSourceData:
    var data := PowerSourceData.new()
    var position:Vector3 = p.next_vector3()
    data.position = position.rotated(Vector3.UP, context.rotate.y) + context.origin
    data.nominal_voltage = float(p.next_token())
    data.voltage_frequency = float(p.next_token())
    data.internal_resistance = float(p.next_token())
    data.max_output_current = float(p.next_token())
    data.fast_fuse_timeout = float(p.next_token())
    data.fast_fuse_repetition = float(p.next_token())
    data.slow_fuse_timeout = float(p.next_token())
    if data.internal_resistance < 0.1:
        # matches TractionPower.cpp:65 - real-world DC substations are never this low
        data.internal_resistance = 0.2
    var token = p.next_token().to_lower()
    if token == "recuperation":
        data.recuperation = true
    elif token == "section":
        data.is_section = true
    while not token.is_empty() and not token == "end":
        token = p.next_token()
    return data
