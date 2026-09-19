extends MaszynaGutTest

## E3DRenderingServer RID API: OPTIMIZED instances create no nodes, NODES instances build
## a node tree under the attached node and follow the lights state.


func test_optimized_instance_creates_no_nodes() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)

    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_OPTIMIZED)
    E3DRenderingServer.instance_attach_node(rid, parent)
    E3DRenderingServer.instance_set_scenario(rid, parent.get_world_3d().scenario)
    E3DRenderingServer.instance_set_transform(rid, Transform3D(Basis(), Vector3(10, 0, 0)))
    E3DRenderingServer.instance_set_lights_state(rid, {"00": true})
    E3DRenderingServer.instance_build(rid)

    assert_true(rid.is_valid())
    assert_eq(parent.get_child_count(true), 0)
    E3DRenderingServer.instance_free(rid)


func test_nodes_instance_builds_tree_and_follows_lights_state() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)

    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_NODES)
    E3DRenderingServer.instance_attach_node(rid, parent)
    E3DRenderingServer.instance_set_lights_state(rid, {"00": false})
    E3DRenderingServer.instance_build(rid)

    var light_on: Node3D = parent.get_node(NodePath("light_on00"))
    var mesh: MeshInstance3D = parent.get_node(NodePath("light_on00/mesh"))
    assert_eq(parent.get_child_count(), 0, "generated nodes are internal")
    assert_eq(parent.get_child_count(true), 1)
    assert_not_null(mesh)
    assert_false(light_on.visible)

    E3DRenderingServer.instance_set_lights_state(rid, {"00": true})
    assert_true(light_on.visible)

    E3DRenderingServer.instance_free(rid)
    assert_eq(parent.get_child_count(true), 0)


func test_nodes_instance_rebuilds_on_options_change() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)

    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_NODES)
    E3DRenderingServer.instance_attach_node(rid, parent)
    E3DRenderingServer.instance_build(rid)
    assert_not_null(parent.get_node_or_null(NodePath("light_on00/mesh")))

    E3DRenderingServer.instance_set_options(rid, "", [], ["mesh"], false, [])
    assert_eq(parent.get_child_count(true), 1)
    assert_null(parent.get_node_or_null(NodePath("light_on00/mesh")))
    E3DRenderingServer.instance_free(rid)


func _create_model() -> E3DModel:
    var model: E3DModel = E3DModel.new()
    var light_on: E3DSubModel = E3DSubModel.new()
    light_on.resource_name = "light_on00"
    light_on.submodel_type = E3DSubModel.SUBMODEL_TRANSFORM
    var mesh_submodel: E3DSubModel = E3DSubModel.new()
    mesh_submodel.resource_name = "mesh"
    mesh_submodel.submodel_type = E3DSubModel.SUBMODEL_GL_TRIANGLES
    var mesh: ArrayMesh = ArrayMesh.new()
    mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, BoxMesh.new().get_mesh_arrays())
    mesh_submodel.mesh = mesh
    light_on.submodels = [mesh_submodel]
    model.submodels = [light_on]
    var light_definition: E3DModelLightDefinition = E3DModelLightDefinition.new()
    light_definition.on_submodel_path = NodePath("light_on00")
    model.register_light("00", light_definition)
    return model
