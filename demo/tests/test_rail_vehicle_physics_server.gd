extends MaszynaGutTest


var created_tracks: Array[RID] = []
var created_vehicle_rids: Array[RID] = []
var created_controllers: Array[VehicleController] = []


func after_each() -> void:
    for vehicle_rid: RID in created_vehicle_rids:
        RailVehicleServer.vehicle_free(vehicle_rid)
    created_vehicle_rids.clear()

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
        TrackManager.TRACK_NORMAL
    )
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        15.0,
        TrackManager.DIRECTION_NORMAL
    )

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(10.0, TrackManager.rail_height, 0.0)
    )


func test_vehicle_set_track_follows_connected_tracks_for_initial_offsets() -> void:
    for index:int in range(3):
        _register_track(_curve(Vector3(index * 10, 0, 0), Vector3((index + 1) * 10, 0, 0)))
    TrackManager.topology_rebuild()
    var vehicle_rid:RID = _create_vehicle()
    for direction:TrackManager.Direction in [TrackManager.DIRECTION_NORMAL, TrackManager.DIRECTION_REVERSED]:
        RailVehicleServer.vehicle_set_track(vehicle_rid, created_tracks[2], -15.0, direction)
        _assert_vector_eq(RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
            Vector3(5.0, TrackManager.rail_height, 0.0))
        RailVehicleServer.vehicle_set_track(vehicle_rid, created_tracks[0], 25.0, direction)
        _assert_vector_eq(RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
            Vector3(25.0, TrackManager.rail_height, 0.0))


func test_vehicle_set_track_initializes_switch_state_from_active_branch() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_DIVERGING)
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        switch_rid,
        3.0,
        TrackManager.DIRECTION_NORMAL
    )

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(2.1213, TrackManager.rail_height, 2.1213),
        "switch initialization should sample the active diverging branch"
    )


func test_vehicle_get_transform_applies_common_and_diverging_orientation() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    var vehicle_rid: RID = _create_vehicle()

    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_COMMON)
    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        switch_rid,
        4.0,
        TrackManager.DIRECTION_NORMAL
    )
    var common_forward: Vector3 = -RailVehicleServer.vehicle_get_transform(vehicle_rid).basis.z.normalized()

    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_DIVERGING)
    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        switch_rid,
        4.0,
        TrackManager.DIRECTION_NORMAL
    )
    var diverging_forward: Vector3 = -RailVehicleServer.vehicle_get_transform(vehicle_rid).basis.z.normalized()

    _assert_vector_eq(common_forward, Vector3.RIGHT, "common branch should face along curve1")
    assert_true(diverging_forward.distance_to(Vector3.RIGHT) > 0.1, "diverging branch should not reuse curve1 orientation")


func test_vehicle_move_crosses_connected_tracks() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    var second_rid: RID = _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        8.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_move(vehicle_rid, 5.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(13.0, TrackManager.rail_height, 0.0)
    )


func test_vehicle_transform_at_distance_crosses_tracks_without_moving_vehicle() -> void:
    var first_rid:RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid:RID = _create_vehicle()
    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        first_rid,
        8.0,
        TrackManager.DIRECTION_REVERSED
    )

    var sampled_transform:Transform3D = (
        RailVehicleServer.vehicle_get_transform_at_distance(vehicle_rid, 5.0)
    )

    _assert_vector_eq(
        sampled_transform.origin,
        Vector3(13.0, TrackManager.rail_height, 0.0),
        "sample should continue on the connected track",
    )
    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(8.0, TrackManager.rail_height, 0.0),
        "sample should not move the vehicle",
    )


func test_vehicle_transform_at_distance_does_not_change_switch_state() -> void:
    var start_rid:RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    var switch_rid:RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var vehicle_rid:RID = _create_vehicle()
    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        start_rid,
        12.0,
        TrackManager.DIRECTION_REVERSED
    )

    RailVehicleServer.vehicle_get_transform_at_distance(vehicle_rid, 5.0)

    assert_eq(
        TrackManager.switch_get_active_track(switch_rid),
        TrackManager.TRACK_COMMON,
        "sampling through a diverging branch should not move the switch",
    )


func test_vehicle_move_stops_at_ambiguous_shared_node() -> void:
    var source_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 10.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        source_rid,
        8.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_move(vehicle_rid, 5.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(10.0, TrackManager.rail_height, 0.0)
    )


func test_process_movement_advances_bound_controller_vehicle() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var controller: VehicleController = _create_controller(5.0)
    # the vehicle the controller already belongs to - a second handle would step it twice
    var vehicle_rid: RID = controller.get_rid()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        8.0,
        TrackManager.DIRECTION_REVERSED
    )

    RailVehicleServer.vehicle_process_movement(vehicle_rid, 1.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(3.0, TrackManager.rail_height, 0.0)
    )


func test_process_movement_moves_vehicle_toward_its_own_front() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var controller: VehicleController = _create_controller(5.0)
    # the vehicle the controller already belongs to - a second handle would step it twice
    var vehicle_rid: RID = controller.get_rid()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        4.0,
        TrackManager.DIRECTION_NORMAL
    )
    var forward: Vector3 = -RailVehicleServer.vehicle_get_transform(vehicle_rid).basis.z.normalized()

    RailVehicleServer.vehicle_process_movement(vehicle_rid, 1.0)

    var moved_by: Vector3 = (
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin
        - Vector3(4.0, TrackManager.rail_height, 0.0)
    )
    assert_true(
        moved_by.normalized().distance_to(forward) < 0.01,
        "positive mover velocity should move the vehicle toward its own front, got %s vs front %s" % [moved_by, forward]
    )


