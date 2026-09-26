extends MaszynaGutTest

const EventImporter = preload("res://addons/libmaszyna/importer/maszyna_event_importer.gd")
const NodeImporter = preload("res://addons/libmaszyna/importer/maszyna_node_importer.gd")
const MAX_WAIT:float = 5.0
## Longer than any test runs, so the event stays queued
const NEVER:float = 3600.0
## Fast enough for one frame to pass the cap
const FAST_SPEED:float = 1000.0
## ScenarioEventServer::MAX_FRAME_TIME (Timer.cpp:84)
const MAX_FRAME_TIME:float = 1.0


class RecordingAction extends ScenarioEventAction:
    var runs:Array[RID] = []
    var else_runs:Array[RID] = []
    var activators:Array[RID] = []

    func _run(event:RID, activator:RID) -> void:
        runs.append(event)
        activators.append(activator)

    func _run_else(event:RID, _activator:RID) -> void:
        else_runs.append(event)


class RequeueingAction extends ScenarioEventAction:
    var count:int = 0

    func _run(event:RID, _activator:RID) -> void:
        count += 1
        ScenarioEventServer.event_queue(event)


func test_events_run_in_time_order_and_in_queue_order_on_equal_times() -> void:
    var action:RecordingAction = RecordingAction.new()
    var late:RID = _create_event(action, 0.2)
    var first:RID = _create_event(action, 0.0)
    var second:RID = _create_event(action, 0.0)

    ScenarioEventServer.event_queue(late)
    ScenarioEventServer.event_queue(first)
    ScenarioEventServer.event_queue(second)
    await wait_until(func() -> bool: return action.runs.size() == 3, MAX_WAIT)

    var expected:Array[RID] = [first, second, late]
    assert_eq(action.runs, expected)
    _free_events([late, first, second])


func test_a_queued_event_is_not_queued_again_and_keeps_its_activator() -> void:
    var action:RecordingAction = RecordingAction.new()
    var event:RID = _create_event(action, 0.0)
    # any RID stands in for a vehicle - the server does not look into its activators
    var activator:RID = ScenarioEventServer.memory_create()
    var other_activator:RID = ScenarioEventServer.memory_create()

    assert_true(ScenarioEventServer.event_queue(event, activator))
    assert_false(ScenarioEventServer.event_queue(event, other_activator), "a waiting event should refuse")
    assert_true(ScenarioEventServer.event_is_queued(event))
    await wait_until(func() -> bool: return action.runs.size() == 1, MAX_WAIT)

    var expected:Array[RID] = [activator]
    assert_eq(action.activators, expected)
    assert_false(ScenarioEventServer.event_is_queued(event))
    _free_events([event])
    ScenarioEventServer.memory_free(activator)
    ScenarioEventServer.memory_free(other_activator)


func test_an_event_queueing_itself_runs_once_a_frame() -> void:
    var action:RequeueingAction = RequeueingAction.new()
    var event:RID = _create_event(action, 0.0)

    # the server connected to process_frame when the event was queued, before this awaits it
    ScenarioEventServer.event_queue(event)
    await get_tree().process_frame
    var count:int = action.count
    await get_tree().process_frame

    assert_eq(count, 1, "the pass should not run the event it queued")
    assert_eq(action.count, 2, "the next frame should run it again")
    _free_events([event])


