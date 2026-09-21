extends MaszynaGutTest

## E3DRenderingServer RID API: OPTIMIZED instances create no nodes, NODES instances build
## a node tree under the attached node and follow the lights state.
##
## The lights half covers what a scenery model node declares with `lights`/`lightcolors`: the
## server resolves those modes against the time of day and the light level, exactly as
## TAnimModel::RaPrepare() does (AnimModel.cpp:578-627).

## Full daylight and midday, the state a scenery starts in
const DAY_LIGHT_LEVEL: float = 1.0
const NIGHT_LIGHT_LEVEL: float = 0.1
const MIDDAY: float = 12.0


func after_each() -> void:
    # the time of day and the light level are singleton state shared by every test
    E3DRenderingServer.set_current_time(MIDDAY)
    E3DRenderingServer.set_light_level(DAY_LIGHT_LEVEL)


func test_optimized_instance_creates_no_nodes() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)

    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_OPTIMIZED, E3DRenderingServer.INSTANCE_KIND_STATIC)
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

    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_NODES, E3DRenderingServer.INSTANCE_KIND_STATIC)
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

    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_NODES, E3DRenderingServer.INSTANCE_KIND_STATIC)
    E3DRenderingServer.instance_attach_node(rid, parent)
    E3DRenderingServer.instance_build(rid)
    assert_not_null(parent.get_node_or_null(NodePath("light_on00/mesh")))

    E3DRenderingServer.instance_set_options(rid, "", [], ["mesh"], false, [])
    assert_eq(parent.get_child_count(true), 1)
    assert_null(parent.get_node_or_null(NodePath("light_on00/mesh")))
    E3DRenderingServer.instance_free(rid)


func test_dark_light_follows_the_light_level() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)
    var rid: RID = _create_lit_instance(parent)
    # `lights 3` - the mode every street lamp in the data set declares
    E3DRenderingServer.instance_set_lights_modes(rid, [float(E3DRenderingServer.LIGHT_MODE_DARK)])
    var light_on: Node3D = parent.get_node(NodePath("light_on00"))

    E3DRenderingServer.set_light_level(DAY_LIGHT_LEVEL)
    assert_false(light_on.visible, "unlit in daylight")

    E3DRenderingServer.set_light_level(NIGHT_LIGHT_LEVEL)
    assert_true(light_on.visible, "lit once it gets dark")

    E3DRenderingServer.instance_free(rid)


func test_dark_light_fraction_is_its_own_threshold() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)
    var rid: RID = _create_lit_instance(parent)
    # `lights 3.4` - comes on below a light level of 0.4 instead of the default 0.325
    E3DRenderingServer.instance_set_lights_modes(rid, [3.4])
    var light_on: Node3D = parent.get_node(NodePath("light_on00"))

    E3DRenderingServer.set_light_level(0.5)
    assert_false(light_on.visible, "still above its own threshold")

    E3DRenderingServer.set_light_level(0.35)
    assert_true(light_on.visible, "below its own threshold, but above the default one")

    E3DRenderingServer.instance_free(rid)


func test_home_light_is_forced_off_late_at_night() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)
    var rid: RID = _create_lit_instance(parent)
    E3DRenderingServer.instance_set_lights_modes(rid, [float(E3DRenderingServer.LIGHT_MODE_HOME)])
    var light_on: Node3D = parent.get_node(NodePath("light_on00"))

    E3DRenderingServer.set_light_level(NIGHT_LIGHT_LEVEL)
    E3DRenderingServer.set_current_time(22.0)
    assert_true(light_on.visible, "a lit window in the evening")

    E3DRenderingServer.set_current_time(3.0)
    assert_false(light_on.visible, "the same window is dark between 1:00 and 5:00")

    E3DRenderingServer.set_current_time(6.0)
    assert_true(light_on.visible, "and lit again before dawn")

    E3DRenderingServer.instance_free(rid)


func test_lights_state_overrides_the_declared_mode() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)
    var rid: RID = _create_lit_instance(parent)
    E3DRenderingServer.instance_set_lights_modes(rid, [float(E3DRenderingServer.LIGHT_MODE_OFF)])
    var light_on: Node3D = parent.get_node(NodePath("light_on00"))
    assert_false(light_on.visible)

    E3DRenderingServer.instance_set_lights_state(rid, {"00": true})
    assert_true(light_on.visible, "a manual state wins over the declared mode")

    E3DRenderingServer.instance_free(rid)


func test_emission_light_handle_switches_the_submodels() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)
    var rid: RID = _create_lit_instance(parent)
    var light_on: Node3D = parent.get_node(NodePath("light_on00"))

    var light: RID = E3DRenderingServer.emission_light_create(rid, "00")
    assert_true(light.is_valid())

    E3DRenderingServer.light_enable(light)
    assert_true(light_on.visible)

    E3DRenderingServer.light_disable(light)
    assert_false(light_on.visible)

    E3DRenderingServer.light_free(light)
    E3DRenderingServer.instance_free(rid)


func test_instance_free_releases_its_lights() -> void:
    var parent: Node3D = Node3D.new()
    add_child_autoqfree(parent)
    var rid: RID = _create_lit_instance(parent)
    var before: int = E3DRenderingServer.get_light_statistics()["total"]

    E3DRenderingServer.emission_light_create(rid, "00")
    assert_eq(E3DRenderingServer.get_light_statistics()["total"], before + 1)

    E3DRenderingServer.instance_free(rid)
    assert_eq(
        E3DRenderingServer.get_light_statistics()["total"], before, "freeing the instance frees its lights"
    )


func _create_lit_instance(parent: Node3D) -> RID:
    var rid: RID = E3DRenderingServer.instance_create(_create_model(), E3DRenderingServer.INSTANCER_NODES, E3DRenderingServer.INSTANCE_KIND_STATIC)
    E3DRenderingServer.instance_attach_node(rid, parent)
    E3DRenderingServer.instance_build(rid)
    return rid


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
