extends MaszynaGutTest


const CELL_SIZE:float = 1000.0


func _triangle_area(a:Vector3, b:Vector3, c:Vector3) -> float:
    return (b - a).cross(c - a).length() * 0.5


## A chunk is streamed and culled by its cell, so no piece of it may reach outside
func test_triangle_crossing_cells_is_cut_along_the_grid() -> void:
    var vertices:PackedVector3Array = PackedVector3Array([
        Vector3(-1500, 0, -1500), Vector3(0, 0, 1500), Vector3(1500, 0, -1500),
    ])
    var normals:PackedVector3Array = PackedVector3Array([Vector3.UP, Vector3.UP, Vector3.UP])
    var uvs:PackedVector2Array = PackedVector2Array([
        Vector2(-1.5, -1.5), Vector2(0.0, 1.5), Vector2(1.5, -1.5),
    ])
    var source_facing:Vector3 = (vertices[1] - vertices[0]).cross(vertices[2] - vertices[0])
    var chunks:Array = SceneryTrianglesBuilder.build_chunks([["grass", vertices, normals, uvs]], CELL_SIZE)
    assert_gt(chunks.size(), 1)
    var area:float = 0.0
    for chunk:Dictionary in chunks:
        var points:PackedVector3Array = chunk["vertices"]
        var cell_min:Vector2 = Vector2(chunk["chunk_x"], chunk["chunk_z"]) * CELL_SIZE
        for index:int in range(0, points.size(), 3):
            var corners:Array[Vector3] = []
            for offset:int in 3:
                var world_position:Vector3 = points[index + offset] + chunk["origin"]
                corners.append(world_position)
                assert_between(world_position.x, cell_min.x - 0.001, cell_min.x + CELL_SIZE + 0.001)
                assert_between(world_position.z, cell_min.y - 0.001, cell_min.y + CELL_SIZE + 0.001)
                # the source UVs are the position in kilometres, so a cut vertex must follow it
                var uv:Vector2 = chunk["uvs"][index + offset]
                assert_almost_eq(uv, Vector2(world_position.x, world_position.z) / CELL_SIZE, Vector2(0.0001, 0.0001))
                assert_almost_eq(chunk["normals"][index + offset], Vector3.UP, Vector3(0.0001, 0.0001, 0.0001))
            area += _triangle_area(corners[0], corners[1], corners[2])
            var facing:Vector3 = (corners[1] - corners[0]).cross(corners[2] - corners[0])
            assert_gt(facing.dot(source_facing), 0.0, "a piece must keep the winding of its triangle")
    assert_almost_eq(area, _triangle_area(vertices[0], vertices[1], vertices[2]), 0.01)


func test_triangle_within_one_cell_is_stored_unchanged() -> void:
    var vertices:PackedVector3Array = PackedVector3Array([
        Vector3(1100, 5, 1100), Vector3(1100, 7, 1900), Vector3(1900, 6, 1100),
    ])
    var normals:PackedVector3Array = PackedVector3Array([Vector3.UP, Vector3.RIGHT, Vector3.FORWARD])
    var uvs:PackedVector2Array = PackedVector2Array([Vector2(0, 0), Vector2(1, 0), Vector2(0, 1)])
    var chunks:Array = SceneryTrianglesBuilder.build_chunks([["grass", vertices, normals, uvs]], CELL_SIZE)
    assert_eq(chunks.size(), 1)
    var chunk:Dictionary = chunks[0]
    assert_eq(chunk["vertices"].size(), 3)
    for index:int in 3:
        assert_eq(chunk["vertices"][index] + chunk["origin"], vertices[index])
        assert_eq(chunk["normals"][index], normals[index])
        assert_eq(chunk["uvs"][index], uvs[index])


## Two triangles sharing an edge list its ends in opposite order; a cut computed from either end
## would differ in the last bits and open a crack along the grid line
func test_shared_edge_is_cut_at_identical_vertices() -> void:
    var edge_start:Vector3 = Vector3(310.7, 3.3, 120.9)
    var edge_end:Vector3 = Vector3(2790.1, 9.1, 2611.3)
    var normals:PackedVector3Array = PackedVector3Array([Vector3.UP, Vector3.UP, Vector3.UP])
    var uvs:PackedVector2Array = PackedVector2Array([Vector2.ZERO, Vector2.RIGHT, Vector2.DOWN])
    var cuts:Array[Array] = []
    for triangle:PackedVector3Array in [
        PackedVector3Array([edge_start, edge_end, Vector3(2900, 0, 150)]),
        PackedVector3Array([edge_end, edge_start, Vector3(200, 0, 2800)]),
    ]:
        var on_edge:Array[Vector3] = []
        for chunk:Dictionary in SceneryTrianglesBuilder.build_chunks([["grass", triangle, normals, uvs]], CELL_SIZE):
            for point:Vector3 in chunk["vertices"]:
                var world_position:Vector3 = point + chunk["origin"]
                var along:float = (world_position - edge_start).dot(edge_end - edge_start) / (edge_end - edge_start).length_squared()
                var on_line:bool = world_position.distance_to(edge_start.lerp(edge_end, along)) < 0.0001
                if on_line and not on_edge.has(world_position):
                    on_edge.append(world_position)
        on_edge.sort()
        cuts.append(on_edge)
    assert_gt(cuts[0].size(), 2, "the edge crosses grid lines, so it must have been cut")
    assert_eq(cuts[0], cuts[1])


func test_entries_share_buffers_only_with_same_material_and_cell() -> void:
    var vertices:PackedVector3Array = PackedVector3Array([
        Vector3(-10, 0, -10), Vector3(-20, 0, -10), Vector3(-10, 0, -20),
    ])
    var normals:PackedVector3Array = PackedVector3Array([Vector3.UP, Vector3.UP, Vector3.UP])
    var uvs:PackedVector2Array = PackedVector2Array([Vector2.ZERO, Vector2.RIGHT, Vector2.DOWN])
    var chunks:Array = SceneryTrianglesBuilder.build_chunks([
        ["grass", vertices, normals, uvs],
        ["grass", vertices, normals, uvs],
        ["sand", vertices, normals, uvs],
    ], 1000.0)
    assert_eq(chunks.size(), 2)
    for chunk:Dictionary in chunks:
        assert_eq(chunk["chunk_x"], -1)
        assert_eq(chunk["chunk_z"], -1)
        assert_eq(chunk["vertices"].size(), 6 if chunk["texture"] == "grass" else 3)
