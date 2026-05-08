extends MaszynaGutTest

var created_tracks: Array[TrackNormal3D] = []
var created_track_rids: Array[RID] = []


func after_each() -> void:
    for track: TrackNormal3D in created_tracks:
        remove_child(track)
        track.queue_free()
    created_tracks.clear()
    for track_rid: RID in created_track_rids:
        if TrackManager.track_exists(track_rid):
            TrackManager.track_free(track_rid)
    created_track_rids.clear()
    TrackManager.topology_rebuild()
    await wait_idle_frames(2)


func test_switch_is_right_returns_true_for_right_switch() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "right_switch",
        _curve(
            Vector3(0.0, 0.0, 0.0),
            Vector3(0.0, 0.0, -20.0)
        ),
        _curve(
            Vector3(0.0, 0.0, 0.0),
            Vector3(6.0, 0.0, -20.0),
            Vector3(0.0, 0.0, -16.667),
            Vector3(0.0, 0.0, 16.667)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "right switch should register in TrackManager")
    assert_true(TrackManager.switch_is_right(track_rid))


func test_switch_is_right_returns_false_for_left_switch() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "left_switch",
        _curve(
            Vector3(0.0, 0.0, 0.0),
            Vector3(0.0, 0.0, -20.0)
        ),
        _curve(
            Vector3(0.0, 0.0, 0.0),
            Vector3(-6.0, 0.0, -20.0),
            Vector3(0.0, 0.0, -16.667),
            Vector3(0.0, 0.0, 16.667)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "left switch should register in TrackManager")
    assert_false(TrackManager.switch_is_right(track_rid))


func test_switch_is_right_returns_true_for_smoothed_demo3d_right_switch() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "smoothed_right_switch",
        _curve(
            Vector3(-19.19, 0.0, 130.0),
            Vector3(-19.19, 0.0, 70.0)
        ),
        _curve(
            Vector3(-19.19, 0.0, 130.0),
            Vector3(-12.53, 0.0, 80.0),
            Vector3(0.0, 0.0, -16.667),
            Vector3(0.0, 0.0, 16.667)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "smoothed right switch should register in TrackManager")
    assert_true(TrackManager.switch_is_right(track_rid))


func test_switch_is_right_returns_false_for_smoothed_demo3d_left_switch() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "smoothed_left_switch",
        _curve(
            Vector3(19.19, 0.0, 130.0),
            Vector3(19.19, 0.0, 70.0)
        ),
        _curve(
            Vector3(19.19, 0.0, 130.0),
            Vector3(12.53, 0.0, 80.0),
            Vector3(0.0, 0.0, -16.667),
            Vector3(0.0, 0.0, 16.667)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "smoothed left switch should register in TrackManager")
    assert_false(TrackManager.switch_is_right(track_rid))


func test_switch_is_right_returns_true_for_right_switch_with_common_end() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "right_switch_common_end",
        _curve(
            Vector3(0.0, 0.0, -20.0),
            Vector3(0.0, 0.0, 0.0)
        ),
        _curve(
            Vector3(6.0, 0.0, -20.0),
            Vector3(0.0, 0.0, 0.0)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "right switch with common end should register in TrackManager")
    assert_true(TrackManager.switch_is_right(track_rid))


func test_switch_is_right_returns_false_for_left_switch_with_common_end() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "left_switch_common_end",
        _curve(
            Vector3(0.0, 0.0, -20.0),
            Vector3(0.0, 0.0, 0.0)
        ),
        _curve(
            Vector3(-6.0, 0.0, -20.0),
            Vector3(0.0, 0.0, 0.0)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "left switch with common end should register in TrackManager")
    assert_false(TrackManager.switch_is_right(track_rid))


