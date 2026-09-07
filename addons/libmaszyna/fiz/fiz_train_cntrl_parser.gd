@tool
extends RefCounted
class_name FizTrainCntrlParser

## Cntrl. section dispatcher: this single FIZ section's keys fan out to several different
## Godot classes (TrainController general subset, TrainBrake brake subset, and later
## TrainEngine's controller-position-count subset once Engine: creates that node - Cntrl.
## conventionally appears before Engine: in real files, so the engine-relevant keys are
## stashed on the context for Engine:'s parser to pick up). Also owns the brake-position table
## (BPT) that immediately follows the Cntrl. line, by delegating to the brake parser.
## LoadFIZ_Cntrl: Mover.cpp:10707.

var controller_parser: FizTrainControllerParser
var brake_parser: FizTrainBrakeParser


func _init(p_controller_parser: FizTrainControllerParser, p_brake_parser: FizTrainBrakeParser) -> void:
    controller_parser = p_controller_parser
    brake_parser = p_brake_parser


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    controller_parser.apply_cntrl(kv, context)

    var brake: TrainBrake = context.get_part("TrainBrake")
    if brake != null:
        brake_parser.apply_cntrl(kv, brake, context)
    else:
        push_warning("FIZ Cntrl.: no TrainBrake node yet (Brake: should precede Cntrl.) - brake-related Cntrl. keys ignored.")

    # Engine:'s controller-position-count subset (MCPN, SCPN, AutoRelay, ...) is applied once
    # Engine: creates the TrainEngine-family node, since Cntrl. conventionally precedes Engine:.
    context.cntrl_kv = kv


func wants_bpt_table(context: FizImportContext) -> bool:
    return brake_parser.wants_bpt_table(context)


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    brake_parser.parse_row(p, context)


func end_table(context: FizImportContext) -> void:
    brake_parser.end_table(context)