func test_the_condition_chooses_between_the_events_and_the_else_events() -> void:
    var action:RecordingAction = RecordingAction.new()
    var memory:RID = ScenarioEventServer.memory_create()
    var chosen:RID = _create_event(action, 0.0)
    var otherwise:RID = _create_event(action, 0.0)
    var multiple_action:MaszynaLegacyMultipleAction = MaszynaLegacyMultipleAction.new()
    var events:Array[RID] = [chosen]
    var else_events:Array[RID] = [otherwise]
    multiple_action.events = events
    multiple_action.else_events = else_events
    var multiple:RID = _create_event(multiple_action, 0.0)
    var condition:MaszynaLegacyEventCondition = MaszynaLegacyEventCondition.new()
    var memories:Array[RID] = [memory]
    condition.memories = memories
    condition.text = "go*"
    condition.mask = ScenarioEventServer.MEMORY_FIELD_TEXT
    ScenarioEventServer.event_attach_condition(multiple, condition)
    var activator:RID = ScenarioEventServer.memory_create()

    ScenarioEventServer.memory_set_values(memory, "go_ahead", 0.0, 0.0)
    ScenarioEventServer.event_queue(multiple, activator)
    await wait_until(func() -> bool: return action.runs.size() == 1, MAX_WAIT)
    ScenarioEventServer.memory_set_values(memory, "stop", 0.0, 0.0)
    ScenarioEventServer.event_queue(multiple)
    await wait_until(func() -> bool: return action.runs.size() == 2, MAX_WAIT)

    var expected:Array[RID] = [chosen, otherwise]
    assert_eq(action.runs, expected, "a text with * should be compared up to it")
    assert_eq(action.activators[0], activator, "multiple should hand its activator on")
    _free_events([chosen, otherwise, multiple])
    ScenarioEventServer.memory_free(memory)
    ScenarioEventServer.memory_free(activator)


func test_pause_stops_the_time_and_the_speed_scales_it() -> void:
    var event:RID = _create_event(RecordingAction.new(), NEVER)
    ScenarioEventServer.event_queue(event)

    MaszynaRuntime.pause()
    var paused_at:float = ScenarioEventServer.get_time()
    await get_tree().process_frame
    await get_tree().process_frame
    assert_eq(ScenarioEventServer.get_time(), paused_at, "the time should stand while paused")
    MaszynaRuntime.unpause()

    MaszynaRuntime.simulation_speed = FAST_SPEED
    var fast_from:float = ScenarioEventServer.get_time()
    await get_tree().process_frame
    assert_almost_eq(
        ScenarioEventServer.get_time() - fast_from,
        MAX_FRAME_TIME,
        0.001,
        "a frame should advance at most MAX_FRAME_TIME"
    )
    MaszynaRuntime.simulation_speed = 1.0
    _free_events([event])


func test_a_launcher_fires_only_when_its_condition_passes() -> void:
    var event:RID = _create_event(RecordingAction.new(), NEVER)
    var memory:RID = ScenarioEventServer.memory_create()
    var condition:MaszynaLegacyEventCondition = MaszynaLegacyEventCondition.new()
    var memories:Array[RID] = [memory]
    condition.memories = memories
    condition.value1 = 1.0
    condition.mask = ScenarioEventServer.MEMORY_FIELD_VALUE1
    var launcher:RID = ScenarioEventServer.launcher_create()
    ScenarioEventServer.launcher_set_events(launcher, event, RID())
    ScenarioEventServer.launcher_attach_condition(launcher, condition)

    ScenarioEventServer.launcher_fire(launcher)
    assert_false(ScenarioEventServer.event_is_queued(event))
    ScenarioEventServer.memory_set_values(memory, "", 1.0, 0.0)
    ScenarioEventServer.launcher_fire(launcher)
    assert_true(ScenarioEventServer.event_is_queued(event))

    ScenarioEventServer.launcher_free(launcher)
    ScenarioEventServer.memory_free(memory)
    _free_events([event])


func test_a_launcher_fires_when_the_clock_shows_its_time() -> void:
    var clock:float = MaszynaRuntime.time_of_day
    var event:RID = _create_event(RecordingAction.new(), NEVER)
    var launcher:RID = ScenarioEventServer.launcher_create()
    ScenarioEventServer.launcher_set_events(launcher, event, RID())
    ScenarioEventServer.launcher_set_time_of_day(launcher, 10, 50)

    MaszynaRuntime.time_of_day = 10.5
    assert_false(ScenarioEventServer.event_is_queued(event), "10:30 is not its time")
    MaszynaRuntime.time_of_day = 10.0 + 50.5 / 60.0
    assert_true(ScenarioEventServer.event_is_queued(event), "10:50 is")

    ScenarioEventServer.launcher_free(launcher)
    _free_events([event])
    MaszynaRuntime.time_of_day = clock


