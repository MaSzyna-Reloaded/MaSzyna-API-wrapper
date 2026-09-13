@tool
extends RefCounted
class_name FizTrainEngineParser

## Engine: section dispatcher. Decodes EngineType, creates the matching concrete TrainEngine
## subclass, applies the fields common to every engine (FizTrainEngineCommon) plus the
## stashed Cntrl./Power: subsets, then delegates the remaining type-specific fields to the
## matching concrete engine parser. LoadFIZ_Engine: Mover.cpp:11119.
##
## NOTE: only Diesel still gets the generic common-fields-only stub treatment (see the TODO
## branch below) - ElectricSeriesMotor, DieselElectric and ElectricInductionMotor all have
## their own dedicated field-mapping parsers.

var electric_series_parser: FizTrainElectricSeriesEngineParser = FizTrainElectricSeriesEngineParser.new()
var diesel_electric_parser: FizTrainDieselElectricEngineParser = FizTrainDieselElectricEngineParser.new()
var electric_induction_parser: FizTrainElectricInductionEngineParser = FizTrainElectricInductionEngineParser.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var engine_type: int = FizTrainEngineCommon.parse_engine_type(FizLineUtil.get_string(kv, "EngineType"))
    context.engine_type = engine_type

    var node: TrainEngine
    match engine_type:
        TrainEngine.ELECTRIC_SERIES_MOTOR:
            node = electric_series_parser.create_node()
            context.add_part("TrainEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            FizTrainEngineCommon.apply_power(node, context.power_kv)
            electric_series_parser.apply_engine_fields(kv, node)
        TrainEngine.DIESEL_ELECTRIC:
            node = diesel_electric_parser.create_node()
            context.add_part("TrainEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            diesel_electric_parser.apply_engine_fields(kv, node)
        TrainEngine.ELECTRIC_INDUCTION_MOTOR:
            node = electric_induction_parser.create_node()
            context.add_part("TrainEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            FizTrainEngineCommon.apply_power(node, context.power_kv)
            electric_induction_parser.apply_engine_fields(kv, node)
        TrainEngine.DIESEL, \
        TrainEngine.WHEELS_DRIVEN, TrainEngine.DUMB, TrainEngine.STEAM:
            # TODO: type-specific field mapping not written yet - only common fields applied.
            push_warning(
                    "FIZ Engine:EngineType=%s: only common TrainEngine fields are mapped so far." %
                    FizLineUtil.get_string(kv, "EngineType"))
            node = _create_stub_node(engine_type)
            if node:
                context.add_part("TrainEngine", node)
                FizTrainEngineCommon.apply_engine_common(node, kv, context)
                FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
        _:
            push_warning("FIZ Engine:EngineType=%s: unrecognized or unsupported." % FizLineUtil.get_string(kv, "EngineType"))


func _create_stub_node(engine_type: int) -> TrainEngine:
    match engine_type:
        TrainEngine.DIESEL: return TrainDieselEngine.new()
        _: return null
