extends MaszynaGutTest


var created_tracks: Array[RID] = []
var created_vehicle_rids: Array[RID] = []
var created_controllers: Array[TrainController] = []


func after_each() -> void:
    for vehicle_rid: RID in created_vehicle_rids:
        RailVehiclePhysicsServer.vehicle_free(vehicle_rid)
    created_vehicle_rids.clear()

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


func test_vehicle_set_track_initializes_normal_track_state() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        15.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(10.0, TrackManager.RAIL_HEIGHT, 0.0)
    )


func test_vehicle_set_track_initializes_switch_state_from_active_branch() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        switch_rid,
        3.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(2.1213, TrackManager.RAIL_HEIGHT, 2.1213),
        "switch initialization should sample the active diverging branch"
    )


func test_vehicle_get_transform_applies_common_and_diverging_orientation() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    var vehicle_rid: RID = _create_vehicle()

    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        switch_rid,
        4.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var common_forward: Vector3 = -RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).basis.z.normalized()

    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        switch_rid,
        4.0,
        TrackManager.Direction.DIRECTION_NORMAL
    )
    var diverging_forward: Vector3 = -RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).basis.z.normalized()

    _assert_vector_eq(common_forward, Vector3.RIGHT, "common branch should face along curve1")
    assert_true(diverging_forward.distance_to(Vector3.RIGHT) > 0.1, "diverging branch should not reuse curve1 orientation")


func test_vehicle_move_crosses_connected_tracks() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    var second_rid: RID = _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        8.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_move(vehicle_rid, 5.0)

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(13.0, TrackManager.RAIL_HEIGHT, 0.0)
    )


func test_vehicle_move_stops_at_ambiguous_shared_node() -> void:
    var source_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 10.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        source_rid,
        8.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_move(vehicle_rid, 5.0)

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(10.0, TrackManager.RAIL_HEIGHT, 0.0)
    )


func test_process_movement_advances_bound_controller_vehicle() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()
    var controller: TrainController = _create_controller()
    _set_controller_velocity(controller, 5.0)

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        8.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_bind_controller(vehicle_rid, controller.get_rid())

    RailVehiclePhysicsServer.process_movement(vehicle_rid, 1.0)

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(13.0, TrackManager.RAIL_HEIGHT, 0.0)
    )


func test_process_movement_without_bound_controller_is_noop() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        3.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.process_movement(vehicle_rid, 1.0)

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(3.0, TrackManager.RAIL_HEIGHT, 0.0)
    )


func test_process_movement_with_invalid_controller_reference_is_noop() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()
    var controller: TrainController = _create_controller()
    _set_controller_velocity(controller, 5.0)

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        3.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_bind_controller(vehicle_rid, controller.get_rid())

    remove_child(controller)
    controller.queue_free()
    await wait_idle_frames(1)

    RailVehiclePhysicsServer.process_movement(vehicle_rid, 1.0)

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(3.0, TrackManager.RAIL_HEIGHT, 0.0)
    )


func test_removed_track_makes_transform_and_movement_noop() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()
    var controller: TrainController = _create_controller()
    _set_controller_velocity(controller, 5.0)

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        3.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_bind_controller(vehicle_rid, controller.get_rid())

    TrackManager.track_free(track_rid)

    RailVehiclePhysicsServer.vehicle_move(vehicle_rid, 5.0)
    RailVehiclePhysicsServer.process_movement(vehicle_rid, 1.0)

    assert_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid),
        Transform3D.IDENTITY,
        "removed track should make transform queries safe and inert"
    )


func test_vehicle_reverse_on_switch_keeps_occupied_branch_after_switch_change() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        8.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_move(vehicle_rid, 5.0)
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)

    RailVehiclePhysicsServer.vehicle_move(vehicle_rid, -1.0)

    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(1.4142, TrackManager.RAIL_HEIGHT, 1.4142),
        "reverse movement should keep sampling the occupied diverging branch"
    )


func test_vehicle_move_forces_switch_diverging_when_entering_from_diverging_branch() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TrackType.TRACK_NORMAL
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.SwitchTrack.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var distance_on_switch: float = 2.8579
    var vehicle_rid: RID = _create_vehicle()

    RailVehiclePhysicsServer.vehicle_set_track(
        vehicle_rid,
        start_rid,
        12.0,
        TrackManager.Direction.DIRECTION_REVERSED
    )
    RailVehiclePhysicsServer.vehicle_move(vehicle_rid, 5.0)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.SwitchTrack.TRACK_DIVERGING)
    _assert_vector_eq(
        RailVehiclePhysicsServer.vehicle_get_transform(vehicle_rid).origin,
        _track_position(
            switch_rid,
            TrackManager.track_get_length(switch_rid, TrackManager.SwitchTrack.TRACK_DIVERGING) - distance_on_switch
        ),
        "diverging branch entry should force diverging switch route"
    )


func _create_vehicle() -> RID:
    var vehicle_rid: RID = RailVehiclePhysicsServer.vehicle_create()
    created_vehicle_rids.append(vehicle_rid)
    return vehicle_rid


func _create_controller() -> TrainController:
    var controller: TrainController = TrainController.new()
    controller.name = "MockController%d" % created_controllers.size()
    controller.train_id = "mock_train_%d" % created_controllers.size()
    controller.type_name = "test"
    add_child(controller)
    created_controllers.append(controller)
    return controller


func _set_controller_velocity(controller: TrainController, velocity: float) -> void:
    var state: Dictionary = controller.get_state()
    state["velocity"] = velocity


func _register_track(
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve = null,
    type: int = TrackManager.TrackType.TRACK_NORMAL
) -> RID:
    var track_rid: RID = TrackManager.track_create()
    created_tracks.append(track_rid)
    TrackManager.track_update_curves(track_rid, curve1, curve2)
    TrackManager.track_update(track_rid, type, "", 1.435)
    return track_rid


func _curve(
    p1: Vector3,
    p2: Vector3,
    c1: Vector3 = Vector3.ZERO,
    c2: Vector3 = Vector3.ZERO
) -> MaszynaTrackCurve:
    var curve: MaszynaTrackCurve = MaszynaTrackCurve.new()
    curve.p1 = p1
    curve.p2 = p2
    curve.c1 = c1
    curve.c2 = c2
    return curve


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


func _assert_vector_eq(actual: Vector3, expected: Vector3, message: String = "") -> void:
    assert_true(actual.distance_to(expected) <= 0.02, "%s expected %s, got %s" % [message, expected, actual])