func test_scenery_memcells_and_value_events() -> void:
    var root:MaszynaIncludeNode = _build_scenery(
        "node -1 0 Cell1 memcell 0 0 0 Start 1 2 none endmemcell "
        + "node -1 0 cell2 memcell 0 0 0 Other 7 8 none endmemcell "
        + "event set_cell updatevalues 0 cell1 Go * 5 endevent "
        + "event add_cell addvalues 0 CELL1 _x 1 * endevent "
        + "event copy_cell copyvalues 0 cell2 cell1 2 endevent",
        []
    )
    var cell1:RID = ScenarioEventServer.memory_get_rid_by_name(&"Cell1")
    var cell2:RID = ScenarioEventServer.memory_get_rid_by_name(&"cell2")
    assert_eq(ScenarioEventServer.memory_get_text(cell1), "Start")

    await _run_event(&"set_cell")
    assert_eq(ScenarioEventServer.memory_get_text(cell1), "Go", "the text keeps its case")
    assert_eq(ScenarioEventServer.memory_get_value1(cell1), 1.0, "* should leave the value")
    assert_eq(ScenarioEventServer.memory_get_value2(cell1), 5.0)

    await _run_event(&"add_cell")
    assert_eq(ScenarioEventServer.memory_get_text(cell1), "Go_x")
    assert_eq(ScenarioEventServer.memory_get_value1(cell1), 2.0)
    assert_eq(ScenarioEventServer.memory_get_value2(cell1), 5.0)

    await _run_event(&"copy_cell")
    assert_eq(ScenarioEventServer.memory_get_text(cell2), "Other", "mask 2 copies only value 1")
    assert_eq(ScenarioEventServer.memory_get_value1(cell2), 2.0)
    assert_eq(ScenarioEventServer.memory_get_value2(cell2), 8.0)
    root.free()


func test_scenery_onstart_and_negative_delay_events_are_queued() -> void:
    var root:MaszynaIncludeNode = _build_scenery(
        "event scenery_onstart updatevalues 0.0 none a 0 0 endevent "
        + "event at_start updatevalues -30 none b 0 0 endevent "
        + "event later updatevalues 30 none c 0 0 endevent",
        []
    )

    assert_true(ScenarioEventServer.event_is_queued(ScenarioEventServer.event_get_rid_by_name(&"scenery_onstart")))
    var at_start:RID = ScenarioEventServer.event_get_rid_by_name(&"at_start")
    assert_true(ScenarioEventServer.event_is_queued(at_start))
    assert_eq(ScenarioEventServer.event_get_delay(at_start), 30.0, "the delay itself is positive")
    assert_false(ScenarioEventServer.event_is_queued(ScenarioEventServer.event_get_rid_by_name(&"later")))
    root.free()


func test_scenery_lights_event_shows_the_aspect() -> void:
    var instance:E3DModelInstance = E3DModelInstance.new()
    instance.model = E3DModel.new()
    add_child_autoqfree(instance)
    var semaphore:RID = SemaphoreServer.semaphore_create(instance.get_e3d_instance())
    SemaphoreServer.semaphore_set_name(semaphore, &"Sem_A")
    var aspects:Dictionary = {&"sem_ligh1": PackedFloat32Array([1.0])}
    SemaphoreServer.semaphore_set_kind(semaphore, MaszynaLegacySemaphoreKindFactory.create_kind(aspects))
    var system:RID = SemaphoreServer.system_create()
    SemaphoreServer.system_attach_delegate(system, MaszynaLegacySemaphoreDelegate.new())
    SemaphoreServer.system_add_semaphore(system, semaphore)
    var model_data:MaszynaModelData = MaszynaModelData.new()
    model_data.name = "Sem_A"
    var models:Array[MaszynaModelData] = [model_data]
    var root:MaszynaIncludeNode = _build_scenery("event sem_a_sem_ligh1 lights 0 sem_a 1 endevent", models)

    await _run_event(&"sem_a_sem_ligh1")

    assert_eq(SemaphoreServer.semaphore_get_aspect(semaphore), &"sem_ligh1")
    assert_eq(SemaphoreServer.semaphore_get_light_state(semaphore, 0), SemaphoreServer.LIGHT_STATE_ON)
    root.free()
    SemaphoreServer.system_free(system)


