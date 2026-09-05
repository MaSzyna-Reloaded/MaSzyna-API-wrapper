@tool
extends RefCounted
class_name FizTrainElectricInductionEngineParser

## TrainElectricInductionEngine's own subset of Engine: (EngineType=ElectricInductionMotor,
## called directly by FizTrainEngineParser once it creates the node), plus ffList:/ffBrakeList:
## and PmaxList:+rows (registered directly in FizTrainControllerInstancer's section table).
##
## Engine: key mapping confirmed two ways: (1) directly against a real vehicle line
## (`en57al_v1/al-zachpom_i_rb.fiz:45`): `Engine: EngineType=ElectricInductionMotor
## Trans=46:252 dfic=377 dfmax=0.98 p=2 cfu=46.8 cim=14.2 icif=0.671 Uzmax=2340 Uzh=2340 DU=20
## I0=20 fcfu=49 F0=130 a1=0 Pmax=0 Fh=130 Ph=1000 Vh0=5 Vh1=10 Imax=557 abed=0 edep=0.95` - 21
## keys in this exact order; (2) TMoverParameters::eimc_labels (Mover.cpp:36-39), a dense
## debug-display array in the SAME 21-entry order ("dfic:,dfmax:,p:,scfu:,cim:,icif:,Uzmax:,
## Uzh:,DU:,I0:,fcfu:,F0:,a1:,Pmax:,Fh:,Ph:,Vh0:,Vh1:,Imax:,abed:,eped:") - matching 1:1
## against the real file's key order confirms the semantic identity of each key (minor label
## spelling quirks: "scfu"/"cfu" and "eped"/"edep" are the same key, just written differently
## in the debug label vs the real FIZ key). `abed`/`edep` have no corresponding
## TrainElectricInductionEngine property at all (not invented here - see AGENTS.md). `fcfuH`
## (the braking-mode counterpart of `fcfu`, mirroring `Uzh` being `Uzmax`'s braking
## counterpart) maps to `inverter_uf_setpoint_braking` by the same naming pattern, though it
## doesn't appear in the one real example checked.
##
## DElist-backed property (`wwlist`, reused from TrainDieselElectricEngine's row shape - see
## FizTrainDieselElectricEngineParser's docstring) is what unblocks `ffList:`/`ffBrakeList:` in
## Batch 6 - see this class's own note there, and fiz_train_controller_instancer.gd's comment
## on those two prefixes.


var _wwlist_rows: Array[WWListItem] = []
var _max_power_rows: Array[CurvePointItem] = []
var _active_table: String = ""


func create_node() -> TrainElectricInductionEngine:
    return TrainElectricInductionEngine.new()


