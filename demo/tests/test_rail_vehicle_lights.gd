extends MaszynaGutTest


func test_model_instance_path_updates_lights_and_preserves_shared_state() -> void:
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    var first_model: E3DModelInstance = E3DModelInstance.new()
    var second_model: E3DModelInstance = E3DModelInstance.new()

    first_model.name = "FirstModel"
    second_model.name = "SecondModel"
    vehicle.add_child(first_model)
    vehicle.add_child(second_model)
    add_child_autoqfree(vehicle)

    first_model._model = _create_model_with_lights(["front", "rear"])
    first_model.lights_state = {
        "front": true,
        "rear": false,
    }
    first_model._e3d_loaded = true

    second_model._model = _create_model_with_lights(["front", "cab"])
    second_model.lights_state = {
        "front": false,
        "cab": false,
    }
    second_model._e3d_loaded = true

    vehicle.model_instance_path = NodePath("FirstModel")
    await wait_idle_frames(2)

    assert_eq(vehicle.lights, {
        "front": true,
        "rear": false,
    }, "Vehicle should copy lights from the selected model")

    vehicle.model_instance_path = NodePath("SecondModel")
    await wait_idle_frames(2)

    assert_eq(vehicle.lights, {
        "front": true,
        "cab": false,
    }, "Shared lights should preserve state and new lights should default to model state")
    assert_eq(second_model.lights_state, vehicle.lights, "Selected model should receive reconciled light state")


func test_lights_property_updates_selected_model() -> void:
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    var model_node: E3DModelInstance = E3DModelInstance.new()

    model_node.name = "Model"
    vehicle.add_child(model_node)
    add_child_autoqfree(vehicle)

    model_node._model = _create_model_with_lights(["front"])
    model_node.lights_state = {"front": false}
    model_node._e3d_loaded = true

    vehicle.model_instance_path = NodePath("Model")
    await wait_idle_frames(2)

    vehicle.lights = {"front": true}
    await wait_idle_frames(2)

    assert_eq(model_node.lights_state, {"front": true}, "Vehicle lights should be applied to the selected model")


func test_model_reload_reapplies_vehicle_lights() -> void:
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    var model_node: E3DModelInstance = E3DModelInstance.new()

    model_node.name = "Model"
    vehicle.add_child(model_node)
    add_child_autoqfree(vehicle)

    model_node._model = _create_model_with_lights(["front"])
    model_node.lights_state = {"front": false}
    model_node._e3d_loaded = true

    vehicle.model_instance_path = NodePath("Model")
    vehicle.lights = {"front": true}
    await wait_idle_frames(2)

    model_node.lights_state = {"front": false}
    model_node.e3d_loaded.emit()
    await wait_idle_frames(2)

    assert_eq(model_node.lights_state, {"front": true}, "Vehicle lights should be reapplied after model reload")


func _create_model_with_lights(light_names: Array[String]) -> E3DModel:
    var model: E3DModel = E3DModel.new()

    for light_name: String in light_names:
        model.register_light(light_name, E3DModelLightDefinition.new())

    return model
