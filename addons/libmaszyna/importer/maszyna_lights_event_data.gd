@tool
extends Resource
class_name MaszynaLightsEventData

## Scenery `event <name> lights <delay> <targets> <values> endevent` (lights_event,
## Event.cpp:1741-1803) - what a semaphore's kind is made of (see
## [MaszynaLegacySemaphoreKindFactory])

## Lower case, as the original keeps event names (Event.cpp:2202)
@export var name:String = ""
## Names of the models it is aimed at
@export var targets:PackedStringArray = []
## One TAnimModel::LightSet() value per light, light 0 first
@export var values:PackedFloat32Array = []