## The EIM-specific subset of Engine:'s key/value set (common fields already applied by
## FizTrainEngineCommon via FizTrainEngineParser).
func apply_engine_fields(kv: Dictionary, node: TrainElectricInductionEngine) -> void:
    if kv.has("dfic"):
        node.set_slip_current_ratio(FizLineUtil.get_float(kv, "dfic"))
    if kv.has("dfmax"):
        node.set_max_slip(FizLineUtil.get_float(kv, "dfmax"))
    if kv.has("p"):
        node.set_pole_pairs(FizLineUtil.get_float(kv, "p"))
    if kv.has("cfu"):
        node.set_nominal_uf_ratio(FizLineUtil.get_float(kv, "cfu"))
    if kv.has("cim"):
        node.set_current_torque_ratio(FizLineUtil.get_float(kv, "cim"))
    if kv.has("icif"):
        node.set_current_three_phase_ratio(FizLineUtil.get_float(kv, "icif"))
    if kv.has("Uzmax"):
        node.set_max_supply_voltage(FizLineUtil.get_float(kv, "Uzmax"))
    if kv.has("Uzh"):
        node.set_max_supply_voltage_braking(FizLineUtil.get_float(kv, "Uzh"))
    if kv.has("DU"):
        node.set_inverter_voltage_drop(FizLineUtil.get_float(kv, "DU"))
    if kv.has("I0"):
        node.set_no_load_current(FizLineUtil.get_float(kv, "I0"))
    if kv.has("fcfu"):
        node.set_inverter_uf_setpoint(FizLineUtil.get_float(kv, "fcfu"))
    if kv.has("fcfuH"):
        node.set_inverter_uf_setpoint_braking(FizLineUtil.get_float(kv, "fcfuH"))
    if kv.has("F0"):
        node.set_initial_force(FizLineUtil.get_float(kv, "F0"))
    if kv.has("a1"):
        node.set_force_drop_rate(FizLineUtil.get_float(kv, "a1"))
    if kv.has("Pmax"):
        node.set_max_power(FizLineUtil.get_float(kv, "Pmax"))
    if kv.has("Fh"):
        node.set_max_braking_force(FizLineUtil.get_float(kv, "Fh"))
    if kv.has("Ph"):
        node.set_max_braking_power(FizLineUtil.get_float(kv, "Ph"))
    # NOTE: matches TrainElectricInductionEngine::_do_update_internal_mover's existing
    # (already-wired) assignment exactly: eimc_p_Vh0 <- braking_decay_velocity, eimc_p_Vh1 <-
    # braking_decay_start_velocity - the property names read as though this should be swapped,
    # but the FIZ key must land in the eimc[] slot the C++ side already pushes it from.
    if kv.has("Vh0"):
        node.set_braking_decay_velocity(FizLineUtil.get_float(kv, "Vh0"))
    if kv.has("Vh1"):
        node.set_braking_decay_start_velocity(FizLineUtil.get_float(kv, "Vh1"))
    if kv.has("Imax"):
        node.set_motor_max_current(FizLineUtil.get_float(kv, "Imax"))


## Standard section-parser interface, used for "ffList:"/"ffBrakeList:" (both share this
## instance's wwlist target, first-write-wins - see class doc and
## fiz_train_controller_instancer.gd's comment on these two prefixes) and "PmaxList:"
## (TrainElectricInductionEngine.max_power_table, already fully wired C++-side).
func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    FizLineUtil.read_key_values(p) # header line's own key=value pairs, if any - informational
    if prefix == "PmaxList:":
        _active_table = "PmaxList"
        _max_power_rows = []
    else:
        _active_table = "ff"
        _wwlist_rows = []


func _get_node(context: FizImportContext) -> TrainElectricInductionEngine:
    var node: TrainPart = context.get_part("TrainEngine")
    return node as TrainElectricInductionEngine


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    match _active_table:
        "ff": _parse_ff_row(p)
        "PmaxList": _parse_curve_row(p)


func _parse_ff_row(p: MaszynaParser) -> void:
    # readFFList (Mover.cpp:8530-8543): 2 columns, RPM and GenPower - same DElist row shape as
    # WWList's first two columns, reused via WWListItem (max_voltage/max_current stay at 0.0).
    var tokens: Array = p.get_tokens(2)
    if tokens.size() < 2:
        return
    var item := WWListItem.new()
    item.set_rpm(float(tokens[0]))
    item.set_max_power(float(tokens[1]))
    _wwlist_rows.append(item)


func _parse_curve_row(p: MaszynaParser) -> void:
    var tokens: Array = p.get_tokens(2)
    if tokens.size() < 2:
        return
    var item := CurvePointItem.new()
    item.set_x(float(tokens[0]))
    item.set_y(float(tokens[1]))
    _max_power_rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node := _get_node(context)
    if node == null:
        _wwlist_rows = []
        _max_power_rows = []
        return

    if _active_table == "ff":
        if not _wwlist_rows.is_empty():
            if not node.get_wwlist().is_empty():
                # first-write-wins: ffList:/ffBrakeList: target the same DElist-backed
                # property - whichever of the two appeared first in the file keeps it.
                push_warning(
                        "FIZ: ffList:/ffBrakeList: both present in the same file - keeping the " +
                        "first one parsed, discarding this table's rows.")
            else:
                node.set_wwlist(_wwlist_rows)
    elif _active_table == "PmaxList":
        if not _max_power_rows.is_empty():
            node.set_max_power_table(_max_power_rows)

    _wwlist_rows = []
    _max_power_rows = []
    _active_table = ""
