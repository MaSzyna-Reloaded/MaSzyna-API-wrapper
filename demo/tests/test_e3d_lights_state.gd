extends MaszynaGutTest

var instancer: E3DInstancer


func before_each() -> void:
    instancer = load("res://addons/libmaszyna/e3d/e3d_nodes_instancer.gd").new()
    add_child_autoqfree(instancer)


func test_model_instance_synchronizes_lights_state_with_model() -> void:
    var instance: E3DModelInstance = load("res://addons/libmaszyna/e3d/e3d_model_instance.gd").new()
    var model: E3DModel = _create_model_with_lights(["front", "rear"])
    add_child_autoqfree(instance)

    instance._model = model
    instance.lights_state = {
        "front": true,
        "stale": true,
    }

    assert_eq(instance.lights_state.size(), 2, "Only lights declared by the model should remain in state")
    assert_true(instance.lights_state["front"], "Existing state value should be preserved")
    assert_false(instance.lights_state["rear"], "Missing light should be initialized as disabled")


func test_nodes_instancer_maps_light_on_prefix_family() -> void:
    var target_node: E3DModelInstance = load("res://addons/libmaszyna/e3d/e3d_model_instance.gd").new()
    var model: E3DModel = E3DModel.new()
    var light_on: E3DSubModel = _create_transform_submodel("light_on00", false)
    var light_off: E3DSubModel = _create_transform_submodel("light_off00", true)
    var spotlight: E3DSubModel = _create_spotlight_submodel("spotlight", false)
    var light_definition: E3DModelLightDefinition = E3DModelLightDefinition.new()
    add_child_autoqfree(target_node)

    light_on.submodels = [spotlight]
    model.submodels = [light_on, light_off]
    light_definition.on_submodel_path = NodePath("light_on00")
    light_definition.off_submodel_path = NodePath("light_off00")
    model.register_light("00", light_definition)

    target_node._model = model
    target_node.lights_state = {"00": false}

    instancer.instantiate(target_node, model, false)

    var light_on_node: Node3D = target_node.get_node(NodePath("light_on00"))
    var light_off_node: Node3D = target_node.get_node(NodePath("light_off00"))
    var spotlight_node: SpotLight3D = target_node.get_node(NodePath("light_on00/spotlight"))

    assert_false(light_on_node.visible, "On node should start hidden when light is disabled")
    assert_true(light_off_node.visible, "Off node should start visible when light is disabled")
    assert_false(spotlight_node.visible, "Spotlight should start hidden when light is disabled")

    target_node.lights_state = {"00": true}
    instancer.sync_lights(target_node)

    assert_true(light_on_node.visible, "On node should become visible when light is enabled")
    assert_false(light_off_node.visible, "Off node should become hidden when light is enabled")
    assert_true(spotlight_node.visible, "Spotlight should become visible when light is enabled")


func _create_model_with_lights(light_names: Array[String]) -> E3DModel:
    var model: E3DModel = E3DModel.new()

    for light_name: String in light_names:
        model.register_light(light_name, E3DModelLightDefinition.new())

    return model


func _create_transform_submodel(name: String, visible: bool) -> E3DSubModel:
    var submodel: E3DSubModel = E3DSubModel.new()
    submodel.resource_name = name
    submodel.submodel_type = E3DSubModel.SUBMODEL_TRANSFORM
    submodel.visible = visible
    return submodel


func _create_spotlight_submodel(name: String, visible: bool) -> E3DSubModel:
    var submodel: E3DSubModel = E3DSubModel.new()
    submodel.resource_name = name
    submodel.submodel_type = E3DSubModel.SUBMODEL_FREE_SPOTLIGHT
    submodel.visible = visible
    return submodel
