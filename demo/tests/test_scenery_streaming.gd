extends MaszynaGutTest

## SceneryStreamingServer: registered placements are built only while the streaming camera is
## within their range of the chunk they fall into.

## Frames given to the streaming worker and the frame-budgeted build/clear before a check fails
const STREAMING_FRAMES:int = 60
const CHUNK_SIZE_M:float = 1000.0

var _camera:Camera3D
var _rids:Array[RID] = []


func before_each() -> void:
    _camera = Camera3D.new()
    add_child_autoqfree(_camera)
    E3DRenderingServer.set_model_loader(_load_test_model)
    SceneryStreamingServer.set_camera(_camera)


func after_each() -> void:
    for rid:RID in _rids:
        E3DRenderingServer.instance_free(rid)
    _rids.clear()
    SceneryStreamingServer.set_camera(null)
    E3DRenderingServer.set_model_loader(E3DModelManager.load_model)


func test_registered_instance_is_built_only_within_range() -> void:
    _register(Vector3.ZERO, 200.0)

    await _move_camera(Vector3(4 * CHUNK_SIZE_M, 0, 0))
    assert_eq(SceneryStreamingServer.get_streamed_count(), 0, "built while out of range")

    await _move_camera(Vector3.ZERO)
    assert_eq(SceneryStreamingServer.get_streamed_count(), 1, "not built inside the range")

    await _move_camera(Vector3(4 * CHUNK_SIZE_M, 0, 0))
    assert_eq(SceneryStreamingServer.get_streamed_count(), 0, "not cleared after leaving")


func test_range_of_a_node_without_one_is_capped_by_the_draw_distance() -> void:
    var draw_distance:float = SceneryStreamingServer.get_draw_distance()
    _register(Vector3.ZERO, 0.0)

    await _move_camera(Vector3(draw_distance + 2 * CHUNK_SIZE_M, 0, 0))
    assert_eq(SceneryStreamingServer.get_streamed_count(), 0, "built beyond the draw distance")

    await _move_camera(Vector3(draw_distance * 0.5, 0, 0))
    assert_eq(SceneryStreamingServer.get_streamed_count(), 1, "not built within the draw distance")


func test_registered_instance_is_freed_before_it_is_ever_built() -> void:
    var rid:RID = _register(Vector3.ZERO, 200.0)

    await _move_camera(Vector3(4 * CHUNK_SIZE_M, 0, 0))
    E3DRenderingServer.instance_free(rid)
    _rids.erase(rid)

    await _move_camera(Vector3.ZERO)
    assert_eq(SceneryStreamingServer.get_streamed_count(), 0, "a freed registration was built")


## Building a chunk needs a real renderer (a headless material has no shader), so this covers the
## registration side only: an out-of-range chunk stays unbuilt and frees cleanly
func test_triangle_chunk_out_of_range_is_not_built() -> void:
    var chunk := MaszynaTrianglesChunkData.new()
    chunk.mesh = _create_mesh()
    chunk.position = Vector3.ZERO
    chunk.range_max = 200.0
    var rid:RID = SceneryChunkRenderingServer.create_chunk(chunk, get_tree().root.world_3d.scenario)

    await _move_camera(Vector3(4 * CHUNK_SIZE_M, 0, 0))
    assert_eq(SceneryStreamingServer.get_streamed_count(), 0, "chunk built while out of range")

    SceneryChunkRenderingServer.free_chunk(rid)
    await _move_camera(Vector3.ZERO)
    assert_eq(SceneryStreamingServer.get_streamed_count(), 0, "a freed chunk was built")


func _register(position:Vector3, range_max:float) -> RID:
    var rid:RID = E3DRenderingServer.instance_register(
        "models/test", "streamed", PackedStringArray(),
        Transform3D(Basis(), position), 0.0, range_max, get_tree().root.world_3d.scenario
    )
    _rids.append(rid)
    return rid


## Planning runs on a worker thread and the builds are spread over frames, so a check waits for
## the streaming to settle instead of assuming it happened in one frame
func _move_camera(position:Vector3) -> void:
    _camera.global_position = position
    await wait_idle_frames(STREAMING_FRAMES)


func _load_test_model(_data_path:String, _filename:String) -> E3DModel:
    var mesh_submodel:E3DSubModel = E3DSubModel.new()
    mesh_submodel.resource_name = "mesh"
    mesh_submodel.submodel_type = E3DSubModel.SUBMODEL_GL_TRIANGLES
    mesh_submodel.mesh = _create_mesh()
    var model:E3DModel = E3DModel.new()
    model.submodels = [mesh_submodel]
    return model


func _create_mesh() -> ArrayMesh:
    var mesh:ArrayMesh = ArrayMesh.new()
    mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, BoxMesh.new().get_mesh_arrays())
    return mesh
