@tool
extends RefCounted
class_name FizTrainEngineParser

## Engine: section dispatcher. Decodes EngineType, creates the matching concrete RailVehicleEngine
## subclass, applies the fields common to every engine (FizTrainEngineCommon) plus the
## stashed Cntrl./Power: subsets, then delegates the remaining type-specific fields to the
## matching concrete engine parser. LoadFIZ_Engine: Mover.cpp:11119.
##
## WheelsDriven, Dumb and Steam engines are not built (the TODO branch below); every other type
## has its own field-mapping parser.

var electric_series_parser: FizTrainElectricSeriesEngineParser = FizTrainElectricSeriesEngineParser.new()
var diesel_electric_parser: FizTrainDieselElectricEngineParser = FizTrainDieselElectricEngineParser.new()
var electric_induction_parser: FizTrainElectricInductionEngineParser = FizTrainElectricInductionEngineParser.new()
var diesel_parser: FizTrainDieselEngineParser = FizTrainDieselEngineParser.new()


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    var kv: Dictionary = FizLineUtil.read_key_values(p)
    var engine_type: int = FizTrainEngineCommon.parse_engine_type(FizLineUtil.get_string(kv, "EngineType"))
    context.engine_type = engine_type

    var node: RailVehicleEngine
    match engine_type:
        RailVehicleEngine.ELECTRIC_SERIES_MOTOR:
            node = electric_series_parser.create_node()
            context.add_part("RailVehicleEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            FizTrainEngineCommon.apply_cntrl_electric_subset(node as RailVehicleElectricEngine, context.cntrl_kv)
            FizTrainEngineCommon.apply_power(node, context.power_kv)
            electric_series_parser.apply_engine_fields(kv, node)
        RailVehicleEngine.DIESEL_ELECTRIC:
            node = diesel_electric_parser.create_node()
            context.add_part("RailVehicleEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            diesel_electric_parser.apply_engine_fields(kv, node)
            FizTrainDieselEngineParser.apply_diesel_common(kv, node as RailVehicleDieselEngine)
        RailVehicleEngine.ELECTRIC_INDUCTION_MOTOR:
            node = electric_induction_parser.create_node()
            context.add_part("RailVehicleEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            FizTrainEngineCommon.apply_cntrl_electric_subset(node as RailVehicleElectricEngine, context.cntrl_kv)
            FizTrainEngineCommon.apply_power(node, context.power_kv)
            electric_induction_parser.apply_engine_fields(kv, node)
        RailVehicleEngine.DIESEL:
            node = MoverRailVehicleDieselEngine.new()
            context.add_part("RailVehicleEngine", node)
            FizTrainEngineCommon.apply_engine_common(node, kv, context)
            FizTrainEngineCommon.apply_cntrl_engine_subset(node, context.cntrl_kv)
            diesel_parser.apply_engine_fields(kv, node as RailVehicleDieselEngine)
        RailVehicleEngine.WHEELS_DRIVEN, RailVehicleEngine.DUMB, RailVehicleEngine.STEAM:
            # TODO: no wrapper engine class for these yet
            push_warning(
                    "FIZ Engine:EngineType=%s: no engine class for it yet." % FizLineUtil.get_string(kv, "EngineType"))
        _:
            push_warning("FIZ Engine:EngineType=%s: unrecognized or unsupported." % FizLineUtil.get_string(kv, "EngineType"))
