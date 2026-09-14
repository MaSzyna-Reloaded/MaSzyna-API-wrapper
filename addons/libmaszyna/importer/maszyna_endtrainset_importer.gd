@tool
extends RefCounted

## Closes whatever "trainset:" opened (deserialize_endtrainset(), simulationstateserializer.cpp).
## Vehicle-coupling/timetable/driver assignment isn't ported - see
## maszyna_node_dynamic_importer.gd for what is.
func import(_p:MaszynaParser, context: MaszynaImporterContext) -> Array:
    context.trainset_open = false
    return []
