@tool
extends RefCounted
class_name FizTrainLightingParser

## Light: and Headlights: sections -> TrainLighting. LoadFIZ_Light: Mover.cpp:11035,
## LoadFIZ_Headlights: Mover.cpp:10328, LoadFIZ_PowerParamsDecode: Mover.cpp:11547.
##
## NOTE: LightsList: rows are recognized (table_end "endL" in the instancer's section table)
## but not yet decoded into `lights/list` here - the two raw `valueA valueB` bitmask columns
## (readLightsList, Mover.cpp:9479) need to be split into LightListItem's 10 named booleans
## (cabin_a/b head_light + left/right red/white signal), and the exact bit layout wasn't
## pinned down by the FIZ format research - left as a follow-up rather than guessed.


func parse(p: MaszynaParser, context: FizImportContext, prefix: String = "") -> void:
    if prefix == "Headlights:":
        _parse_headlights(FizLineUtil.read_key_values(p), context)
        return
    if prefix == "LightsList:":
        return # header carries Size/Wrap/Default - table_end handling covers the rest for now.
    _parse_light(FizLineUtil.read_key_values(p), context)


func _get_node(context: FizImportContext) -> TrainLighting:
    var node: TrainLighting = context.get_part("TrainLighting")
    if node == null:
        node = TrainLighting.new()
        context.add_part("TrainLighting", node)
    return node


func _parse_light(kv: Dictionary, context: FizImportContext) -> void:
    var node := _get_node(context)

    if kv.has("Light"):
        node.set_light_source(FizTrainControllerParser.parse_power_source(FizLineUtil.get_string(kv, "Light")))
    if kv.has("LGeneratorEngine"):
        node.set_generator_engine(FizTrainEngineCommon.parse_engine_type(FizLineUtil.get_string(kv, "LGeneratorEngine")))
    if kv.has("AlterLight"):
        node.set_alternative_light_source(FizTrainControllerParser.parse_power_source(FizLineUtil.get_string(kv, "AlterLight")))
    if kv.has("AlterLMaxVoltage"):
        node.set_alternative_max_voltage(FizLineUtil.get_float(kv, "AlterLMaxVoltage"))

    # LMaxVoltage feeds TrainController.battery_voltage, not a TrainLighting property - see
    # doc_classes/TrainController.xml ([code]Light:LMaxVoltage[/code]).
    if kv.has("LMaxVoltage"):
        context.controller.set_battery_voltage(FizLineUtil.get_float(kv, "LMaxVoltage"))


func _parse_headlights(kv: Dictionary, context: FizImportContext) -> void:
    var node := _get_node(context)
    if kv.has("DimmedMultiplier"):
        node.set_head_light_dimmed_multiplier(FizLineUtil.get_float(kv, "DimmedMultiplier"))
    if kv.has("NormalMultiplier"):
        node.set_head_light_normal_multiplier(FizLineUtil.get_float(kv, "NormalMultiplier"))
    if kv.has("HighbeamDimmedMultiplier"):
        node.set_high_beam_dimmed_multiplier(FizLineUtil.get_float(kv, "HighbeamDimmedMultiplier"))
    if kv.has("HighBeamMultiplier"):
        node.set_high_beam_normal_multiplier(FizLineUtil.get_float(kv, "HighBeamMultiplier"))
    if kv.has("LampRed") or kv.has("LampGreen") or kv.has("LampBlue"):
        node.set_head_light_color(Color(
                FizLineUtil.get_float(kv, "LampRed", 255.0) / 255.0,
                FizLineUtil.get_float(kv, "LampGreen", 255.0) / 255.0,
                FizLineUtil.get_float(kv, "LampBlue", 255.0) / 255.0))
