extends MaszynaGutTest


func test_crossing_cells_preserves_each_triangle_once() -> void:
    var vertices:PackedVector3Array = PackedVector3Array([
        Vector3(-1500, 2, -1500), Vector3(1500, 3, -1500), Vector3(0, 4, 1500),
        Vector3(1999, 5, 1999), Vector3(2002, 6, 1999), Vector3(1999, 7, 2002),
    ])
    var normals:PackedVector3Array = PackedVector3Array([
        Vector3.UP, Vector3.RIGHT, Vector3.FORWARD,
        Vector3.DOWN, Vector3.LEFT, Vector3.BACK,
    ])
    var uvs:PackedVector2Array = PackedVector2Array([
        Vector2(0, 0), Vector2(1, 0), Vector2(0, 1),
        Vector2(2, 0), Vector2(2, 1), Vector2(1, 2),
    ])
    var chunks:Array = SceneryTrianglesBuilder.build_chunks([["grass", vertices, normals, uvs]], 1000.0)
    assert_eq(chunks.size(), 2)
    var seen:Array[int] = []
    for chunk:Dictionary in chunks:
        var points:PackedVector3Array = chunk["vertices"]
        assert_eq(points.size(), 3)
        for index:int in range(points.size()):
            var world_position:Vector3 = points[index] + chunk["origin"]
            var source_index:int = vertices.find(world_position)
            assert_gte(source_index, 0)
            if source_index < 0:
                continue
            assert_false(seen.has(source_index), "vertices must not be duplicated across cells")
            seen.append(source_index)
            assert_eq(chunk["normals"][index], normals[source_index])
            assert_eq(chunk["uvs"][index], uvs[source_index])
    assert_eq(seen.size(), vertices.size())


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
