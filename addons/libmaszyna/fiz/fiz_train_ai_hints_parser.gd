@tool
extends RefCounted
class_name FizTrainAIHintsParser

## AI: section parser -> TrainAIHints. Registered directly in FizTrainControllerInstancer's
## section table.
##
## Keys confirmed against a real vehicle line: `AI: Pantstate=1 LocalBrakeAccFactor=0.98
## IdlePantUp=No`.


func create_node() -> TrainAIHints:
    return TrainAIHints.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var node := create_node()
    context.add_part("TrainAIHints", node)

    if kv.has("Pantstate"):
        node.set_pantograph_state(FizLineUtil.get_int(kv, "Pantstate", TrainAIHints.PANTOGRAPH_STATE_FRONT))
    if kv.has("LocalBrakeAccFactor"):
        node.set_local_brake_acceleration_factor(FizLineUtil.get_float(kv, "LocalBrakeAccFactor"))
    if kv.has("IdlePantUp"):
        node.set_raise_pantographs_when_idle(FizLineUtil.get_bool(kv, "IdlePantUp"))
