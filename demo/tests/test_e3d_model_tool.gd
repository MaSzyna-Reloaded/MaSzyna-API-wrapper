extends MaszynaGutTest

var tool


func before_each() -> void:
    tool = E3DModelTool


func test_returns_first_root_mesh() -> void:
    var model: E3DModel = E3DModel.new()
    var first_mesh: ArrayMesh = ArrayMesh.new()
    var second_mesh: ArrayMesh = ArrayMesh.new()

    model.submodels = [
        _create_submodel(first_mesh),
        _create_submodel(second_mesh),
    ]

    assert_same(tool.find_first_mesh(model), first_mesh, "First root mesh should be returned")


func test_returns_nested_mesh_when_root_has_no_mesh() -> void:
    var model: E3DModel = E3DModel.new()
    var nested_mesh: ArrayMesh = ArrayMesh.new()
    var root: E3DSubModel = _create_submodel()
    root.submodels = [_create_submodel(nested_mesh)]
    model.submodels = [root]

    assert_same(tool.find_first_mesh(model), nested_mesh, "Nested mesh should be returned")


func test_returns_null_when_model_has_no_mesh() -> void:
    var model: E3DModel = E3DModel.new()
    var root: E3DSubModel = _create_submodel()
    root.submodels = [_create_submodel()]
    model.submodels = [root]

    assert_null(tool.find_first_mesh(model), "Model without meshes should return null")


func test_uses_depth_first_preorder() -> void:
    var model: E3DModel = E3DModel.new()
    var nested_mesh: ArrayMesh = ArrayMesh.new()
    var later_root_mesh: ArrayMesh = ArrayMesh.new()
    var first_root: E3DSubModel = _create_submodel()
    first_root.submodels = [_create_submodel(nested_mesh)]
    model.submodels = [
        first_root,
        _create_submodel(later_root_mesh),
    ]

    assert_same(tool.find_first_mesh(model), nested_mesh, "Nested mesh from first branch should win")


func test_get_aabb_applies_root_submodel_transform() -> void:
    var model: E3DModel = E3DModel.new()
    var submodel: E3DSubModel = _create_submodel(_create_mesh())
    submodel.transform = Transform3D(Basis.IDENTITY, Vector3(10, 0, 0))
    model.submodels = [submodel]

    var aabb: AABB = tool.get_aabb(model)

    assert_eq(aabb.position, Vector3(9, -2, -3), "AABB position should include root transform")
    assert_eq(aabb.size, Vector3(2, 4, 6), "AABB size should match transformed mesh extents")


func test_get_aabb_applies_nested_submodel_transforms() -> void:
    var model: E3DModel = E3DModel.new()
    var root: E3DSubModel = _create_submodel()
    var nested: E3DSubModel = _create_submodel(_create_mesh())
    root.transform = Transform3D(Basis.IDENTITY, Vector3(10, 0, 0))
    nested.transform = Transform3D(Basis.IDENTITY, Vector3(0, 20, 0))
    root.submodels = [nested]
    model.submodels = [root]

    var aabb: AABB = tool.get_aabb(model)

    assert_eq(aabb.position, Vector3(9, 18, -3), "AABB position should include parent and child transforms")
    assert_eq(aabb.size, Vector3(2, 4, 6), "AABB size should match transformed mesh extents")


func test_get_aabb_does_not_include_origin_when_model_has_meshes() -> void:
    var model: E3DModel = E3DModel.new()
    var submodel: E3DSubModel = _create_submodel(_create_mesh())
    submodel.transform = Transform3D(Basis.IDENTITY, Vector3(10, 20, 30))
    model.submodels = [submodel]

    var aabb: AABB = tool.get_aabb(model)

    assert_eq(aabb.position, Vector3(9, 18, 27), "AABB should start at mesh bounds, not origin")
    assert_eq(aabb.size, Vector3(2, 4, 6), "AABB should not be expanded by an empty initial AABB")


func _create_submodel(mesh: ArrayMesh = null) -> E3DSubModel:
    var submodel: E3DSubModel = E3DSubModel.new()
    submodel.mesh = mesh
    return submodel


func _create_mesh() -> ArrayMesh:
    var arrays: Array = []
    arrays.resize(Mesh.ARRAY_MAX)
    arrays[Mesh.ARRAY_VERTEX] = PackedVector3Array([
        Vector3(-1, -2, -3),
        Vector3(1, -2, -3),
        Vector3(1, 2, 3),
    ])

    var mesh: ArrayMesh = ArrayMesh.new()
    mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
    return mesh
