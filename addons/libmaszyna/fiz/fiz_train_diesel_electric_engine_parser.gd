@tool
extends RefCounted
class_name FizTrainDieselElectricEngineParser

## TrainDieselElectricEngine's own subset of Engine: (EngineType=DieselElectric/DumbDE, called
## directly by FizTrainEngineParser once it creates the node), plus WWList:+rows and
## MotorParamTable:+rows (both registered directly in FizTrainControllerInstancer's section
## table, using the standard parse()/parse_row()/end_table() interface).
##
## WWList: rows map 1:1 onto TrainDieselElectricEngine.wwlist (WWListItem), which
## _do_update_internal_mover already pushes into the mover's DElist/SST tables - that C++ side
## was already fully wired, only the FIZ-side parser was missing (this is what blocked
## main_switch/direction/brake on any DumbDE vehicle: DElist stayed all-zero, and MainCtrlPosNo
## is derived from wwlist.size(), so it stayed at -1 with no rows at all).
##
## WWList row columns (readWWList, this repo's vendored Mover.cpp doesn't keep the original
## LoadFIZ_* loader, so this is sourced from the still-present reader function plus the
## already-written C++ side's field mapping): RPM GenPower Umax Imax, optionally followed by
## 3 more columns (shunt-mode wakeup Umin/Umax/Pmax) enabling has_shunting for that row.
##
## MotorParamTable: (the diesel/diesel-electric variant, no "0" suffix - distinct from
## MotorParamTable0:, which only ElectricSeriesMotor uses) rows share FizTrainEngineCommon's
## parse_motor_param_row() and populate the same TrainEngine.motor_param_table -> mover
## MotorParam[] used by TractionForce()'s DieselElectric branch for Im (motor current). Without
## it MotorParam[] stays all-zero, which divides by zero computing Im (-> inf), which then
## zeroes Ft via the "clamp Im to tempImax" step (Mover.cpp ~5403-5423) - this is what left a
## DumbDE vehicle producing engine current but exactly zero traction force even with the WWList
## fix applied and every startup step (fuel/oil pump, main switch, direction, controller
## position) done correctly.
##
## Engine:'s diesel-electric-specific subset (Flat/Vhyp/Vadd/Cr/RelayType/ShuntMode/HeatingRPM):
## exact original LoadFIZ_Engine semantics aren't recoverable from this repo's vendored source
## (the FIZ loader itself isn't part of the vendored physics-only copy), so unit conversions
## below (Vhyp/Vadd km/h->m/s, matching every other velocity-like FIZ field in this codebase)
## and the Flat/ShuntMode literal-"1" comparison (documented server-side quirk, replicated from
## the original research pass over upstream eu07/maszyna) are best-effort.

var _wwlist_rows: Array[WWListItem] = []
var _motor_param_rows: Array[MotorParameter] = []
var _active_table: String = ""


func create_node() -> TrainDieselElectricEngine:
    return TrainDieselElectricEngine.new()


## The diesel-electric-specific subset of Engine:'s key/value set (common fields already
## applied by FizTrainEngineCommon via FizTrainEngineParser).
func apply_engine_fields(kv: Dictionary, node: TrainDieselElectricEngine) -> void:
    if kv.has("Flat"):
        # Original quirk: compares to the literal string "1", not the normal Yes/No convention.
        node.set_generator_voltage_flat(FizLineUtil.get_string(kv, "Flat") == "1")
    if kv.has("Vhyp"):
        node.set_hyperbolic_speed(FizLineUtil.get_float(kv, "Vhyp") / 3.6)
    if kv.has("Vadd"):
        node.set_additional_speed(FizLineUtil.get_float(kv, "Vadd") / 3.6)
    if kv.has("Cr"):
        node.set_power_correction_ratio(FizLineUtil.get_float(kv, "Cr"))
    if kv.has("RelayType"):
        node.set_shunt_relay_type(FizLineUtil.get_int(kv, "RelayType"))
    if kv.has("ShuntMode"):
        # Same literal-"1" quirk as Flat.
        node.set_shunt_mode_allowed(FizLineUtil.get_string(kv, "ShuntMode") == "1")
    if kv.has("HeatingRPM"):
        node.set_heating_rpm(FizLineUtil.get_float(kv, "HeatingRPM"))


## Standard section-parser interface, used for "WWList:" and "MotorParamTable:" (registered
## directly against this instance in FizTrainControllerInstancer's section table).
func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    FizLineUtil.read_key_values(p) # header line's own key=value pairs (e.g. WWList's "Size=") -
                                    # informational only, rows are self-terminating either way.
    if prefix == "MotorParamTable:":
        _active_table = "MotorParamTable"
        _motor_param_rows = []
    else:
        _active_table = "WWList"
        _wwlist_rows = []


func _get_node(context: FizImportContext) -> TrainDieselElectricEngine:
    var node: TrainPart = context.get_part("TrainEngine")
    return node as TrainDieselElectricEngine


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    match _active_table:
        "WWList": _parse_wwlist_row(p)
        "MotorParamTable": _parse_motor_param_row(p)


func _parse_wwlist_row(p: MaszynaParser) -> void:
    var tokens: Array = p.get_tokens(7)
    if tokens.size() < 4:
        return
    var item := WWListItem.new()
    item.set_rpm(float(tokens[0]))
    item.set_max_power(float(tokens[1]))
    item.set_max_voltage(float(tokens[2]))
    item.set_max_current(float(tokens[3]))
    if tokens.size() >= 7:
        item.set_has_shunting(true)
        item.set_min_wakeup_voltage(float(tokens[4]))
        item.set_max_wakeup_voltage(float(tokens[5]))
        item.set_max_wakeup_power(float(tokens[6]))
    _wwlist_rows.append(item)


func _parse_motor_param_row(p: MaszynaParser) -> void:
    var item := FizTrainEngineCommon.parse_motor_param_row(p)
    if item:
        _motor_param_rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node := _get_node(context)
    if node == null:
        _wwlist_rows = []
        _motor_param_rows = []
        return
    if not _wwlist_rows.is_empty():
        node.set_wwlist(_wwlist_rows)
    if not _motor_param_rows.is_empty():
        node.set_motor_param_table(_motor_param_rows)
    _wwlist_rows = []
    _motor_param_rows = []
