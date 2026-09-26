@tool
extends RefCounted

## Closes whatever "trainset:" opened (deserialize_endtrainset(), simulationstateserializer.cpp).
## Vehicles are coupled at runtime by TrainSet3D; the timetable reaches the trainset's driver when
## the drivers are built (SceneryInstancer._build_drivers()).
func import(_p:MaszynaParser, context: MaszynaImporterContext) -> Array:
    context.trainset_open = false
    context.trainset_node = null
    return []
