@tool
extends Resource
class_name MaszynaTrianglesChunkData

## One merged mesh of scenery "triangles" nodes sharing a texture, a 1 km cell and a visibility
## range (SceneryTrianglesBuilder.build_chunks()). The material is resolved through
## MaterialManager when the chunk is built, never stored here, so a cached scenery still follows
## season/weather material variants (material_manager::on_season_change, material.cpp:571).

@export var mesh:ArrayMesh
@export var position:Vector3 = Vector3.ZERO
@export var material_name:String = ""
@export var range_min:float = 0.0
## Visible up to this distance, 0 - no limit of its own (capped by the streaming draw distance)
@export var range_max:float = 0.0
