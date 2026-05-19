extends MaszynaGutTest

var created_tracks: Array[RID] = []
var topology_change_count: int = 0


func after_each() -> void:
    for track_rid: RID in created_tracks:
        if TrackManager.get_track(track_rid):
            TrackManager.unregister_track(track_rid)
    created_tracks.clear()
    TrackManager.rebuild_topology()


func test_connects_normal_track_endpoints() -> void:
    var first_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    var second_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rid(first_rid, 1), second_rid)
    assert_eq(TrackManager.get_connected_track_rid(second_rid, 0), first_rid)


func test_switch_branch_neighbors_with_common_start() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.SWITCH
    )
    var previous_rid: RID = _register_track(_curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)))
    var main_next_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    var diverging_next_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 10.0), Vector3(20.0, 0.0, 20.0)))
    TrackManager.rebuild_topology()

    var main_neighbors: TrackManager.BranchNeighbors = TrackManager.get_track_branch_neighbors(switch_rid, 0)
    var diverging_neighbors: TrackManager.BranchNeighbors = TrackManager.get_track_branch_neighbors(switch_rid, 1)

    assert_eq(TrackManager.get_track(switch_rid).switch_common_endpoint_index, 0)
    assert_eq(main_neighbors.previous_track_rid, previous_rid)
    assert_eq(main_neighbors.previous_endpoint_index, 1)
    assert_eq(main_neighbors.next_track_rid, main_next_rid)
    assert_eq(main_neighbors.next_endpoint_index, 0)
    assert_eq(diverging_neighbors.previous_track_rid, previous_rid)
    assert_eq(diverging_neighbors.previous_endpoint_index, 1)
    assert_eq(diverging_neighbors.next_track_rid, diverging_next_rid)
    assert_eq(diverging_neighbors.next_endpoint_index, 0)


func test_switch_branch_neighbors_with_common_end() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(20.0, 0.0, 10.0), Vector3(10.0, 0.0, 0.0)),
        TrackManager.TrackType.SWITCH
    )
    var main_previous_rid: RID = _register_track(_curve(Vector3(-10.0, 0.0, 0.0), Vector3(0.0, 0.0, 0.0)))
    var diverging_previous_rid: RID = _register_track(_curve(Vector3(30.0, 0.0, 20.0), Vector3(20.0, 0.0, 10.0)))
    var next_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(30.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()

    var main_neighbors: TrackManager.BranchNeighbors = TrackManager.get_track_branch_neighbors(switch_rid, 0)
    var diverging_neighbors: TrackManager.BranchNeighbors = TrackManager.get_track_branch_neighbors(switch_rid, 1)

    assert_eq(TrackManager.get_track(switch_rid).switch_common_endpoint_index, 1)
    assert_eq(main_neighbors.previous_track_rid, main_previous_rid)
    assert_eq(main_neighbors.previous_endpoint_index, 1)
    assert_eq(main_neighbors.next_track_rid, next_rid)
    assert_eq(main_neighbors.next_endpoint_index, 0)
    assert_eq(diverging_neighbors.previous_track_rid, diverging_previous_rid)
    assert_eq(diverging_neighbors.previous_endpoint_index, 1)
    assert_eq(diverging_neighbors.next_track_rid, next_rid)
    assert_eq(diverging_neighbors.next_endpoint_index, 0)


func test_switch_common_node_does_not_report_own_branch_as_connection() -> void:
    var switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.SWITCH
    )
    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rids(switch_rid, 0), [])
    assert_eq(TrackManager.get_connected_track_rid(switch_rid, 0), TrackManager.UNDEFINED_TRACK)


func test_rebuild_topology_preserves_connections() -> void:
    var first_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    var second_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))

    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rid(first_rid, 1), second_rid)
    assert_eq(TrackManager.get_connected_track_rid(second_rid, 0), first_rid)


func test_register_track_marks_topology_changed() -> void:
    TrackManager.is_topology_changed = false
    topology_change_count = 0
    TrackManager.topology_changed.connect(_on_topology_changed)

    var track_rid: RID = TrackManager.register_track()
    created_tracks.append(track_rid)

    TrackManager.topology_changed.disconnect(_on_topology_changed)
    assert_true(TrackManager.is_topology_changed)
    assert_eq(topology_change_count, 1)


func test_unregister_track_marks_topology_changed() -> void:
    var track_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()
    topology_change_count = 0
    TrackManager.topology_changed.connect(_on_topology_changed)

    TrackManager.unregister_track(track_rid)
    created_tracks.erase(track_rid)

    TrackManager.topology_changed.disconnect(_on_topology_changed)
    assert_true(TrackManager.is_topology_changed)
    assert_eq(topology_change_count, 1)