func test_a_track_event_fires_once_per_entry_in_its_direction() -> void:
    var action:RecordingAction = RecordingAction.new()
    var to_end:RID = _create_event(action, 0.0)
    var to_start:RID = _create_event(action, 0.0)
    var track:RID = ScenarioEventServer.memory_create() # stands in for a track
    var other_track:RID = ScenarioEventServer.memory_create()
    var vehicle:RID = ScenarioEventServer.memory_create() # and for a vehicle
    ScenarioEventServer.track_add_event(track, ScenarioEventServer.TRACK_EVENTALL2, to_end)
    ScenarioEventServer.track_add_event(track, ScenarioEventServer.TRACK_EVENTALL1, to_start)

    RailVehicleServer.vehicle_heading_to_track_end.emit(vehicle, track)
    assert_true(ScenarioEventServer.event_is_queued(to_end))
    await wait_until(func() -> bool: return action.runs.size() == 1, MAX_WAIT)
    RailVehicleServer.vehicle_stopped_on_track.emit(vehicle, track)
    RailVehicleServer.vehicle_heading_to_track_end.emit(vehicle, track)
    assert_false(ScenarioEventServer.event_is_queued(to_end), "once per entry")
    RailVehicleServer.vehicle_heading_to_track_start.emit(vehicle, track)
    assert_true(ScenarioEventServer.event_is_queued(to_start), "the other direction once too")
    RailVehicleServer.vehicle_heading_to_track_end.emit(vehicle, other_track)
    RailVehicleServer.vehicle_heading_to_track_end.emit(vehicle, track)
    assert_true(ScenarioEventServer.event_is_queued(to_end), "a new entry fires again")

    ScenarioEventServer.track_clear_events(track)
    _free_events([to_end, to_start])
    for rid:RID in [track, other_track, vehicle]:
        ScenarioEventServer.memory_free(rid)


func test_a_standing_event_goes_on_while_the_vehicle_stands() -> void:
    var action:RecordingAction = RecordingAction.new()
    var standing:RID = _create_event(action, 0.0)
    var track:RID = ScenarioEventServer.memory_create()
    var vehicle:RID = ScenarioEventServer.memory_create()
    ScenarioEventServer.track_add_event(track, ScenarioEventServer.TRACK_EVENTALL0, standing)

    RailVehicleServer.vehicle_stopped_on_track.emit(vehicle, track)
    await wait_until(func() -> bool: return action.runs.size() >= 2, MAX_WAIT)
    assert_gt(action.runs.size(), 1, "queued again after its run")
    RailVehicleServer.vehicle_heading_to_track_end.emit(vehicle, track)
    await wait_until(func() -> bool: return not ScenarioEventServer.event_is_queued(standing), MAX_WAIT)
    var runs:int = action.runs.size()
    await get_tree().process_frame
    assert_eq(action.runs.size(), runs, "not once the vehicle moves")

    ScenarioEventServer.track_clear_events(track)
    _free_events([standing])
    ScenarioEventServer.memory_free(track)
    ScenarioEventServer.memory_free(vehicle)


func test_scenery_animation_turns_the_submodel() -> void:
    var instance:E3DModelInstance = E3DModelInstance.new()
    var model:E3DModel = E3DModel.new()
    var arm:E3DSubModel = E3DSubModel.new()
    arm.resource_name = "Ramie01"
    arm.submodel_type = E3DSubModel.SUBMODEL_TRANSFORM
    var submodels:Array[E3DSubModel] = [arm]
    model.submodels = submodels
    instance.model = model
    add_child_autoqfree(instance)
    var model_data:MaszynaModelData = MaszynaModelData.new()
    model_data.name = "rog1"
    var models:Array[MaszynaModelData] = [model_data]
    var instances:Dictionary[String, RID] = {"rog1": instance.get_e3d_instance()}
    var root:MaszynaIncludeNode = _build_scenery(
        "event rog1on animation 0 rog1 rotate ramie01 0 0 90 900 endevent", models, instances
    )

    await _run_event(&"rog1on")
    var arm_node:Node3D = instance.find_child("Ramie01", true, false)
    await wait_until(
        func() -> bool: return arm_node.basis.is_equal_approx(Basis(Vector3(0, 0, 1), deg_to_rad(90.0))), MAX_WAIT
    )

    assert_true(arm_node.basis.is_equal_approx(Basis(Vector3(0, 0, 1), deg_to_rad(90.0))), "turned by 90 degrees about z")
    root.free()