func test_process_movement_without_bound_controller_is_noop() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        3.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_process_movement(vehicle_rid, 1.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(3.0, TrackManager.rail_height, 0.0)
    )


func test_process_movement_with_invalid_controller_reference_is_noop() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var controller: VehicleController = _create_controller(5.0)
    # the vehicle the controller already belongs to - a second handle would step it twice
    var vehicle_rid: RID = controller.get_rid()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        3.0,
        TrackManager.DIRECTION_REVERSED
    )

    # what makes the reference invalid is the server losing it, not a local variable being
    # dropped - the vehicle then has nothing to ask for a velocity and stays where it is
    RailVehicleServer.vehicle_attach_controller(vehicle_rid, 0)
    await wait_idle_frames(1)

    RailVehicleServer.vehicle_process_movement(vehicle_rid, 1.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(3.0, TrackManager.rail_height, 0.0)
    )


func test_removed_track_makes_transform_and_movement_noop() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.topology_rebuild()
    var controller: VehicleController = _create_controller(5.0)
    # the vehicle the controller already belongs to - a second handle would step it twice
    var vehicle_rid: RID = controller.get_rid()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        3.0,
        TrackManager.DIRECTION_REVERSED
    )

    TrackManager.track_free(track_rid)

    RailVehicleServer.vehicle_move(vehicle_rid, 5.0)
    RailVehicleServer.vehicle_process_movement(vehicle_rid, 1.0)

    assert_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid),
        Transform3D.IDENTITY,
        "removed track should make transform queries safe and inert"
    )


func test_vehicle_reverse_on_switch_keeps_occupied_branch_after_switch_change() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        created_tracks[0],
        8.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_move(vehicle_rid, 5.0)
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_COMMON)

    RailVehicleServer.vehicle_move(vehicle_rid, -1.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        Vector3(1.4142, TrackManager.rail_height, 1.4142),
        "reverse movement should keep sampling the occupied diverging branch"
    )


func test_vehicle_move_forces_switch_diverging_when_entering_from_diverging_branch() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var distance_on_switch: float = 2.8579
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        start_rid,
        12.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_move(vehicle_rid, 5.0)

    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.TRACK_DIVERGING)
    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        _track_position(
            switch_rid,
            TrackManager.track_get_length(switch_rid, TrackManager.TRACK_DIVERGING) - distance_on_switch
        ),
        "diverging branch entry should force diverging switch route"
    )


func test_vehicle_move_large_offset_forces_switch_when_entering_from_diverging_branch() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(20.0, 0.0, 20.0), Vector3(10.0, 0.0, 10.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    var next_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(-10.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_COMMON)
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        start_rid,
        12.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_move(vehicle_rid, 25.0)

    var expected_next_offset: float = 25.0 \
        - (TrackManager.track_get_length(start_rid) - 12.0) \
        - TrackManager.track_get_length(switch_rid, TrackManager.TRACK_DIVERGING)
    assert_eq(TrackManager.switch_get_active_track(switch_rid), TrackManager.TRACK_DIVERGING)
    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        _track_position(next_rid, expected_next_offset),
        "large diverging-side entry should force diverging without incremental simulation"
    )


func test_vehicle_move_from_common_point_uses_active_switch_branch() -> void:
    var start_rid: RID = _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TRACK_NORMAL
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TRACK_SWITCH
    )
    TrackManager.switch_set_active_track(switch_rid, TrackManager.TRACK_DIVERGING)
    TrackManager.topology_rebuild()
    var vehicle_rid: RID = _create_vehicle()

    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        start_rid,
        8.0,
        TrackManager.DIRECTION_REVERSED
    )
    RailVehicleServer.vehicle_move(vehicle_rid, 15.0)

    _assert_vector_eq(
        RailVehicleServer.vehicle_get_transform(vehicle_rid).origin,
        _track_position(switch_rid, 13.0),
        "common-point entry should follow the active diverging branch"
    )


func _create_vehicle() -> RID:
    var vehicle_rid: RID = RailVehicleServer.vehicle_create()
    created_vehicle_rids.append(vehicle_rid)
    return vehicle_rid


## `velocity` (m/s) is the mover's own velocity, given as the initial velocity in km/h.
func _create_controller(velocity: float = 0.0) -> VehicleController:
    # the initial velocity is read while the Mover is initialised, so it goes in before the build
    var controller: VehicleController = build_vehicle(
            "mock_train_%d" % created_controllers.size(), null, velocity * 3.6)
    controller.type_name = "test"
    created_controllers.append(controller)
    return controller


func _register_track(
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve = null,
    type: int = TrackManager.TRACK_NORMAL
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
    var vehicle_rid: RID = RailVehicleServer.vehicle_create()
    RailVehicleServer.vehicle_set_track(
        vehicle_rid,
        track_rid,
        offset,
        TrackManager.DIRECTION_NORMAL
    )
    var transform: Transform3D = RailVehicleServer.vehicle_get_transform(vehicle_rid)
    RailVehicleServer.vehicle_free(vehicle_rid)
    return transform.origin


func _assert_vector_eq(actual: Vector3, expected: Vector3, message: String = "") -> void:
    assert_true(actual.distance_to(expected) <= 0.02, "%s expected %s, got %s" % [message, expected, actual])