func test_connected_endpoint_change_marks_topology_changed() -> void:
    var first_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    var second_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()
    topology_change_count = 0
    TrackManager.topology_changed.connect(_on_topology_changed)

    TrackManager.update_track(
        first_rid,
        TrackManager.TrackType.NORMAL,
        "",
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(11.0, 0.0, 0.0)),
        null,
        1.435
    )

    TrackManager.topology_changed.disconnect(_on_topology_changed)
    assert_eq(TrackManager.get_connected_track_rid(first_rid, 1), second_rid)
    assert_true(TrackManager.is_topology_changed)
    assert_eq(topology_change_count, 1)


func test_unconnected_endpoint_change_does_not_mark_topology_changed() -> void:
    var first_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    var second_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    TrackManager.rebuild_topology()
    topology_change_count = 0
    TrackManager.topology_changed.connect(_on_topology_changed)

    TrackManager.update_track(
        first_rid,
        TrackManager.TrackType.NORMAL,
        "",
        _curve(Vector3(-1.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        1.435
    )

    TrackManager.topology_changed.disconnect(_on_topology_changed)
    assert_eq(TrackManager.get_connected_track_rid(first_rid, 1), second_rid)
    assert_false(TrackManager.is_topology_changed)
    assert_eq(topology_change_count, 0)


func test_disconnected_graphs_stay_independent() -> void:
    var first_a_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    var second_a_rid: RID = _register_track(_curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)))
    var first_b_rid: RID = _register_track(_curve(Vector3(100.0, 0.0, 0.0), Vector3(110.0, 0.0, 0.0)))
    var second_b_rid: RID = _register_track(_curve(Vector3(110.0, 0.0, 0.0), Vector3(120.0, 0.0, 0.0)))

    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rid(first_a_rid, 1), second_a_rid)
    assert_eq(TrackManager.get_connected_track_rid(first_b_rid, 1), second_b_rid)
    assert_true(not TrackManager.get_track(first_a_rid).graph_id == TrackManager.get_track(first_b_rid).graph_id)


func test_incompatible_track_types_do_not_share_topology() -> void:
    var track_rid: RID = _register_track(_curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)))
    var road_rid: RID = _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.ROAD
    )

    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rids(track_rid, 1), [])
    assert_eq(TrackManager.get_connected_track_rids(road_rid, 0), [])
    assert_true(not TrackManager.get_track(track_rid).graph_id == TrackManager.get_track(road_rid).graph_id)


func test_road_connects_to_cross() -> void:
    var road_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.ROAD
    )
    var cross_rid: RID = _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        _curve(Vector3(15.0, 0.0, -5.0), Vector3(15.0, 0.0, 5.0)),
        TrackManager.TrackType.CROSS
    )

    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rid(road_rid, 1), cross_rid)
    assert_eq(TrackManager.get_connected_track_rid(cross_rid, 0), road_rid)


func test_river_connects_to_tributary() -> void:
    var river_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.RIVER
    )
    var tributary_rid: RID = _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        null,
        TrackManager.TrackType.TRIBUTARY
    )

    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rid(river_rid, 1), tributary_rid)
    assert_eq(TrackManager.get_connected_track_rid(tributary_rid, 0), river_rid)


func test_switches_do_not_connect_to_each_other_without_regular_track() -> void:
    var first_switch_rid: RID = _register_track(
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 0.0)),
        _curve(Vector3(0.0, 0.0, 0.0), Vector3(10.0, 0.0, 10.0)),
        TrackManager.TrackType.SWITCH
    )
    var second_switch_rid: RID = _register_track(
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)),
        _curve(Vector3(10.0, 0.0, 0.0), Vector3(20.0, 0.0, 10.0)),
        TrackManager.TrackType.SWITCH
    )

    TrackManager.rebuild_topology()

    assert_eq(TrackManager.get_connected_track_rids(first_switch_rid, 1), [])
    assert_eq(TrackManager.get_connected_track_rids(second_switch_rid, 0), [])


func _register_track(
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve = null,
    type: int = TrackManager.TrackType.NORMAL,
) -> RID:
    var track_rid: RID = TrackManager.register_track()
    created_tracks.append(track_rid)
    TrackManager.update_track(track_rid, type, "", curve1, curve2, 1.435)
    return track_rid


func _curve(p1: Vector3, p2: Vector3) -> MaszynaTrackCurve:
    var curve: MaszynaTrackCurve = MaszynaTrackCurve.new()
    curve.p1 = p1
    curve.p2 = p2
    return curve


func _on_topology_changed() -> void:
    topology_change_count += 1
