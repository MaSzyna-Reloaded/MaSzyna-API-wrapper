extends MaszynaGutTest

const EventImporter = preload("res://addons/libmaszyna/importer/maszyna_event_importer.gd")


class RecordingDelegate extends SemaphoreSystemDelegate:
    var added: Array[RID] = []
    var events: Array[StringName] = []

    func _semaphore_added(_system: RID, semaphore: RID) -> void:
        added.append(semaphore)

    func _handle_event(system: RID, event: StringName, _arguments: Dictionary) -> void:
        events.append(event)
        for semaphore: RID in SemaphoreServer.system_get_semaphores(system):
            SemaphoreServer.semaphore_light_enable(semaphore, 0)


func test_semaphore_node_registers_the_model_under_its_name() -> void:
    var model: E3DModelInstance = _create_model_instance()
    var semaphore_node: SemaphoreNode = _create_semaphore_node(&"test_registered", model)

    var semaphore: RID = SemaphoreServer.semaphore_get_rid_by_name(&"test_registered")
    assert_true(semaphore.is_valid(), "the model's semaphore should be registered under the node's name")
    assert_eq(semaphore_node.get_semaphore(), semaphore)


func test_enabling_a_light_shows_its_submodel_and_reports_the_change() -> void:
    var model: E3DModelInstance = _create_model_instance()
    var semaphore_node: SemaphoreNode = _create_semaphore_node(&"test_enable", model)
    watch_signals(semaphore_node)

    semaphore_node.enable_light(0)

    assert_true(model.get_node(NodePath("light_on00")).visible, "light_on00 should be shown")
    assert_false(model.get_node(NodePath("light_off00")).visible, "light_off00 should be hidden")
    assert_eq(semaphore_node.get_light_state(0), SemaphoreServer.LIGHT_STATE_ON)
    assert_signal_emitted_with_parameters(semaphore_node, "light_state_changed", [0, SemaphoreServer.LIGHT_STATE_ON])


func test_a_blinking_light_goes_on_and_off() -> void:
    var model: E3DModelInstance = _create_model_instance()
    var semaphore_node: SemaphoreNode = _create_semaphore_node(&"test_blink", model)
    var light_on: Node3D = model.get_node(NodePath("light_on00"))

    semaphore_node.blink_light(0, 0.05, 0.05, 0.0)

    var seen_on: bool = false
    var seen_off: bool = false
    for i: int in range(30):
        await wait_idle_frames(1)
        seen_on = seen_on or light_on.visible
        seen_off = seen_off or not light_on.visible
        if seen_on and seen_off:
            break
    assert_eq(semaphore_node.get_light_state(0), SemaphoreServer.LIGHT_STATE_BLINKING)
    assert_true(seen_on and seen_off, "a blinking light should be seen both on and off")


func test_a_system_delegate_receives_its_semaphores_and_events() -> void:
    var model: E3DModelInstance = _create_model_instance()
    _create_semaphore_node(&"test_system", model)
    var semaphore: RID = SemaphoreServer.semaphore_get_rid_by_name(&"test_system")
    var delegate: RecordingDelegate = RecordingDelegate.new()
    var system: RID = SemaphoreServer.system_create()
    SemaphoreServer.system_add_semaphore(system, semaphore)

    SemaphoreServer.system_attach_delegate(system, delegate)
    SemaphoreServer.system_send_event(system, &"proceed", {})

    assert_eq(delegate.added, [semaphore] as Array[RID], "an attached delegate learns the semaphores held")
    assert_eq(delegate.events, [&"proceed"] as Array[StringName])
    assert_eq(SemaphoreServer.semaphore_get_light_state(semaphore, 0), SemaphoreServer.LIGHT_STATE_ON)
    SemaphoreServer.system_free(system)


func test_freeing_the_model_removes_its_semaphore_from_the_system() -> void:
    var model: E3DModelInstance = _create_model_instance()
    _create_semaphore_node(&"test_freed", model)
    var system: RID = SemaphoreServer.system_create()
    SemaphoreServer.system_add_semaphore(system, SemaphoreServer.semaphore_get_rid_by_name(&"test_freed"))

    remove_child(model)
    model.free()

    assert_eq(SemaphoreServer.system_get_semaphores(system).size(), 0, "the semaphore should leave its system")
    assert_false(SemaphoreServer.semaphore_get_rid_by_name(&"test_freed").is_valid())
    SemaphoreServer.system_free(system)


