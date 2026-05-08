extends MaszynaGutTest

var created_tracks: Array[RID] = []
var created_vehicles: Array[RailVehicle3D] = []
var created_controllers: Array[TrainController] = []


func after_each() -> void:
    for vehicle: RailVehicle3D in created_vehicles:
        if is_instance_valid(vehicle):
            if vehicle.get_parent():
                vehicle.get_parent().remove_child(vehicle)
            vehicle.queue_free()
    created_vehicles.clear()

    for controller: TrainController in created_controllers:
        if is_instance_valid(controller):
            if controller.get_parent():
                controller.get_parent().remove_child(controller)
            controller.queue_free()
    created_controllers.clear()

    for track_rid: RID in created_tracks:
        if TrackManager.track_exists(track_rid):
            TrackManager.track_free(track_rid)
    created_tracks.clear()
    TrackManager.topology_rebuild()


func test_start_track_name_initializes_and_clamps_offset() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    TrackManager.topology_rebuild()

    var fixture: Dictionary = await _create_vehicle("start", 15.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    _assert_vector_eq(controller.get_world_position(), _rail_position(10.0, 0.0), "controller should be placed at track end")
    _assert_vector_eq(vehicle.global_position, _rail_position(10.0, 0.0), "vehicle should be placed at track end")


func test_start_track_name_retries_after_tracks_changed_when_track_is_added_later() -> void:
    var fixture: Dictionary = await _create_vehicle("late_track", 4.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    _assert_vector_eq(controller.get_world_position(), Vector3.ZERO, "controller should stay at origin before the named track exists")
    _assert_vector_eq(vehicle.global_position, Vector3.ZERO, "vehicle should stay at origin before the named track exists")

    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "late_track"
    )
    TrackManager.topology_rebuild()
    await wait_idle_frames(2)

    _assert_vector_eq(controller.get_world_position(), _rail_position(4.0, 0.0), "controller should move after the missing track appears")
    _assert_vector_eq(vehicle.global_position, _rail_position(4.0, 0.0), "vehicle should move after the missing track appears")


func test_move_on_track_moves_forward_and_backward_on_current_track() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 2.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(3.0)
    _assert_vector_eq(controller.get_world_position(), _rail_position(5.0, 0.0), "positive movement should update controller position")
    _assert_vector_eq(vehicle.global_position, _rail_position(5.0, 0.0), "positive movement should update position")

    vehicle.move_on_track(-4.0)
    _assert_vector_eq(controller.get_world_position(), _rail_position(1.0, 0.0), "negative movement should update controller position")
    _assert_vector_eq(vehicle.global_position, _rail_position(1.0, 0.0), "negative movement should update position")


func test_move_on_track_crosses_connected_tracks_and_clamps_at_dead_end() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(5.0)
    _assert_vector_eq(controller.get_world_position(), _rail_position(13.0, 0.0), "movement should continue on next track")
    _assert_vector_eq(vehicle.global_position, _rail_position(13.0, 0.0), "position should continue on next track")

    vehicle.move_on_track(20.0)
    _assert_vector_eq(controller.get_world_position(), _rail_position(20.0, 0.0), "movement should clamp at graph end")
    _assert_vector_eq(vehicle.global_position, _rail_position(20.0, 0.0), "dead-end clamp should place vehicle at endpoint")


func test_move_on_track_updates_direction_when_entering_track_end() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    _register_track(_curve(Vector3(20.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(5.0)

    _assert_vector_eq(_vehicle_forward(vehicle), Vector3.LEFT, "vehicle should follow reversed track orientation")
    _assert_vector_eq(vehicle.global_position, _rail_position(13.0, 0.0), "vehicle should continue through reversed track")


func test_move_on_track_uses_switch_common_route() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    _register_track(_curve(Vector3(10.0, 0.0, 10.0), Vector3(20.0, 0.0, 20.0)))
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(15.0)

    _assert_vector_eq(controller.get_world_position(), _rail_position(13.0, 0.0), "common switch route should continue on main track")
    _assert_vector_eq(vehicle.global_position, _rail_position(13.0, 0.0), "common switch route should use curve1")


func test_move_on_track_uses_switch_diverging_route() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    _register_track(_curve(Vector3(10.0, 0.0, 10.0), Vector3(20.0, 0.0, 20.0)))
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(19.1421)

    _assert_vector_eq(controller.get_world_position(), _rail_position(12.1213, 12.1213), "diverging route should continue after switch curve length")
    _assert_vector_eq(vehicle.global_position, _rail_position(12.1213, 12.1213), "diverging route should use curve2")


func test_move_on_track_forces_switch_diverging_when_entering_from_diverging_branch() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var start_offset: float = 12.0
    var move_distance: float = 5.0
    var distance_on_switch: float = move_distance - (TrackManager.track_get_length(start_rid) - start_offset)
    var fixture: Dictionary = await _create_vehicle("start", start_offset)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(move_distance)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_DIVERGING)
    _assert_vector_eq(
        controller.get_world_position(),
        _track_position(switch_rid, TrackManager.track_get_length(switch_rid) - distance_on_switch),
        "diverging branch entry should force diverging switch route"
    )


func test_move_on_track_does_not_force_switch_before_entering_from_diverging_track() -> void:
    _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 14.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(0.1)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)


func test_move_on_track_does_not_force_switch_when_approaching_diverging_endpoint() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle(
        "start",
        TrackManager.track_get_length(start_rid) - 0.1
    )
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(0.05)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)


func test_move_on_track_forces_switch_common_when_entering_from_straight_branch() -> void:
    _register_track(
        _curve(Vector3(20.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(5.0)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)
    _assert_vector_eq(
        controller.get_world_position(),
        _track_position(switch_rid, TrackManager.track_get_length(switch_rid) - 3.0),
        "straight branch entry should force common switch route"
    )


func test_switch_change_does_not_move_vehicle_already_on_switch() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(5.0)
    var position_before_switch_change: Vector3 = controller.get_world_position()

    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    vehicle._process(0.0)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)
    _assert_vector_eq(
        controller.get_world_position(),
        position_before_switch_change,
        "vehicle already on switch should stay on its current branch after switch state change"
    )
    _assert_vector_eq(
        vehicle.global_position,
        position_before_switch_change,
        "vehicle transform should not jump to another switch branch"
    )


func test_switch_change_then_reverse_keeps_vehicle_on_occupied_diverging_branch() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle("start", 8.0)
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(5.0)

    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    vehicle.move_on_track(-1.0)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)
    _assert_vector_eq(
        controller.get_world_position(),
        _switch_track_position(switch_rid, 2.0, TrackManager.SwitchTrack.TRACK_DIVERGING),
        "reversing after a switch change should keep the vehicle on the occupied diverging branch"
    )
    _assert_vector_eq(
        vehicle.global_position,
        _switch_track_position(switch_rid, 2.0, TrackManager.SwitchTrack.TRACK_DIVERGING),
        "vehicle transform should stay on the occupied diverging branch while reversing"
    )


func test_reversing_from_diverging_track_forces_switch_at_blade_boundary() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var start_length: float = TrackManager.track_get_length(start_rid)
    var boundary_margin: float = 0.02
    var fixture: Dictionary = await _create_vehicle(
        "start",
        start_length - boundary_margin,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    vehicle._process(0.0)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)

    vehicle.move_on_track(-0.01)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_COMMON)

    vehicle.move_on_track(-0.02)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_DIVERGING)


