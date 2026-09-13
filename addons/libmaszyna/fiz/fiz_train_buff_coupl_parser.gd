@tool
extends RefCounted
class_name FizTrainBuffCouplParser

## BuffCoupl./BuffCoupl1./BuffCoupl2. sections -> TrainBuffCoupl (one node per coupler end
## encountered). LoadFIZ_BuffCoupl: Mover.cpp:10619.
##
## Setters are only called when the corresponding FIZ key is present - TrainBuffCoupl's own
## compiled-in property defaults already match the FIZ format's "key absent" behavior, except
## the coupler/buffer stiffness+tolerance fields, whose x1000 unit conversion depends on the
## resolved coupler type and are therefore always (re)applied once that type is known.

const _COUPLER_TYPE_MAP := {
    "automatic": TrainBuffCoupl.COUPLER_TYPE_AUTOMATIC,
    "screw": TrainBuffCoupl.COUPLER_TYPE_SCREW,
    "chain": TrainBuffCoupl.COUPLER_TYPE_CHAIN,
    "bare": TrainBuffCoupl.COUPLER_TYPE_BARE,
    "articulated": TrainBuffCoupl.COUPLER_TYPE_ARTICULATED,
}

const ALLOWED_FIXED_COUPLING_LOCK := 128


func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := TrainBuffCoupl.new()

    var coupler_type: int = _COUPLER_TYPE_MAP.get(FizLineUtil.get_string(kv, "CType").to_lower(), TrainBuffCoupl.COUPLER_TYPE_AUTOMATIC)
    if kv.has("CType"):
        node.set_coupler_type(coupler_type)

    # kC/FmaxC/kB/FmaxB are given in kN / kN*m^-1 and converted to base SI (x1000) for "real"
    # coupler types only; Bare/Articulated use hardcoded physical constants in the original
    # code that weren't pinned down precisely enough to replicate here (FIZ-provided values,
    # if any, are applied unconverted).
    var is_real_coupler: bool = coupler_type in [
        TrainBuffCoupl.COUPLER_TYPE_AUTOMATIC, TrainBuffCoupl.COUPLER_TYPE_SCREW, TrainBuffCoupl.COUPLER_TYPE_CHAIN]
    var unit_mult: float = 1000.0 if is_real_coupler else 1.0

    if kv.has("kC"):
        node.set_coupler_stiffness_k(FizLineUtil.get_float(kv, "kC") * unit_mult)
    if kv.has("DmaxC"):
        node.set_coupler_max_compression_tolerance(FizLineUtil.get_float(kv, "DmaxC"))
    if kv.has("FmaxC"):
        node.set_coupler_max_tension_tolerance(FizLineUtil.get_float(kv, "FmaxC") * unit_mult)

    if kv.has("kB"):
        node.set_buffer_stiffness_k(FizLineUtil.get_float(kv, "kB") * unit_mult)
    if kv.has("DmaxB"):
        node.set_buffer_max_compression_tolerance(FizLineUtil.get_float(kv, "DmaxB"))
    if kv.has("FmaxB"):
        node.set_buffer_max_tension_tolerance(FizLineUtil.get_float(kv, "FmaxB") * unit_mult)

    if kv.has("beta"):
        node.set_damping_beta(FizLineUtil.get_float(kv, "beta"))

    if kv.has("AllowedFlag"):
        var allowed: int = FizLineUtil.get_int(kv, "AllowedFlag")
        if allowed < 0:
            allowed = -allowed | ALLOWED_FIXED_COUPLING_LOCK
        node.set_allowed_flag(allowed)
    if kv.has("AutomaticFlag"):
        node.set_automatic_flag(FizLineUtil.get_int(kv, "AutomaticFlag"))
    if kv.has("PowerCoupling"):
        node.set_power_coupling(FizLineUtil.get_int(kv, "PowerCoupling"))
    if kv.has("PowerFlag"):
        node.set_power_flag(FizLineUtil.get_int(kv, "PowerFlag"))
    if kv.has("ControlType"):
        node.set_control_type(FizLineUtil.get_string(kv, "ControlType"))

    context.add_part(_part_name_for(prefix, context), node)


func _part_name_for(prefix: String, context: FizImportContext) -> String:
    match prefix:
        "BuffCoupl1.": return "TrainBuffCouplFront"
        "BuffCoupl2.": return "TrainBuffCouplBack"
        _: return "TrainBuffCoupl"
