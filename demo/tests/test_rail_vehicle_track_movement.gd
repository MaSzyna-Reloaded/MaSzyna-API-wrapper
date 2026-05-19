extends MaszynaGutTest

var created_tracks: Array[RID] = []
var created_vehicles: Array[RailVehicle3D] = []


func after_each() -> void:
    for vehicle: RailVehicle3D in created_vehicles:
        if is_instance_valid(vehicle):
            if vehicle.get_parent():
                vehicle.get_parent().remove_child(vehicle)
            vehicle.queue_free()
    created_vehicles.clear()

    for track_rid: RID in created_tracks:
        if TrackManager.get_track(track_rid):
            TrackManager.unregister_track(track_rid)
    created_tracks.clear()
    TrackManager.rebuild_topology()


func test_start_track_name_initializes_and_clamps_offset() -> void:
    var track_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    TrackManager.rebuild_topology()

    var vehicle: RailVehicle3D = await _create_vehicle("start", 15.0)

    assert_eq(vehicle.current_track_rid, track_rid)
    _assert_float_eq(vehicle.current_track_offset, 10.0, "start offset should clamp to track length")
    _assert_vector_eq(vehicle.global_position, Vector3(10.0, 0.0, 0.0), "vehicle should be placed at track end")


func test_move_on_track_moves_forward_and_backward_on_current_track() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    TrackManager.rebuild_topology()
    var vehicle: RailVehicle3D = await _create_vehicle("start", 2.0)

    vehicle.move_on_track(3.0)
    _assert_float_eq(vehicle.current_track_offset, 5.0, "positive movement should increase offset")
    _assert_vector_eq(vehicle.global_position, Vector3(5.0, 0.0, 0.0), "positive movement should update position")

    vehicle.move_on_track(-4.0)
    _assert_float_eq(vehicle.current_track_offset, 1.0, "negative movement should decrease offset")
    _assert_vector_eq(vehicle.global_position, Vector3(1.0, 0.0, 0.0), "negative movement should update position")


func test_move_on_track_crosses_connected_tracks_and_clamps_at_dead_end() -> void:
    var first_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    var second_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()
    var vehicle: RailVehicle3D = await _create_vehicle("start", 8.0)

    assert_eq(vehicle.current_track_rid, first_rid)
    vehicle.move_on_track(5.0)
    assert_eq(vehicle.current_track_rid, second_rid)
    _assert_float_eq(vehicle.current_track_offset, 3.0, "movement should continue on next track")
    _assert_vector_eq(vehicle.global_position, Vector3(13.0, 0.0, 0.0), "position should continue on next track")

    vehicle.move_on_track(20.0)
    assert_eq(vehicle.current_track_rid, second_rid)
    _assert_float_eq(vehicle.current_track_offset, 10.0, "movement should clamp at graph end")
    _assert_vector_eq(vehicle.global_position, Vector3(20.0, 0.0, 0.0), "dead-end clamp should place vehicle at endpoint")


func test_move_on_track_updates_direction_when_entering_track_end() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    var reversed_rid: RID = _register_track(_curve(Vector3(20.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()
    var vehicle: RailVehicle3D = await _create_vehicle("start", 8.0)

    vehicle.move_on_track(5.0)

    assert_eq(vehicle.current_track_rid, reversed_rid)
    assert_eq(vehicle.direction, RailVehicle3D.Direction.OPPOSITE_TO_CONSIST_FRONT)
    _assert_float_eq(vehicle.current_track_offset, 7.0, "offset should decrease on reversed connected track")
    _assert_vector_eq(vehicle.global_position, Vector3(13.0, 0.0, 0.0), "vehicle should continue through reversed track")


func test_move_on_track_uses_switch_common_route() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.SWITCH
    )
    var main_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    _register_track(_curve(Vector3(10.0, 0.0, 10.0), Vector3(20.0, 0.0, 20.0)))
    TrackManager.set_switch_state(switch_rid, TrackManager.SwitchState.COMMON)
    TrackManager.rebuild_topology()
    var vehicle: RailVehicle3D = await _create_vehicle("start", 8.0)

    vehicle.move_on_track(15.0)

    assert_eq(vehicle.current_track_rid, main_rid)
    _assert_float_eq(vehicle.current_track_offset, 3.0, "common switch route should continue on main track")
    _assert_vector_eq(vehicle.global_position, Vector3(13.0, 0.0, 0.0), "common switch route should use curve1")


func test_move_on_track_uses_switch_diverging_route() -> void:
    _register_track(
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.SWITCH
    )
    _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    var diverging_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 10.0), Vector3(20.0, 0.0, 20.0)))
    TrackManager.set_switch_state(switch_rid, TrackManager.SwitchState.DIVERGING)
    TrackManager.rebuild_topology()
    var vehicle: RailVehicle3D = await _create_vehicle("start", 8.0)

    vehicle.move_on_track(19.1421)

    assert_eq(vehicle.current_track_rid, diverging_rid)
    _assert_float_eq(vehicle.current_track_offset, 3.0, "diverging route should continue after switch curve length")
    _assert_vector_eq(vehicle.global_position, Vector3(12.1213, 0.0, 12.1213), "diverging route should use curve2")


func test_track_transform_applies_roll_and_direction_rotation() -> void:
    _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0), 0.0, 10.0),
        null,
        TrackManager.TrackType.NORMAL,
        "start"
    )
    TrackManager.rebuild_topology()
    var vehicle: RailVehicle3D = await _create_vehicle("start", 5.0)
    var aligned_forward: Vector3 = -vehicle.global_basis.z.normalized()
    var aligned_up: Vector3 = vehicle.global_basis.y.normalized()

    vehicle.direction = RailVehicle3D.Direction.OPPOSITE_TO_CONSIST_FRONT
    vehicle._process_dirty()
    var opposite_forward: Vector3 = -vehicle.global_basis.z.normalized()

    _assert_vector_eq(aligned_forward, Vector3.RIGHT, "aligned vehicle forward should follow track tangent")
    assert_true(aligned_up.distance_to(Vector3.UP) > 0.01, "roll should tilt vehicle up vector")
    _assert_vector_eq(opposite_forward, Vector3.LEFT, "opposite direction should rotate vehicle by 180 degrees")


func _create_vehicle(track_name: String, offset: float) -> RailVehicle3D:
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = track_name
    vehicle.start_track_offset = offset
    add_child(vehicle)
    created_vehicles.append(vehicle)
    await wait_idle_frames(2)
    return vehicle


func _register_track(
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve = null,
    type: int = TrackManager.TrackType.NORMAL,
    name: String = "",
) -> RID:
    var track_rid: RID = TrackManager.register_track()
    created_tracks.append(track_rid)
    TrackManager.update_track(track_rid, type, name, curve1, curve2, 1.435)
    return track_rid


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


func _assert_float_eq(actual: float, expected: float, message: String = "") -> void:
    assert_true(absf(actual - expected) <= 0.01, "%s expected %s, got %s" % [message, expected, actual])


func _assert_vector_eq(actual: Vector3, expected: Vector3, message: String = "") -> void:
    assert_true(actual.distance_to(expected) <= 0.02, "%s expected %s, got %s" % [message, expected, actual])
