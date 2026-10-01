@tool
extends MaszynaCompiledScenery
class_name MaszynaCompiledSubscene

## The subscene's terrain, chunked (SceneryTrianglesSink) - added to the scenery's sink wherever the
## subscene is included
@export var triangle_geometries:Array[MaszynaTrianglesChunkGeometry] = []
