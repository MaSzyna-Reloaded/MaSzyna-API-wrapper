@tool
extends MeshInstance3D
class_name SceneryTrianglesChunk

## Mesh of scenery "triangles" nodes sharing one material. The material is resolved through
## MaterialManager when the chunk enters the scene, never stored in the compiled scenery cache,
## so a cached scenery still follows season/weather material variants (material_manager::on_season_change,
## material.cpp:571).

@export var material_name:String = ""


func _ready() -> void:
    material_override = MaterialManager.get_material("", material_name)
