@tool
extends RefCounted

## "trainset: name track offset velocity" (deserialize_trainset(), simulationstateserializer.cpp)
## opens a trainset: every "node ... dynamic ... enddynamic" until the matching "endtrainset:" is a
## vehicle of it, which the trainset stands on this track from this offset (TrainSet3D).
func import(p:MaszynaParser, context: MaszynaImporterContext) -> Array:
    var tokens:Array = p.get_tokens(4)
    if tokens.size() < 4:
        return []

    context.trainset_open = true
    context.trainset_name = tokens[0]
    context.trainset_track = tokens[1]
    context.trainset_offset = float(tokens[2])
    context.trainset_velocity = float(tokens[3])
    context.trainset_node = TrainSet3D.new()
    context.trainset_node.name = context.trainset_name
    context.trainset_node.timetable = context.trainset_name
    context.trainset_node.velocity = context.trainset_velocity
    context.trainset_node.start_track_name = context.trainset_track
    context.trainset_node.start_track_offset = context.trainset_offset
    return [context.trainset_node]
