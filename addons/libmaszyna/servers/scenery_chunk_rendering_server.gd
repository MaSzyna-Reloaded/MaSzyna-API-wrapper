@tool
extends Node

## RID based renderer of merged scenery triangle meshes (MaszynaTrianglesChunkData). A real
## scenery merges tens of thousands of "triangles" nodes into a few thousand chunks, and 40% of
## them declare no visibility range at all - drawn from anywhere on the map they alone cost more
## than the rest of the scenery. Chunks are registered with SceneryStreamingServer and their
## RenderingServer instance exists only while the camera is within range of their chunk.

class ChunkState:
    var mesh:ArrayMesh
    var transform:Transform3D
    var material_name:String
    var range_min:float
    var range_max:float
    var scenario:RID
    var stream_rid:RID
    var mesh_instance:RID
    ## MaterialManager only keeps a weakref (material_manager.gd), so the chunk holds the material
    ## for as long as it is built - like E3DInstanceData.materials does for models
    var material:Material

var _chunks:Dictionary[RID, ChunkState] = {}
var _next_id:int = 0
var _stream_owner:int = -1


## Registers a chunk for streaming; nothing is rendered until the camera comes within its range
func create_chunk(chunk:MaszynaTrianglesChunkData, scenario:RID) -> RID:
    if _stream_owner < 0:
        _stream_owner = SceneryStreamingServer.owner_create(Callable(), _stream_build, _stream_clear)

    var state := ChunkState.new()
    state.mesh = chunk.mesh
    state.transform = Transform3D(Basis(), chunk.position)
    state.material_name = chunk.material_name
    state.range_min = chunk.range_min
    state.range_max = chunk.range_max
    state.scenario = scenario

    _next_id += 1
    var rid:RID = rid_from_int64(_next_id)
    _chunks[rid] = state
    state.stream_rid = SceneryStreamingServer.stream_register(
        _stream_owner, rid, chunk.position, chunk.range_max
    )
    return rid


func free_chunk(rid:RID) -> void:
    var state:ChunkState = _chunks.get(rid)
    if not state:
        return
    SceneryStreamingServer.stream_free(state.stream_rid)
    _stream_clear(rid)
    _chunks.erase(rid)


func _stream_build(rid:RID, _preloaded:Variant) -> void:
    var state:ChunkState = _chunks.get(rid)
    if not state or state.mesh_instance.is_valid():
        return
    # an unresolved texture leaves nothing worth drawing, like a scenery model that fails to load
    var material:Material = MaterialManager.get_material("", state.material_name)
    if not material:
        return
    state.mesh_instance = RenderingServer.instance_create()
    RenderingServer.instance_set_base(state.mesh_instance, state.mesh.get_rid())
    RenderingServer.instance_set_scenario(state.mesh_instance, state.scenario)
    RenderingServer.instance_set_transform(state.mesh_instance, state.transform)
    state.material = material
    RenderingServer.instance_geometry_set_material_override(state.mesh_instance, material.get_rid())
    # the chunk's own range still culls it inside the streamed area, as it did on the node
    if state.range_max > 0:
        RenderingServer.instance_geometry_set_visibility_range(
            state.mesh_instance, state.range_min, state.range_max, 0.0, 0.0,
            RenderingServer.VISIBILITY_RANGE_FADE_DISABLED
        )


func _stream_clear(rid:RID) -> void:
    var state:ChunkState = _chunks.get(rid)
    if not state or not state.mesh_instance.is_valid():
        return
    RenderingServer.free_rid(state.mesh_instance)
    state.mesh_instance = RID()
    state.material = null