func test_move_on_track_forces_demo3d_second_switch_when_entering_from_diverging2() -> void:
    var topology: Dictionary = _register_demo3d_track_topology()
    var second_switch_rid: RID = topology["second_switch_rid"]
    TrackManager.switch_set_active_track(second_switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle(
        "diverging2",
        TrackManager.track_get_length(TrackManager.track_get_rid_by_name("diverging2")) - 1.0
    )
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.move_on_track(2.0)

    assert_eq(TrackManager.switch_get_active_track(second_switch_rid), TrackManager.SwitchTrack.TRACK_DIVERGING)


func test_move_on_track_continues_after_entering_demo3d_second_switch() -> void:
    var topology: Dictionary = _register_demo3d_track_topology()
    var first_switch_rid: RID = topology["first_switch_rid"]
    var second_switch_rid: RID = topology["second_switch_rid"]
    TrackManager.switch_set_active_track(first_switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.switch_set_active_track(second_switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle(
        "start",
        12.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]
    var second_switch_offset: float = 2.0
    var distance_to_second_switch: float = (
        12.0 + TrackManager.track_get_length(first_switch_rid) + second_switch_offset
    )

    vehicle.move_on_track(distance_to_second_switch)

    var second_switch_position: Vector3 = _track_position(second_switch_rid, second_switch_offset)
    _assert_vector_eq(
        controller.get_world_position(),
        second_switch_position,
        "controller should enter the second demo3d switch"
    )

    var position_before_followup_move: Vector3 = controller.get_world_position()
    vehicle.move_on_track(10.0)

    assert_true(
        controller.get_world_position().distance_to(position_before_followup_move) > 1.0,
        "controller should keep moving after entering the second demo3d switch"
    )


func test_track_transform_applies_roll_and_direction_rotation() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0), 0.0, 10.0),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle(
        "start",
        5.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var aligned_forward: Vector3 = -vehicle.global_basis.z.normalized()
    var aligned_up: Vector3 = vehicle.global_basis.y.normalized()

    vehicle.set("start_direction", TrackManager.Direction.DIRECTION_REVERSED)
    vehicle._process_dirty()
    var opposite_forward: Vector3 = -vehicle.global_basis.z.normalized()

    _assert_vector_eq(aligned_forward, Vector3.RIGHT, "aligned vehicle forward should follow track tangent")
    assert_true(aligned_up.distance_to(Vector3.UP) > 0.01, "roll should tilt vehicle up vector")
    _assert_vector_eq(opposite_forward, Vector3.LEFT, "opposite direction should rotate vehicle by 180 degrees")


func test_start_track_properties_reapply_vehicle_track() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "next"
    )
    TrackManager.topology_rebuild()
    var fixture: Dictionary = await _create_vehicle(
        "start",
        2.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var vehicle: RailVehicle3D = fixture["vehicle"]
    var controller: TrainController = fixture["controller"]

    vehicle.start_track_offset = 6.0
    await wait_idle_frames(2)
    _assert_vector_eq(controller.get_world_position(), _rail_position(6.0, 0.0), "offset change should reapply placement")

    vehicle.set("start_direction", TrackManager.Direction.DIRECTION_REVERSED)
    await wait_idle_frames(2)
    _assert_vector_eq(_vehicle_forward(vehicle), Vector3.LEFT, "direction change should reapply orientation")

    vehicle.start_track_name = "next"
    await wait_idle_frames(2)
    _assert_vector_eq(controller.get_world_position(), _rail_position(16.0, 0.0), "track name change should reapply placement on the new track")


func _create_vehicle(
    track_name: String,
    offset: float,
    direction: TrackManager.Direction = TrackManager.Direction.DIRECTION_REVERSED,
) -> Dictionary:
    var controller: TrainController = _create_controller()
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = track_name
    vehicle.start_track_offset = offset
    vehicle.set("start_direction", direction)
    add_child(vehicle)
    vehicle.controller_path = vehicle.get_path_to(controller)
    created_vehicles.append(vehicle)
    await wait_idle_frames(2)
    return {
        "vehicle": vehicle,
        "controller": controller,
    }


func _create_controller() -> TrainController:
    var controller: TrainController = TrainController.new()
    controller.name = "Controller%d" % created_controllers.size()
    controller.train_id = "test_train_%d" % created_controllers.size()
    controller.type_name = "test"
    add_child(controller)
    created_controllers.append(controller)
    return controller


func _register_track(
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve = null,
    type: int = TrackManager.TrackType.TRACK_NORMAL,
    name: String = "",
) -> RID:
    var track_rid: RID = TrackManager.track_create()
    created_tracks.append(track_rid)
    TrackManager.track_update_curves(track_rid, curve1, curve2)
    TrackManager.track_update(track_rid, type, name, 1.435)
    return track_rid


func _register_demo3d_track_topology() -> Dictionary:
    _register_track(_demo_curve(Vector3(-25.8, 0.0, -136.0), Vector3(-25.8, 0.0, 136.0)))
    _register_track(
        _demo_curve(Vector3(-19.19, 0.0, -100.0), Vector3(-19.19, 0.0, 70.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "diverging"
    )
    _register_track(
        _demo_curve(Vector3(-25.8, 0.0, 236.0), Vector3(-25.8, 0.0, 536.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "start"
    )
    var first_switch_rid: RID = _register_track(
        _demo_curve(Vector3(-25.8, 0.0, 236.0), Vector3(-25.8, 0.0, 136.0)),
        _demo_curve(
            Vector3(-25.8, 0.0, 236.0),
            Vector3(-19.19, 0.0, 130.0),
            Vector3(-0.305, 0.0, -21.41),
            Vector3(0.305, 0.0, 20.0)
        ),
        TrackManager.TrackType.TRACK_SWITCH
    )
    var second_switch_rid: RID = _register_track(
        _demo_curve(Vector3(-19.19, 0.0, 130.0), Vector3(-19.19, 0.0, 70.0)),
        _demo_curve(
            Vector3(-19.19, 0.0, 130.0),
            Vector3(-12.53, 0.0, 80.0),
            Vector3(-0.513, 0.0, -8.065),
            Vector3(-0.823, 0.0, 6.27)
        ),
        TrackManager.TrackType.TRACK_SWITCH
    )
    _register_track(
        _demo_curve(
            Vector3(-12.53, 0.0, -100.0),
            Vector3(-12.53, 0.0, 80.0),
            Vector3.ZERO,
            Vector3(1.61, 0.0, -3.35)
        ),
        null,
        TrackManager.TrackType.TRACK_NORMAL,
        "diverging2"
    )
    return {
        "first_switch_rid": first_switch_rid,
        "second_switch_rid": second_switch_rid,
    }


func _curve(
    p1: Vector3,
    p2: Vector3,
    roll1: float = 0.0,
    roll2: float = 0.0,
) -> MaszynaTrackCurve:
    var curve: MaszynaTrackCurve = MaszynaTrackCurve.new()
    curve.p1 = p1
    curve.p2 = p2
    curve.roll1 = roll1
    curve.roll2 = roll2
    return curve


func _demo_curve(
    p1: Vector3,
    p2: Vector3,
    c1: Vector3 = Vector3.ZERO,
    c2: Vector3 = Vector3.ZERO,
) -> MaszynaTrackCurve:
    var curve: MaszynaTrackCurve = _curve(p1, p2)
    curve.c1 = c1
    curve.c2 = c2
    return curve


func _assert_float_eq(actual: float, expected: float, message: String = "") -> void:
    assert_true(absf(actual - expected) <= 0.01, "%s expected %s, got %s" % [message, expected, actual])


func _rail_position(x: float, z: float) -> Vector3:
    return Vector3(x, TrackManager.RAIL_HEIGHT, z)


func _track_position(track_rid: RID, offset: float) -> Vector3:
    var vehicle_rid: RID = RailVehiclePhysicsServer.vehicle_create()
    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        offset,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var transform: Transform3D = RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid)
    RailVehiclePhysicsServer.vehicle_free(vehicle_rid)
    return transform.origin


func _switch_track_position(track_rid: RID, offset: float, switch_track: int) -> Vector3:
    var previous_active_track: int = TrackManager.switch_get_active_track(track_rid)
    TrackManager.switch_set_active_track(track_rid, switch_track)
    var position: Vector3 = _track_position(track_rid, offset)
    TrackManager.switch_set_active_track(track_rid, previous_active_track)
    return position

func _vehicle_forward(vehicle: RailVehicle3D) -> Vector3:
    return -vehicle.global_basis.z.normalized()


func _assert_vector_eq(actual: Vector3, expected: Vector3, message: String = "") -> void:
    assert_true(actual.distance_to(expected) <= 0.02, "%s expected %s, got %s" % [message, expected, actual])
