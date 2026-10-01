@tool
extends Resource
class_name MaszynaTrianglesChunkGeometry

## The triangles of one MaszynaTrianglesChunkData, a cache file of its own read only while the
## camera is within the chunk's range (MaszynaSceneryChunkRenderingServer, ResourceLazyLoader) - a
## compiled scenery holding every chunk's mesh put the whole terrain in memory at once. Arrays, not
## an ArrayMesh: a mesh made on a loading thread races the renderer, so the mesh is made on the
## main thread as the chunk is built.

@export var vertices:PackedVector3Array
@export var normals:PackedVector3Array
@export var uvs:PackedVector2Array
