@tool
extends RefCounted
class_name FizTrainSecuritySystemParser

## Security: section parser -> TrainSecuritySystem (czuwak/SHP/radiostop). Registered directly
## in FizTrainControllerInstancer's section table.
##
## This repo's vendored Mover.cpp doesn't keep the original LoadFIZ_Security loader (no
## LoadFIZ_* functions survived vendoring at all - see the other fiz_train_*_parser.gd files'
## same note), so the AwareSystem= token vocabulary below is inferred from
## TrainSecuritySystem's own property names (aware_system_active/cab_signal/
## separate_acknowledge/sifa - MOVER.h's basic_security_system::vigilance_enabled/
## cabsignal_enabled/separate_acknowledge/is_sifa) rather than confirmed against original
## source - treat the exact token spellings as best-effort pending a more authoritative source
## if precise behavior ever matters.


func create_node() -> TrainSecuritySystem:
    return TrainSecuritySystem.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainSecuritySystem", node)

    if kv.has("AwareSystem"):
        var tokens: PackedStringArray = FizLineUtil.get_string(kv, "AwareSystem").split(",")
        var flags: Array[String] = []
        for token: String in tokens:
            flags.append(token.strip_edges().to_lower())
        node.set_aware_system_active("active" in flags)
        node.set_aware_system_cabsignal("cabsignal" in flags)
        node.set_aware_system_separate_acknowledge("separateacknowledge" in flags)
        node.set_aware_system_sifa("sifa" in flags)
    if kv.has("AwareDelay"):
        node.set_aware_delay(FizLineUtil.get_float(kv, "AwareDelay"))
    if kv.has("SoundSignalDelay"):
        node.set_sound_signal_delay(FizLineUtil.get_float(kv, "SoundSignalDelay"))
    if kv.has("MaxHoldTime"):
        node.set_ca_max_hold_time(FizLineUtil.get_float(kv, "MaxHoldTime"))
    if kv.has("EmergencyBrakeDelay"):
        node.set_emergency_brake_delay(FizLineUtil.get_float(kv, "EmergencyBrakeDelay"))
    if kv.has("RadioStop"):
        node.set_radio_stop_enabled(FizLineUtil.get_bool(kv, "RadioStop"))
    if kv.has("SHPDist"):
        node.set_shp_magnet_distance(FizLineUtil.get_float(kv, "SHPDist"))