func test_switch_is_right_returns_false_for_demo3d_left_switch() -> void:
    var track: TrackSwitch3D = await _create_switch_track(
        "demo3d_left_switch",
        _curve(
            Vector3(-25.8, 0.0, 136.0),
            Vector3(-25.8, 0.0, 100.0)
        ),
        _curve(
            Vector3(-25.8, 0.0, 136.0),
            Vector3(-32.5, 0.0, 100.0),
            Vector3(0.0, 0.0, -11.205),
            Vector3(0.0, 0.0, 10.675)
        )
    )
    var track_rid: RID = TrackManager.track_get_rid_by_name(track.track_name)

    assert_true(track_rid.is_valid(), "demo3d left switch should register in TrackManager")
    assert_false(TrackManager.switch_is_right(track_rid))


func test_get_unique_endpoint_connection_returns_null_for_ambiguous_connections() -> void:
    var previous_rid: RID = TrackManager.track_create()
    created_track_rids.append(previous_rid)
    TrackManager.track_update_curves(
        previous_rid,
        _curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)),
        null
    )
    TrackManager.track_update(previous_rid, TrackManager.TrackType.TRACK_NORMAL, "", 1.435)

    var source_rid: RID = TrackManager.track_create()
    created_track_rids.append(source_rid)
    TrackManager.track_update_curves(
        source_rid,
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null
    )
    TrackManager.track_update(source_rid, TrackManager.TrackType.TRACK_NORMAL, "", 1.435)

    var first_neighbor_rid: RID = TrackManager.track_create()
    created_track_rids.append(first_neighbor_rid)
    TrackManager.track_update_curves(
        first_neighbor_rid,
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null
    )
    TrackManager.track_update(first_neighbor_rid, TrackManager.TrackType.TRACK_NORMAL, "", 1.435)

    var second_neighbor_rid: RID = TrackManager.track_create()
    created_track_rids.append(second_neighbor_rid)
    TrackManager.track_update_curves(
        second_neighbor_rid,
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 10.0)),
        null
    )
    TrackManager.track_update(second_neighbor_rid, TrackManager.TrackType.TRACK_NORMAL, "", 1.435)

    TrackManager.topology_rebuild()

    assert_eq(
        TrackRenderingServer._get_unique_endpoint_connection(
            source_rid,
            TrackManager.EndpointIndex.CURVE1_P2
        ),
        null
    )

    var previous_connection: TrackManager.EndpointRef = TrackRenderingServer._get_unique_endpoint_connection(
        previous_rid,
        TrackManager.EndpointIndex.CURVE1_P2
    )

    assert_not_null(previous_connection)
    assert_eq(previous_connection.track_rid, source_rid)
    assert_eq(previous_connection.endpoint_index, TrackManager.EndpointIndex.CURVE1_P1)


func test_switch_blade_layout_uses_curve1_left_and_curve2_right_for_right_switch() -> void:
    var layout: Dictionary = TrackRenderingServer._get_switch_blade_layout(true, -0.05, 0.0, TrackManager.SWITCH_MAX_OFFSET)

    assert_true(layout["primary_blade_mirrored"], "right switch should place curve1 blade on the left rail")
    assert_false(layout["secondary_blade_mirrored"], "right switch should place curve2 blade on the right rail")
    assert_almost_eq(layout["primary_blade_offset"], 0.0, 0.001)
    assert_almost_eq(layout["secondary_blade_offset"], -0.15, 0.001)


func test_switch_blade_layout_uses_curve1_right_and_curve2_left_for_left_switch() -> void:
    var layout: Dictionary = TrackRenderingServer._get_switch_blade_layout(false, -0.05, 0.0, TrackManager.SWITCH_MAX_OFFSET)

    assert_false(layout["primary_blade_mirrored"], "left switch should place curve1 blade on the right rail")
    assert_true(layout["secondary_blade_mirrored"], "left switch should place curve2 blade on the left rail")
    assert_almost_eq(layout["primary_blade_offset"], 0.0, 0.001)
    assert_almost_eq(layout["secondary_blade_offset"], 0.15, 0.001)


func _create_switch_track(
    track_name: String,
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve
) -> TrackSwitch3D:
    var track: TrackSwitch3D = TrackSwitch3D.new()
    track.track_name = track_name
    track.curve = curve1
    track.diverging_curve = curve2
    add_child(track)
    created_tracks.append(track)
    await wait_idle_frames(2)
    return track


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
