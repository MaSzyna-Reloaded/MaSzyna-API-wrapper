@tool
extends Resource
class_name MaszynaTrainsetData

## Scenery `trainset <name> <track> <offset> <velocity> ... endtrainset` (deserialize_trainset(),
## simulationstateserializer.cpp) - its vehicles stand on the track one after another from the
## offset and are coupled (RailVehicleServer.trainset_place()). A `dynamic` outside a `trainset` is
## a trainset of one, standing by its own track and offset.

## The track its front stands on, and the distance along it [m]
@export var track_name:String = ""
@export var offset:float = 0.0
## Its name; empty for a `dynamic` outside a `trainset`
@export var name:String = ""
## The timetable of its driver (the original takes the trainset's name for it: a file in the
## scenery's directory, `none` for none) and the velocity it starts with - the driver's first
## orders (deserialize_endtrainset(), simulationstateserializer.cpp:839-848). Empty for a
## `dynamic` outside a `trainset`.
@export var timetable:String = ""
@export var velocity:float = 0.0
## Its vehicles, front first
@export var dynamics:Array[MaszynaDynamicData] = []
