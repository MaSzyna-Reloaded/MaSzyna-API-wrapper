@tool
extends RefCounted
class_name FizTrainUniversalControllerParser

## UCList: section parser -> TrainUniversalController. Registered directly in
## FizTrainControllerInstancer's section table.
##
## Row format confirmed exactly against readUCList (Mover.cpp:8427-8442): 10 tokens per row -
## a leading index (discarded, matches every other List's convention) then 9 data columns in
## this exact order: mode, MinCtrlVal, MaxCtrlVal, SetCtrlVal, SpeedUp, SpeedDown,
## ReturnPosition, NextPosFastInc, PrevPosFastDec - mapping 1:1 onto
## UniversalControllerListItem's pneumatic_brake_position/min_percentage/max_percentage/
## target_value/increase_speed/decrease_speed/bounce_back_position/nearest_stable_up/
## nearest_stable_down.

var _rows: Array[UniversalControllerListItem] = []


func create_node() -> TrainUniversalController:
    return TrainUniversalController.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainUniversalController", node)

    if kv.has("IntegratedBrakePN"):
        node.set_integrated_brake_pn(FizLineUtil.get_bool(kv, "IntegratedBrakePN"))
    if kv.has("IntegratedBrake"):
        node.set_integrated_brake(FizLineUtil.get_bool(kv, "IntegratedBrake"))
    _rows = []


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    var tokens: Array = p.get_tokens(10)
    if tokens.size() < 10:
        return
    var item := UniversalControllerListItem.new()
    item.set_pneumatic_brake_position(int(tokens[1]))
    item.set_min_percentage(float(tokens[2]))
    item.set_max_percentage(float(tokens[3]))
    item.set_target_value(float(tokens[4]))
    item.set_increase_speed(float(tokens[5]))
    item.set_decrease_speed(float(tokens[6]))
    item.set_bounce_back_position(int(tokens[7]))
    item.set_nearest_stable_up(int(tokens[8]))
    item.set_nearest_stable_down(int(tokens[9]))
    _rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node: TrainPart = context.get_part("TrainUniversalController")
    if node == null:
        _rows = []
        return
    if not _rows.is_empty():
        (node as TrainUniversalController).set_positions(_rows)
    _rows = []