func test_scenery_voltage_event_sets_the_power_source() -> void:
    var power_source:RID = TractionPowerServer.power_source_create()
    TractionPowerServer.power_source_set_params(power_source, "Pwr1", 3000.0, 0.0, 0.2, 1000.0, 1.0, 3, 60.0, false)
    var root:MaszynaIncludeNode = MaszynaIncludeNode.new()
    root.autoload = false
    add_child(root)
    var power_source_data:MaszynaPowerSourceData = MaszynaPowerSourceData.new()
    power_source_data.name = "Pwr1"
    var power_sources:Array[MaszynaPowerSourceData] = [power_source_data]
    var context:MaszynaImporterContext = _parse("event keyctrl05 voltage 0.1 pwr1 2400 endevent")
    var tracks:Array[RID] = []
    var models:Array[MaszynaModelData] = []
    var model_rids:Array[RID] = []
    MaszynaLegacyEventFactory.build(
        root, context.events, context.memcells, context.launchers, context.sounds, context.tracks, tracks, models,
        model_rids, power_sources
    )

    await _run_event(&"keyctrl05")

    assert_eq(TractionPowerServer.power_source_get_nominal_voltage(power_source), 2400.0)
    root.free()
    TractionPowerServer.power_source_free(power_source)


func test_shift_and_a_digit_queue_the_keyctrl_event() -> void:
    var keyboard:ScenarioKeyboard = ScenarioKeyboard.new()
    add_child_autoqfree(keyboard)
    var event:RID = _create_event(RecordingAction.new(), NEVER)
    ScenarioEventServer.event_set_name(event, &"keyctrl03")
    var press:InputEventAction = InputEventAction.new()
    press.action = &"scenario_keyctrl_3"
    press.pressed = true

    Input.parse_input_event(press)
    Input.flush_buffered_events()
    await get_tree().process_frame

    assert_true(ScenarioEventServer.event_is_queued(event))
    _free_events([event])


func _create_event(action:ScenarioEventAction, delay:float) -> RID:
    var event:RID = ScenarioEventServer.event_create()
    ScenarioEventServer.event_set_delay(event, delay)
    ScenarioEventServer.event_attach_action(event, action)
    return event


func _free_events(events:Array[RID]) -> void:
    for event:RID in events:
        ScenarioEventServer.event_free(event)


## Parses the scenery text and builds it through MaszynaLegacyEventFactory; the returned include
## frees what was built when it is freed
func _build_scenery(
    text:String, models:Array[MaszynaModelData], model_instances:Dictionary[String, RID] = {}
) -> MaszynaIncludeNode:
    var context:MaszynaImporterContext = _parse(text)
    var root:MaszynaIncludeNode = MaszynaIncludeNode.new()
    root.autoload = false
    add_child(root)
    var model_rids:Array[RID] = []
    for model_data:MaszynaModelData in models:
        model_rids.append(model_instances.get(model_data.name, RID()))
    var track_rids:Array[RID] = []
    var power_sources:Array[MaszynaPowerSourceData] = []
    MaszynaLegacyEventFactory.build(
        root, context.events, context.memcells, context.launchers, context.sounds, context.tracks, track_rids,
        models, model_rids, power_sources
    )
    return root


func _parse(text:String) -> MaszynaImporterContext:
    var parser:MaszynaParser = MaszynaParser.new()
    parser.initialize(text.to_utf8_buffer())
    var context:MaszynaImporterContext = MaszynaImporterContext.new()
    parser.register_handler("event", func(p:MaszynaParser) -> Array: return EventImporter.new().import(p, context))
    parser.register_handler("node", func(p:MaszynaParser) -> Array: return NodeImporter.new().import(p, context))
    parser.parse()
    return context


func _run_event(event_name:StringName) -> void:
    var event:RID = ScenarioEventServer.event_get_rid_by_name(event_name)
    ScenarioEventServer.event_queue(event)
    await wait_until(func() -> bool: return not ScenarioEventServer.event_is_queued(event), MAX_WAIT)
