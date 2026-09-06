@tool
extends RefCounted
class_name FizTrainSpeedControlParser

## SpeedControl: section parser -> TrainSpeedControl. Registered directly in
## FizTrainControllerInstancer's section table.
##
## Key mapping confirmed against a real vehicle line (en57-class cohort):
## `SpeedControl: SpeedCtrl=Yes OverrideManual=No InitPwr=1.0 MaxPwrVel=-1 StartVel=-1
## VelStep=10 PwrStep=0.00 MinPwr=1.0 MaxPwr=1.0 MinVel=0 MaxVel=120 Offset=-0.1 kPpos=0.2
## kPneg=0.4 kIpos=0.00 kIneg=0.00 BrakeIntervention=No SpeedCtrlATOF=1`. `Delay=`/
## `ImpulseLever=`/`Buttons=`/`BrakeInterventionVel=`/`PwrUpSpeed=`/`PwrDownSpeed=` don't appear
## in that example - their key spellings below are a best-effort guess following the same
## abbreviation convention (Pwr=/Vel=) seen in the confirmed keys, not verified against a real
## file.


func create_node() -> TrainSpeedControl:
    return TrainSpeedControl.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainSpeedControl", node)

    if kv.has("SpeedCtrl"):
        node.set_speed_control_enabled(FizLineUtil.get_bool(kv, "SpeedCtrl"))
    if kv.has("Delay"):
        node.set_delay(FizLineUtil.get_float(kv, "Delay"))
    if kv.has("ImpulseLever"):
        node.set_impulse_lever(FizLineUtil.get_bool(kv, "ImpulseLever"))
    if kv.has("SpeedCtrlATOF"):
        node.set_disables_on(FizLineUtil.get_int(kv, "SpeedCtrlATOF"))
    if kv.has("Buttons"):
        var tokens: PackedStringArray = FizLineUtil.get_string(kv, "Buttons").split("|")
        var speeds := PackedFloat64Array()
        for token: String in tokens:
            if token.strip_edges().is_valid_float():
                speeds.append(token.strip_edges().to_float())
        node.set_preset_speeds(speeds)
    if kv.has("OverrideManual"):
        node.set_override_manual_power(FizLineUtil.get_bool(kv, "OverrideManual"))
    if kv.has("InitPwr"):
        node.set_initial_power(FizLineUtil.get_float(kv, "InitPwr"))
    if kv.has("MaxPwrVel"):
        node.set_full_power_velocity(FizLineUtil.get_float(kv, "MaxPwrVel"))
    if kv.has("StartVel"):
        node.set_start_velocity(FizLineUtil.get_float(kv, "StartVel"))
    if kv.has("VelStep"):
        node.set_velocity_step(FizLineUtil.get_float(kv, "VelStep"))
    if kv.has("PwrStep"):
        node.set_power_step(FizLineUtil.get_float(kv, "PwrStep"))
    if kv.has("MinPwr"):
        node.set_min_power(FizLineUtil.get_float(kv, "MinPwr"))
    if kv.has("MaxPwr"):
        node.set_max_power(FizLineUtil.get_float(kv, "MaxPwr"))
    if kv.has("MinVel"):
        node.set_min_velocity(FizLineUtil.get_float(kv, "MinVel"))
    if kv.has("MaxVel"):
        node.set_max_velocity(FizLineUtil.get_float(kv, "MaxVel"))
    if kv.has("Offset"):
        node.set_offset(FizLineUtil.get_float(kv, "Offset"))
    if kv.has("kPpos"):
        node.set_proportional_gain_positive(FizLineUtil.get_float(kv, "kPpos"))
    if kv.has("kPneg"):
        node.set_proportional_gain_negative(FizLineUtil.get_float(kv, "kPneg"))
    if kv.has("kIpos"):
        node.set_integral_gain_positive(FizLineUtil.get_float(kv, "kIpos"))
    if kv.has("kIneg"):
        node.set_integral_gain_negative(FizLineUtil.get_float(kv, "kIneg"))
    if kv.has("BrakeIntervention"):
        node.set_brake_intervention(FizLineUtil.get_bool(kv, "BrakeIntervention"))
    if kv.has("BrakeInterventionVel"):
        node.set_brake_intervention_max_velocity(FizLineUtil.get_float(kv, "BrakeInterventionVel"))
    if kv.has("PwrUpSpeed"):
        node.set_power_up_speed(FizLineUtil.get_float(kv, "PwrUpSpeed"))
    if kv.has("PwrDownSpeed"):
        node.set_power_down_speed(FizLineUtil.get_float(kv, "PwrDownSpeed"))