func test_a_reloaded_model_is_registered_again_and_picked_up_by_its_system_node() -> void:
    var model: E3DModelInstance = _create_model_instance()
    var semaphore_node: SemaphoreNode = _create_semaphore_node(&"test_reload", model)
    var system_node: SemaphoreSystemNode = SemaphoreSystemNode.new()
    system_node.semaphore_names = PackedStringArray(["test_reload"])
    add_child_autoqfree(system_node)
    var before: RID = semaphore_node.get_semaphore()

    model.reload()

    var after: RID = SemaphoreServer.semaphore_get_rid_by_name(&"test_reload")
    assert_true(after.is_valid(), "the reloaded model should be registered again")
    assert_ne(after, before, "the new instance should have a new semaphore")
    assert_eq(semaphore_node.get_semaphore(), after)
    assert_eq(SemaphoreServer.semaphore_get_system(after), system_node.get_system())


func test_a_semaphore_node_attaches_to_a_semaphore_registered_later() -> void:
    var semaphore_node: SemaphoreNode = SemaphoreNode.new()
    semaphore_node.semaphore_name = &"test_late"
    add_child_autoqfree(semaphore_node)
    assert_false(semaphore_node.get_semaphore().is_valid())

    var model: E3DModelInstance = _create_model_instance()
    var semaphore: RID = SemaphoreServer.semaphore_create(model.get_e3d_instance())
    SemaphoreServer.semaphore_set_name(semaphore, &"test_late")

    assert_eq(semaphore_node.get_semaphore(), semaphore)


func test_legacy_kind_turns_lights_events_into_aspects() -> void:
    var model: E3DModelInstance = _create_model_instance()
    _create_semaphore_node(&"test_legacy", model)
    var semaphore: RID = SemaphoreServer.semaphore_get_rid_by_name(&"test_legacy")
    var aspects: Dictionary = {
        &"sem_ligh1": PackedFloat32Array([1.35, 1.0]),
        &"sem_ligh2": PackedFloat32Array([-1.0, 0.0]),
    }
    SemaphoreServer.semaphore_set_kind(semaphore, MaszynaLegacySemaphoreKindFactory.create_kind(aspects))
    var system: RID = SemaphoreServer.system_create()
    SemaphoreServer.system_attach_delegate(system, MaszynaLegacySemaphoreDelegate.new())
    SemaphoreServer.system_add_semaphore(system, semaphore)

    SemaphoreServer.system_send_event(system, &"lights", {"semaphore": semaphore, "aspect": &"sem_ligh1"})
    assert_eq(SemaphoreServer.semaphore_get_light_state(semaphore, 0), SemaphoreServer.LIGHT_STATE_BLINKING)
    assert_eq(SemaphoreServer.semaphore_get_light_state(semaphore, 1), SemaphoreServer.LIGHT_STATE_ON)

    SemaphoreServer.system_send_event(system, &"lights", {"semaphore": semaphore, "aspect": &"sem_ligh2"})
    assert_eq(
        SemaphoreServer.semaphore_get_light_state(semaphore, 0),
        SemaphoreServer.LIGHT_STATE_BLINKING,
        "-1 should leave the light as it is"
    )
    assert_eq(SemaphoreServer.semaphore_get_light_state(semaphore, 1), SemaphoreServer.LIGHT_STATE_OFF)
    assert_eq(SemaphoreServer.semaphore_get_aspect(semaphore), &"sem_ligh2")
    SemaphoreServer.system_free(system)


func test_lights_events_of_an_include_give_its_copies_one_kind() -> void:
    var parser: MaszynaParser = MaszynaParser.new()
    parser.initialize((
        "event Sem_A_sem_ligh1 lights 0.0 sem_a 0 1 0 endevent "
        + "event sem_a_sem_ligh3o lights 0.0 sem_a|none 2 0 0 endevent "
        + "event sem_b_sem_ligh1 lights 0.0 sem_b 0 1 0 endevent "
        + "event sem_b_sem_ligh3o lights 0.0 sem_b 2 0 0 endevent "
        + "event sem_a_info multiple 0 none sem_a_sem_ligh1 endevent"
    ).to_utf8_buffer())
    var context: MaszynaImporterContext = MaszynaImporterContext.new()
    parser.register_handler("event", func(p: MaszynaParser) -> Array: return EventImporter.new().import(p, context))
    parser.parse()
    var models: Array[MaszynaModelData] = [
        _create_model_data("sem_a", PackedFloat32Array([0, 0, 1])),
        _create_model_data("sem_b", PackedFloat32Array()),
        _create_model_data("lamp", PackedFloat32Array([3])),
        _create_model_data("house", PackedFloat32Array()),
    ]

    SceneryInstancer.assign_semaphore_kinds(models, context.light_events)

    assert_eq(context.light_events.size(), 4, "only lights events should be kept")
    var kind: SemaphoreKind = models[0].semaphore_kind
    assert_eq(kind.get_aspect_names(), PackedStringArray(["sem_ligh1", "sem_ligh3o"]))
    assert_eq(kind.get_aspect(&"sem_ligh3o").lights, PackedInt32Array([SemaphoreAspect.LIGHT_BLINK, 0, 0]))
    assert_same(models[1].semaphore_kind, kind, "the copies of one include should share their kind")
    assert_eq(models[2].semaphore_kind, SceneryInstancer.GENERIC_SEMAPHORE_KIND, "a lit model no event reaches")
    assert_null(models[3].semaphore_kind, "a model neither lit nor aimed at is no semaphore")


