@tool
extends RefCounted
class_name FizTrainCntrlParser

## Cntrl. section dispatcher: this single FIZ section's keys fan out to several different
## Godot classes (VehicleController general subset, VehicleMasterController - every vehicle's,
## an engine's or not -, VehicleBrake brake subset, and later
## VehicleEngine's controller-position-count subset once Engine: creates that node - Cntrl.
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

    var master_controller:MoverVehicleMasterController = MoverVehicleMasterController.new()
    master_controller.main_position_count = FizLineUtil.get_int(kv, "MCPN")
    master_controller.second_position_count = FizLineUtil.get_int(kv, "SCPN")
    master_controller.direction_change_max_position = FizLineUtil.get_int(kv, "DirChangeMaxPos")
    master_controller.coupled_controllers = FizLineUtil.get_bool(kv, "CoupledCtrl")
    master_controller.initial_delay = FizLineUtil.get_float(kv, "IniCDelay")
    master_controller.step_delay = FizLineUtil.get_float(kv, "SCDelay")
    # without SCDDelay stepping down is as slow as up (Mover.cpp:10868)
    master_controller.step_down_delay = FizLineUtil.get_float(kv, "SCDDelay", master_controller.step_delay)
    context.add_part("VehicleMasterController", master_controller)

    var brake: VehicleBrake = context.get_part("VehicleBrake")
    if brake != null:
        brake_parser.apply_cntrl(kv, brake, context)
    else:
        push_warning("FIZ Cntrl.: no VehicleBrake node yet (Brake: should precede Cntrl.) - brake-related Cntrl. keys ignored.")

    # Engine:'s subset (AutoRelay, Camshaft, ...) is applied once
    # Engine: creates the VehicleEngine-family node when Cntrl. precedes it, or here when it comes
    # after - EN57's Cntrl. is in the brake include that follows its Engine:
    context.cntrl_kv = kv
    var engine: VehicleEngine = context.get_part("VehicleEngine") as VehicleEngine
    if engine:
        FizTrainEngineCommon.apply_cntrl_engine_subset(engine, kv)
        if engine is VehicleElectricEngine:
            FizTrainEngineCommon.apply_cntrl_electric_subset(engine as VehicleElectricEngine, kv)


func wants_bpt_table(context: FizImportContext) -> bool:
    return brake_parser.wants_bpt_table(context)


func parse_row(p: MaszynaParser, context: FizImportContext) -> void:
    brake_parser.parse_row(p, context)


func end_table(context: FizImportContext) -> void:
    brake_parser.end_table(context)
