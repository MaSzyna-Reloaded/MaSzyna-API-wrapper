@tool
extends RefCounted
class_name FizTrainElectricSeriesEngineParser

## TrainElectricSeriesEngine's own subset of Engine: (EngineType=ElectricSeriesMotor, called
## directly by FizTrainEngineParser once it creates the node), plus Circuit:, RList:+rows, and
## MotorParamTable0:+rows (all registered directly in FizTrainControllerInstancer's section
## table, using the standard parse()/parse_row()/end_table() interface) - all configure the
## same node created by FizTrainEngineParser. Series-motor branch of LoadFIZ_Engine:
## Mover.cpp:11119, LoadFIZ_Circuit: Mover.cpp:11416, LoadFIZ_RList: Mover.cpp:11444,
## readRList: Mover.cpp:9231, readMPT0: Mover.cpp:9069.
##
## MotorParamTable0: column mapping (idx A B C D E F, per
## https://wiki.eu07.pl/index.php/Plik_charakterystyki, "Zmienne dla pojazdow elektrycznych z
## rozruchem oporowym" section, which documents its own worked example under the literal
## header "MotorParamTable0:" matching real files exactly): the wiki names every column -
## A=fin, B=bl, C=mfi, D=mIsat, E=fi, F=Isat - but marks EVERY one of them with a trailing "?",
## i.e. the community itself isn't confident about any of these six names, not just some of
## them. C/D/E/F are mapped here to MotorParameter's voltage_constant_multiplier/
## saturation_current_multiplier/voltage_constant/saturation_current (the closest plausible
## match to mfi/mIsat/fi/Isat); A is mapped to initial_voltage_constant on a similarly
## best-effort basis ("fin" plausibly abbreviating "fi initial"/fi0); B ("bl") is left
## unmapped, since nothing about it is identifiable at all against MotorParameter's remaining
## fields. Populating the table at all (rather than leaving it empty) is what fixes a real
## crash-adjacent bug - see test_train_electric_engine_power_source.gd's sibling investigation
## - but the exact per-column values should be treated as approximate pending a more
## authoritative source (e.g. reading Mover.cpp's readMPT0 field-by-field) if precise traction
## behavior ever matters.

var _relay_rows: Array[RelayListItem] = []
var _motor_param_rows: Array[MotorParameter] = []
var _active_table: String = ""


func create_node() -> TrainElectricSeriesEngine:
    return TrainElectricSeriesEngine.new()


## The series-motor-specific subset of Engine:'s key/value set (common fields already applied
## by FizTrainEngineCommon via FizTrainEngineParser).
func apply_engine_fields(kv: Dictionary, node: TrainElectricSeriesEngine) -> void:
    if kv.has("Volt"):
        node.set_nominal_voltage(FizLineUtil.get_float(kv, "Volt"))
    if kv.has("WindingRes"):
        # floors to 0.01 when the key resolves to 0, matching the original's division-by-zero guard.
        node.set_winding_resistance(maxf(FizLineUtil.get_float(kv, "WindingRes"), 0.01))
    if kv.has("nmax"):
        node.set_max_rpm(FizLineUtil.get_float(kv, "nmax") / 60.0)


## Standard section-parser interface, used for "Circuit:", "RList:" and "MotorParamTable0:"
## (registered directly against this instance in FizTrainControllerInstancer's section table).
func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    if prefix == "Circuit:":
        _parse_circuit(FizLineUtil.read_key_values(p), context)
    elif prefix == "RList:":
        _active_table = "RList"
        _relay_rows = []
        _parse_rlist_header(FizLineUtil.read_key_values(p), context)
    elif prefix == "MotorParamTable0:":
        _active_table = "MotorParamTable0"
        _motor_param_rows = []


func _get_node(context: FizImportContext) -> TrainElectricSeriesEngine:
    var node: TrainPart = context.get_part("TrainEngine")
    return node as TrainElectricSeriesEngine


