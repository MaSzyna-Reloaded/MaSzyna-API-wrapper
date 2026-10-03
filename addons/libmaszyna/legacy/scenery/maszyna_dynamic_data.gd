@tool
extends Resource
class_name MaszynaDynamicData

## Scenery `node ... <name> dynamic <datafolder> <skinfile> <mmdfile> ... enddynamic`
## (deserialize_dynamic(), simulationstateserializer.cpp:960-1076) - a vehicle of a trainset
## (MaszynaTrainsetData), built by SceneryInstancer through the servers

## The scenery's name of the vehicle; may be empty or repeated
@export var name:String = ""
## Where its files are, under the game directory (`dynamic/pkp/303e_v1`)
@export var data_path:String = ""
## Its MMD/FIZ file name, without the extension
@export var file_name:String = ""
@export var skin:String = ""
## An `offset` of -1 stands the vehicle reversed (simulationstateserializer.cpp:983, DynObj.cpp:1807)
@export var direction:TrackServer.Direction = TrackServer.DIRECTION_NORMAL
## How far it stands behind the vehicle before it [m] - its `offset`
## (simulationstateserializer.cpp:1066)
@export var gap:float = 0.0
## Its coupling with the next vehicle of the trainset (`couplingdata`), a mask of
## RailVehicleController.CouplingFlags
@export var coupling:int = 0
## The velocity it starts with [km/h]
@export var velocity:float = 0.0
@export var driver_type:VehicleController.DriverType = VehicleController.DRIVER_NOBODY
@export var load_name:String = ""
@export var load_amount:float = 0.0
