@tool
extends RefCounted
class_name FizTrainSwitchesParser

## Switches: and DimmerList: section parser -> TrainSwitches. Registered directly in
## FizTrainControllerInstancer's section table (both prefixes share this one instance).
##
## `Pantograph=`/`Converter=`/`MotorConnectors=` are string switch-type values, not Yes/No -
## confirmed against TrainSwitches::_do_update_internal_mover: "Impulse" (case-insensitive)
## maps to true, anything else (including absent) to false ("impulse"/"" or "impulse"/"toggle"
## on the mover side). `RelayResetButtonX=`/`PantographPresets=`/`PantographPresetDefault=`/
## `ModernDimmer=`/`DimmerList:` have no effect on the simulation in this vendored Mover (see
## TrainSwitches.hpp's class doc) but are still parsed and stored on the node faithfully.
##
## DimmerList: row format has no real example in the operator's ~1300-file corpus (0
## occurrences) - the 3-column mapping to DimmerListItem's high_beam/dimmed/off booleans is a
## best-effort guess from the field names alone, not confirmed against any real file.

var _dimmer_rows: Array[DimmerListItem] = []


func create_node() -> TrainSwitches:
    return TrainSwitches.new()


func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    if prefix == "DimmerList:":
        var kv: Dictionary = FizLineUtil.read_key_values(p)
        var node := _get_node(context)
        if node:
            if kv.has("Cycle"):
                node.set_dimmer_list_cycle(FizLineUtil.get_bool(kv, "Cycle"))
            if kv.has("Default"):
                node.set_dimmer_list_default_position(FizLineUtil.get_int(kv, "Default"))
        _dimmer_rows = []
        return

    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainSwitches", node)

    if kv.has("Pantograph"):
        node.set_pantograph_impulse(FizLineUtil.get_string(kv, "Pantograph").to_lower() == "impulse")
    if kv.has("Converter"):
        node.set_converter_impulse(FizLineUtil.get_string(kv, "Converter").to_lower() == "impulse")
    if kv.has("MotorConnectors"):
        node.set_motor_connectors_impulse(FizLineUtil.get_string(kv, "MotorConnectors").to_lower() == "impulse")
    if kv.has("RelayResetButton1"):
        node.set_relay_reset_button_1(FizLineUtil.get_int(kv, "RelayResetButton1"))
    if kv.has("RelayResetButton2"):
        node.set_relay_reset_button_2(FizLineUtil.get_int(kv, "RelayResetButton2"))
    if kv.has("RelayResetButton3"):
        node.set_relay_reset_button_3(FizLineUtil.get_int(kv, "RelayResetButton3"))
    if kv.has("PantographPresets"):
        var tokens: PackedStringArray = FizLineUtil.get_string(kv, "PantographPresets").split("|")
        var presets := PackedInt32Array()
        for token: String in tokens:
            if token.strip_edges().is_valid_int():
                presets.append(token.strip_edges().to_int())
        node.set_pantograph_presets(presets)
    if kv.has("PantographPresetDefault"):
        node.set_pantograph_preset_default(FizLineUtil.get_int(kv, "PantographPresetDefault"))
    if kv.has("ModernDimmer"):
        node.set_modern_dimmer(FizLineUtil.get_bool(kv, "ModernDimmer"))


func _get_node(context: FizImportContext) -> TrainSwitches:
    var node: TrainPart = context.get_part("TrainSwitches")
    return node as TrainSwitches


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    var tokens: Array = p.get_tokens(3)
    if tokens.size() < 3:
        return
    var item := DimmerListItem.new()
    item.set_high_beam(String(tokens[0]).to_lower() in ["1", "yes", "true"])
    item.set_dimmed(String(tokens[1]).to_lower() in ["1", "yes", "true"])
    item.set_off(String(tokens[2]).to_lower() in ["1", "yes", "true"])
    _dimmer_rows.append(item)


func end_table(context: FizImportContext) -> void:
    var node := _get_node(context)
    if node == null:
        _dimmer_rows = []
        return
    if not _dimmer_rows.is_empty():
        node.set_dimmer_list(_dimmer_rows)
    _dimmer_rows = []
