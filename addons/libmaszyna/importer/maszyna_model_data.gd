@tool
extends Resource
class_name MaszynaModelData

## Scenery "node model" - built by SceneryInstancer as an E3DRenderingServer instance, not a node.

@export var data_path:String = ""
@export var model_filename:String = ""
@export var skins:PackedStringArray = []
@export var position:Vector3 = Vector3.ZERO
## Euler angles (radians), same order as Node3D.rotation
@export var rotation:Vector3 = Vector3.ZERO
@export var range_min:float = 0.0
## Visible up to this distance, 0 - no limit
@export var range_max:float = 0.0