func test_the_light_count_comes_from_the_model() -> void:
    var model: E3DModelInstance = _create_model_instance()
    var semaphore_node: SemaphoreNode = _create_semaphore_node(&"test_light_count", model)

    assert_eq(SemaphoreServer.semaphore_get_light_count(semaphore_node.get_semaphore()), 2)
    assert_eq(semaphore_node.light_count, 2)


func test_an_aspect_of_the_kind_lights_its_lights() -> void:
    var kind: SemaphoreKind = SemaphoreKind.new()
    var aspects: Dictionary[StringName, SemaphoreAspect] = {
        &"stop": _create_aspect(PackedInt32Array([SemaphoreAspect.LIGHT_OFF, SemaphoreAspect.LIGHT_ON])),
        &"proceed": _create_aspect(PackedInt32Array([SemaphoreAspect.LIGHT_BLINK, SemaphoreAspect.LIGHT_OFF])),
    }
    kind.aspects = aspects
    var semaphore_node: SemaphoreNode = SemaphoreNode.new()
    semaphore_node.semaphore_name = &"test_aspect"
    semaphore_node.kind = kind
    semaphore_node.set(&"aspect", &"stop")
    semaphore_node.model = _create_model_instance()
    add_child_autoqfree(semaphore_node)
    var semaphore: RID = semaphore_node.get_semaphore()
    assert_eq(SemaphoreServer.semaphore_get_aspects(semaphore), PackedStringArray(["stop", "proceed"]))
    assert_eq(SemaphoreServer.semaphore_get_aspect(semaphore), &"stop", "the scene's aspect should be shown")
    assert_eq(semaphore_node.get(&"light_1_state"), SemaphoreServer.LIGHT_STATE_ON)
    watch_signals(semaphore_node)

    SemaphoreServer.semaphore_set_aspect(semaphore, &"proceed")

    assert_eq(semaphore_node.get(&"aspect"), &"proceed", "the aspect property should read the server")
    assert_eq(semaphore_node.get_light_state(0), SemaphoreServer.LIGHT_STATE_BLINKING)
    assert_eq(semaphore_node.get_light_state(1), SemaphoreServer.LIGHT_STATE_OFF)
    assert_signal_emitted_with_parameters(semaphore_node, "aspect_changed", [&"proceed"])


func _create_aspect(lights: PackedInt32Array) -> SemaphoreAspect:
    var aspect: SemaphoreAspect = SemaphoreAspect.new()
    aspect.lights = lights
    return aspect


func _create_model_data(model_name: String, lights: PackedFloat32Array) -> MaszynaModelData:
    var model_data: MaszynaModelData = MaszynaModelData.new()
    model_data.name = model_name
    model_data.lights = lights
    return model_data


func _create_model_instance() -> E3DModelInstance:
    var instance: E3DModelInstance = E3DModelInstance.new()
    var model: E3DModel = E3DModel.new()
    var submodels: Array[E3DSubModel] = []
    for light_name: String in ["00", "01"]:
        var light_on: E3DSubModel = _create_transform_submodel("light_on" + light_name, false)
        var light_off: E3DSubModel = _create_transform_submodel("light_off" + light_name, true)
        var definition: E3DModelLightDefinition = E3DModelLightDefinition.new()
        definition.on_submodel_path = NodePath(light_on.resource_name)
        definition.off_submodel_path = NodePath(light_off.resource_name)
        model.register_light(light_name, definition)
        submodels.append(light_on)
        submodels.append(light_off)
    model.submodels = submodels
    instance.model = model
    add_child_autoqfree(instance)
    return instance


func _create_semaphore_node(semaphore_name: StringName, model: E3DModelInstance) -> SemaphoreNode:
    var semaphore_node: SemaphoreNode = SemaphoreNode.new()
    semaphore_node.semaphore_name = semaphore_name
    semaphore_node.model = model
    add_child_autoqfree(semaphore_node)
    return semaphore_node


func _create_transform_submodel(submodel_name: String, visible: bool) -> E3DSubModel:
    var submodel: E3DSubModel = E3DSubModel.new()
    submodel.resource_name = submodel_name
    submodel.submodel_type = E3DSubModel.SUBMODEL_TRANSFORM
    submodel.visible = visible
    return submodel
