@tool
extends RefCounted
class_name FizTrainDieselEngineParser

## TrainDieselEngine's DList:/DMList:/HTCList:/V2NList: table sections. Registered directly in
## FizTrainControllerInstancer's section table. These are independent of whether Engine:'s own
## plain-DieselEngine field subset has a dedicated parser yet (still common-fields-only, same
## status ElectricInductionMotor was in before FizTrainDieselElectricEngineParser existed) -
## the node itself is created by FizTrainEngineParser's stub branch regardless.
##
## DList: header keys confirmed against a real vehicle line: `DList: Size=10 Mmax=2750
## nMmax=18.3 nmax=33.3 Mnmax=2142 nominalfill=1.0 Mstand=250.0 NomFuelConsRate=220`, cross-
## checked directly against TrainDieselEngine::_do_update_internal_mover's own field mapping
## (`dizel_nominalfill = nominal_fuel_dose` etc.) - all six keys map onto TrainDieselEngine's
## existing throttle_table-related properties. Row format confirmed via readDList
## (Mover.cpp:8444-8456): 3 columns, `Relay R Mn` -> ThrottlePositionItem's
## throttle_position/fuel_dose/clutch_behavior.
##
## DMList: rows -> torque_table (CurvePointItem). The C++ side already applies /60.0 to x
## (rpm -> rev/s) when pushing to the mover, matching readV2NMAXList's sibling readMPTDiesel-
## adjacent convention - so this parser pushes the raw rpm value unconverted.
##
## HTCList: rows -> torque_converter_table (CurvePointItem), no conversion either side.
##
## V2NList: rows -> vel2nmax_table (CurvePointItem) - see fiz_train_controller_instancer.gd's
## comment on this prefix for why it's implemented despite 0 occurrences in the operator's
## corpus (dizel_vel2nmax_Table has a real, confirmed consumer at Mover.cpp:7183-7185).

var _throttle_rows: Array[ThrottlePositionItem] = []
var _torque_rows: Array[CurvePointItem] = []
var _tc_rows: Array[CurvePointItem] = []
var _v2n_rows: Array[CurvePointItem] = []
var _active_table: String = ""


func _get_node(context: FizImportContext) -> TrainDieselEngine:
    var node: TrainPart = context.get_part("TrainEngine")
    return node as TrainDieselEngine


func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    match prefix:
        "DList:":
            _active_table = "DList"
            _throttle_rows = []
            var kv: Dictionary = FizLineUtil.read_key_values(p)
            var node := _get_node(context)
            if node:
                if kv.has("Mmax"):
                    node.set_max_torque(FizLineUtil.get_float(kv, "Mmax"))
                if kv.has("nMmax"):
                    node.set_max_torque_rpm(FizLineUtil.get_float(kv, "nMmax"))
                if kv.has("nmax"):
                    node.set_max_rpm(FizLineUtil.get_float(kv, "nmax"))
                if kv.has("Mnmax"):
                    node.set_max_rpm_torque(FizLineUtil.get_float(kv, "Mnmax"))
                if kv.has("nominalfill"):
                    node.set_nominal_fuel_dose(FizLineUtil.get_float(kv, "nominalfill"))
                if kv.has("Mstand"):
                    node.set_resistance_torque(FizLineUtil.get_float(kv, "Mstand"))
                if kv.has("NomFuelConsRate"):
                    node.set_nominal_fuel_consumption_rate(FizLineUtil.get_float(kv, "NomFuelConsRate"))
        "DMList:":
            _active_table = "DMList"
            _torque_rows = []
        "HTCList:":
            _active_table = "HTCList"
            _tc_rows = []
        "V2NList:":
            _active_table = "V2NList"
            _v2n_rows = []
        _:
            _active_table = ""


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    match _active_table:
        "DList": _parse_throttle_row(p)
        "DMList": _parse_curve_row(p, _torque_rows)
        "HTCList": _parse_curve_row(p, _tc_rows)
        "V2NList": _parse_curve_row(p, _v2n_rows)


func _parse_throttle_row(p: MaszynaParser) -> void:
    var tokens: Array = p.get_tokens(3)
    if tokens.size() < 3:
        return
    var item := ThrottlePositionItem.new()
    item.set_throttle_position(int(tokens[0]))
    item.set_fuel_dose(float(tokens[1]))
    item.set_clutch_behavior(int(tokens[2]))
    _throttle_rows.append(item)


func _parse_curve_row(p: MaszynaParser, rows: Array[CurvePointItem]) -> void:
    var tokens: Array = p.get_tokens(2)
    if tokens.size() < 2:
        return
    var item := CurvePointItem.new()
    item.set_x(float(tokens[0]))
    item.set_y(float(tokens[1]))
    rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node := _get_node(context)
    if node:
        match _active_table:
            "DList":
                if not _throttle_rows.is_empty():
                    node.set_throttle_table(_throttle_rows)
            "DMList":
                if not _torque_rows.is_empty():
                    node.set_torque_table(_torque_rows)
            "HTCList":
                if not _tc_rows.is_empty():
                    node.set_torque_converter_table(_tc_rows)
            "V2NList":
                if not _v2n_rows.is_empty():
                    node.set_vel2nmax_table(_v2n_rows)
    _throttle_rows = []
    _torque_rows = []
    _tc_rows = []
    _v2n_rows = []
    _active_table = ""
