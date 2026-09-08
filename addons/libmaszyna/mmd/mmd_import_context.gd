extends RefCounted
class_name MmdImportContext

## Cross-file shared state for a single MMD parse pass, mirroring FizImportContext's role for
## FIZ imports. One instance per MmdCabinInstancer.parse() call (shared across `include`d files).

var base_dir:String = ""
var include_depth:int = 0
var cab_number:int = 1

## Random-file-set `include [a.inc b.inc] end` choices, keyed by "source_file:line" of the
## include directive - owned by the caller (DynamicTrainCabin) and passed back in on every
## parse() call so a later cab1<->cab2 rebuild reuses the same choice instead of re-rolling it.
var random_choices:Dictionary = {}

func log_message(
        severity:String, code:String, message:String,
        source_file:String = "", line:int = 0, mmd_label:String = "", submodel_name:String = "") -> void:
    var location:String = " (%s:%d)" % [source_file, line] if source_file else ""
    var details:String = " [cab=%d label=%s submodel=%s]" % [cab_number, mmd_label, submodel_name]
    var text:String = "MMD [%s] %s%s%s" % [code, message, location, details]
    if severity == "error":
        push_error(text)
    elif severity == "warning":
        push_warning(text)