func _parse_circuit(kv: Dictionary, context: FizImportContext) -> void:
    var node := _get_node(context)
    if node == null:
        return
    if kv.has("CircuitRes"):
        node.set_circuit_resistance(FizLineUtil.get_float(kv, "CircuitRes"))
    if kv.has("ImaxLo"):
        node.set_imax_low(FizLineUtil.get_int(kv, "ImaxLo"))
    if kv.has("ImaxHi"):
        node.set_imax_high(FizLineUtil.get_int(kv, "ImaxHi"))
    if kv.has("IminLo"):
        node.set_imin_low(FizLineUtil.get_int(kv, "IminLo"))
    if kv.has("IminHi"):
        node.set_imin_high(FizLineUtil.get_int(kv, "IminHi"))
    if kv.has("TUHEX_Sum"):
        node.set_tuhex_sum(FizLineUtil.get_float(kv, "TUHEX_Sum"))
    if kv.has("TUHEX_Diff"):
        node.set_tuhex_diff(FizLineUtil.get_float(kv, "TUHEX_Diff"))
    if kv.has("TUHEX_MaxIw"):
        node.set_tuhex_max_current(FizLineUtil.get_float(kv, "TUHEX_MaxIw"))
    if kv.has("TUHEX_MinIw"):
        node.set_tuhex_min_current(FizLineUtil.get_float(kv, "TUHEX_MinIw"))
    if kv.has("TUHEX_Stages"):
        node.set_tuhex_stages(FizLineUtil.get_int(kv, "TUHEX_Stages"))
    if kv.has("TUHEX_Sum1"):
        node.set_tuhex_sum_1(FizLineUtil.get_float(kv, "TUHEX_Sum1"))
    if kv.has("TUHEX_Sum2"):
        node.set_tuhex_sum_2(FizLineUtil.get_float(kv, "TUHEX_Sum2"))
    if kv.has("TUHEX_Sum3"):
        node.set_tuhex_sum_3(FizLineUtil.get_float(kv, "TUHEX_Sum3"))


func _parse_rlist_header(kv: Dictionary, context: FizImportContext) -> void:
    var node := _get_node(context)
    if node == null:
        return
    var vent_str: String = FizLineUtil.get_string(kv, "RVent").to_lower()
    match vent_str:
        "automatic": node.set_fan_type(TrainElectricSeriesEngine.FAN_TYPE_AUTOMATIC)
        "yes": node.set_fan_type(TrainElectricSeriesEngine.FAN_TYPE_YES)
    if vent_str == "automatic" or vent_str == "yes":
        if kv.has("RVentnmax"):
            node.set_fan_max_rpm(FizLineUtil.get_float(kv, "RVentnmax") / 60.0)
        if kv.has("RVentCutOff"):
            node.set_fan_cutoff_resistance(FizLineUtil.get_float(kv, "RVentCutOff"))
    if kv.has("RVentMinI"):
        node.set_fan_min_current(FizLineUtil.get_float(kv, "RVentMinI"))
    if kv.has("RVentSpeed"):
        node.set_fan_speed(FizLineUtil.get_float(kv, "RVentSpeed"))
    if kv.has("DynBrakeRes"):
        node.set_dynamic_brake_resistance(FizLineUtil.get_float(kv, "DynBrakeRes"))
    if kv.has("DynBrakeRes1"):
        node.set_dynamic_brake_resistance_1(FizLineUtil.get_float(kv, "DynBrakeRes1"))
    if kv.has("DynBrakeRes2"):
        node.set_dynamic_brake_resistance_2(FizLineUtil.get_float(kv, "DynBrakeRes2"))


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    match _active_table:
        "RList": _parse_rlist_row(p)
        "MotorParamTable0": _parse_motor_param_row(p)


func _parse_rlist_row(p: MaszynaParser) -> void:
    var tokens: Array = p.get_tokens(6)
    if tokens.size() < 5:
        return
    var item := RelayListItem.new()
    item.set_relay_position(int(tokens[0]))
    item.set_resistance(float(tokens[1]))
    item.set_branch_count(int(tokens[2]))
    item.set_motors_per_branch(int(tokens[3]))
    item.set_auto_switch(String(tokens[4]) == "1" or String(tokens[4]).to_lower() == "yes")
    if tokens.size() >= 6:
        item.set_shunt_index(int(tokens[5]))
    _relay_rows.append(item)


func _parse_motor_param_row(p: MaszynaParser) -> void:
    var item := FizTrainEngineCommon.parse_motor_param_row(p)
    if item:
        _motor_param_rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node := _get_node(context)
    if node == null:
        _relay_rows = []
        _motor_param_rows = []
        return
    if not _relay_rows.is_empty():
        node.set_relay_list(_relay_rows)
    if not _motor_param_rows.is_empty():
        node.set_motor_param_table(_motor_param_rows)
    _relay_rows = []
    _motor_param_rows = []
