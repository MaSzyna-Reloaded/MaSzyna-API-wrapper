@tool
extends RefCounted
class_name FizTrainLoadParser

## Load: section parser -> TrainLoad. Registered directly in FizTrainControllerInstancer's
## section table. Real syntax is a single scalar line (confirmed against ~25 real vehicle
## files, wagons/locomotives alike, all matching this shape exactly):
## `Load: MaxLoad=64 LoadQ=tonns LoadAccepted=Coal,Ore,Calcium LoadSpeed=1 UnLoadSpeed=0.1
## OverLoadFactor=2`. `minimum_load_offsets`/`load_list` (LoadListItem rows) have no
## corresponding key in any real file checked - left at compiled defaults.


func create_node() -> TrainLoad:
    return TrainLoad.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainLoad", node)

    if kv.has("MaxLoad"):
        node.set_max_load(FizLineUtil.get_float(kv, "MaxLoad"))
    if kv.has("LoadQ"):
        match FizLineUtil.get_string(kv, "LoadQ").to_lower():
            "pieces": node.set_load_unit(TrainLoad.LOAD_UNIT_PIECES)
            "tonns", "tons": node.set_load_unit(TrainLoad.LOAD_UNIT_TONS)
    if kv.has("LoadAccepted"):
        var loads: PackedStringArray = FizLineUtil.get_string(kv, "LoadAccepted").split(",")
        var accepted: Array[String] = []
        for load: String in loads:
            accepted.append(load.strip_edges())
        node.set_accepted_loads(accepted)
    if kv.has("LoadSpeed"):
        node.set_load_speed(FizLineUtil.get_float(kv, "LoadSpeed"))
    if kv.has("UnLoadSpeed"):
        node.set_unload_speed(FizLineUtil.get_float(kv, "UnLoadSpeed"))
    if kv.has("OverLoadFactor"):
        node.set_overload_factor(FizLineUtil.get_float(kv, "OverLoadFactor"))
