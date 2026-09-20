@tool
extends Resource
class_name MaszynaCompiledScenery


@export var format_version:int = 0
@export var source_path:String = ""
@export var parameters_hash:String = ""
@export var dependencies:Dictionary = {}
@export var nodes:PackedScene
@export var tracks:Array[MaszynaTrackData] = []
@export var traction:Array[MaszynaTractionData] = []
@export var power_sources:Array[MaszynaPowerSourceData] = []
@export var models:Array[MaszynaModelData] = []
## Merged triangle meshes, streamed by SceneryChunkRenderingServer (not nodes, so not in [member nodes])
@export var triangle_chunks:Array[MaszynaTrianglesChunkData] = []
