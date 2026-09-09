@tool
extends RefCounted
class_name FizTrainWipersParser

## WiperList: section parser -> TrainWipers. Registered directly in
## FizTrainControllerInstancer's section table. This vendored Mover has no wiper field at all
## (see TrainWipers.hpp's class doc) - no reader function exists to confirm the row column
## order, so the mapping below is inferred from field names plus the real example rows' own
## comments: `0 1.0 0.0 0.5 // zadna wycieraczka nie pracuje` (no wiper working) and
## `3 1.0 5.0 0.5 // obie wycieraczki ... z interwalem 5s` (both wipers, 5s interval) - the
## third column matching "interval 5s" exactly is what fixes column 2 as `period`.
## mode -> wiper_mask, col2 -> transit_time, col3 (interval) -> period, col4 -> return_delay.

var _rows: Array[WiperListItem] = []


func create_node() -> TrainWipers:
    return TrainWipers.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainWipers", node)

    if kv.has("Angle"):
        node.set_angle(FizLineUtil.get_float(kv, "Angle"))
    if kv.has("Default"):
        node.set_default_position(FizLineUtil.get_int(kv, "Default"))
    _rows = []


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    var tokens: Array = p.get_tokens(4)
    if tokens.size() < 4:
        return
    var item := WiperListItem.new()
    item.set_wiper_mask(int(tokens[0]))
    item.set_transit_time(float(tokens[1]))
    item.set_period(float(tokens[2]))
    item.set_return_delay(float(tokens[3]))
    _rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node: TrainPart = context.get_part("TrainWipers")
    if node == null:
        _rows = []
        return
    if not _rows.is_empty():
        (node as TrainWipers).set_positions(_rows)
    _rows = []
