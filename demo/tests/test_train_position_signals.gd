extends MaszynaGutTest

var _train_system = null
var _created_tracks: Array[RID] = []
var _created_vehicles: Array[RailVehicle3D] = []
var _created_controllers: Array[TrainController] = []


func before_each() -> void:
    _train_system = Engine.get_singleton("TrainSystem")


func after_each() -> void:
    for vehicle: RailVehicle3D in _created_vehicles:
        if is_instance_valid(vehicle):
            if vehicle.get_parent():
                vehicle.get_parent().remove_child(vehicle)
            vehicle.queue_free()
    _created_vehicles.clear()

    for controller: TrainController in _created_controllers:
        if is_instance_valid(controller):
            if controller.get_parent():
                controller.get_parent().remove_child(controller)
            controller.queue_free()
    _created_controllers.clear()

    for track_rid: RID in _created_tracks:
        if TrackManager.track_exists(track_rid):
            TrackManager.track_free(track_rid)
    _created_tracks.clear()
    TrackManager.topology_rebuild()


func test_train_position_changed_signal_emits_after_crossing_one_meter() -> void:
    var fixture: Dictionary = await _create_fixture(0.0)
    var train: TrainController = fixture["controller"]
    var vehicle: RailVehicle3D = fixture["vehicle"]

    watch_signals(train)
    watch_signals(_train_system)

    vehicle.move_on_track(0.5)
    assert_signal_not_emitted(train, "position_changed", "Should not emit for a 0.5m move")
    assert_signal_not_emitted(_train_system, "train_position_changed", "TrainSystem should not emit yet")

    vehicle.move_on_track(0.6)
    assert_signal_emitted(train, "position_changed", "Should emit after crossing 1m total movement")
    assert_signal_emitted_with_parameters(_train_system, "train_position_changed", ["test_train", train.get_world_position()])


func test_train_position_changed_signal_rearms_after_last_emission() -> void:
    var fixture: Dictionary = await _create_fixture(1.1)
    var train: TrainController = fixture["controller"]
    var vehicle: RailVehicle3D = fixture["vehicle"]

    watch_signals(train)

    vehicle.move_on_track(0.9)
    assert_signal_not_emitted(train, "position_changed", "Should not emit before another full meter of movement")

    vehicle.move_on_track(0.2)
    assert_signal_emitted(train, "position_changed", "Should emit after another 1m from the last emitted position")


func test_train_system_bubbling_after_unregistration() -> void:
    var fixture: Dictionary = await _create_fixture(0.0, "test_train_2")
    var train: TrainController = fixture["controller"]
    var vehicle: RailVehicle3D = fixture["vehicle"]

    watch_signals(_train_system)
    _train_system.unregister_train("test_train_2")

    vehicle.move_on_track(2.0)
    assert_signal_not_emitted(_train_system, "train_position_changed", "Should not bubble after unregistration")


func _create_fixture(offset: float, train_id: String = "test_train") -> Dictionary:
    var track_rid: RID = TrackManager.track_create()
    _created_tracks.append(track_rid)
    TrackManager.track_update_curves(track_rid, _curve(Vector3(0.0, 0.0, 0.0), Vector3(20.0, 0.0, 0.0)), null)
    TrackManager.track_update(track_rid, TrackManager.TrackType.TRACK_NORMAL, "start", 1.435)
    TrackManager.topology_rebuild()

    var controller: TrainController = TrainController.new()
    controller.train_id = train_id
    controller.type_name = "test"
    add_child(controller)
    _created_controllers.append(controller)

    var vehicle: RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = "start"
    vehicle.start_track_offset = offset
    vehicle.set("start_direction", TrackManager.Direction.DIRECTION_REVERSED)
    add_child(vehicle)
    vehicle.controller_path = vehicle.get_path_to(controller)
    _created_vehicles.append(vehicle)
    await wait_idle_frames(2)

    return {
        "controller": controller,
        "vehicle": vehicle,
    }


func _curve(p1: Vector3, p2: Vector3) -> MaszynaTrackCurve:
    var curve: MaszynaTrackCurve = MaszynaTrackCurve.new()
    curve.p1 = p1
    curve.p2 = p2
    return curve
