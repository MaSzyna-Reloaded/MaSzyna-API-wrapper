@tool
extends RefCounted
class_name FizTrainPowerParser

## Power: section. Its fields belong to the TrainElectricEngine-family node (see
## fiz_train_engine_common.gd's apply_power()), which Engine: hasn't created yet at this point
## in the file (Power: conventionally precedes Engine: in real files) - so this parser only
## stashes the key/value set on the context for the concrete engine parser to apply once that
## node exists, mirroring how FizTrainCntrlParser stashes context.cntrl_kv.
## LoadFIZ_Power: Mover.cpp:11058.


func parse(p: MaszynaParser, context: FizImportContext, _prefix: String = "") -> void:
    context.power_kv = FizLineUtil.read_key_values(p)
