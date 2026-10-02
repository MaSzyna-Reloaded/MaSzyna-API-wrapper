extends MaszynaGutTest

const FIXTURES_GAME_DIR: String = "res://tests/fixtures"

var _previous_game_dir: String


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)


func after_each() -> void:
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_vehicle_keeps_no_rain_volume_of_its_own() -> void:
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    add_child_autofree(vehicle)
    MaszynaRailVehicle3DManager.build_into(vehicle, "dynamic/test/synthetic_v1", "synthetic", "", "test_vehicle_rain", 0.0)
    await wait_idle_frames(2)

    assert_eq(_rain_volumes(vehicle).size(), 0)


func test_shown_cab_excludes_rain_over_the_vehicle_body() -> void:
    var vehicle: RailVehicle3D = RailVehicle3D.new()
    add_child_autofree(vehicle)
    MaszynaRailVehicle3DManager.build_into(vehicle, "dynamic/test/synthetic_v1", "synthetic", "", "test_vehicle_rain", 0.0)
    await wait_idle_frames(2)

    var cabin: Cabin3D = CabinSystem.vehicle_show_cabin(vehicle.get_rid())
    var volumes: Array[RainVolume] = _rain_volumes(cabin)
    var controller: VehicleController = vehicle.get_controller()
    CabinSystem.vehicle_hide_cabin(vehicle.get_rid())

    assert_eq(volumes.size(), 1)
    assert_eq(
        volumes[0].size,
        Vector3(controller.dimensions_width, controller.dimensions_height, controller.dimensions_length)
    )
    assert_almost_eq(volumes[0].position.y, controller.dimensions_height * 0.5, 0.000001)
    assert_almost_eq(volumes[0].precipitation_delta, -1.0, 0.000001)


## Internal children too: the vehicle's parts and the cab's generated interior are internal
func _rain_volumes(node: Node) -> Array[RainVolume]:
    var found: Array[RainVolume] = []
    for child: Node in node.get_children(true):
        if child is RainVolume:
            found.append(child)
        found.append_array(_rain_volumes(child))
    return found


func test_rain_volumes_beyond_active_distance_are_ignored() -> void:
    var near_volume: RainVolume = add_child_autofree(RainVolume.new())
    var far_volume: RainVolume = add_child_autofree(RainVolume.new())
    var world_3d: World3D = near_volume.get_world_3d()
    far_volume.position = Vector3(0.0, 0.0, 100.0)

    WeatherServer.set_weather_observer_sample(world_3d, Vector3.ZERO)
    WeatherServer.set_rain_volume_active_distance(world_3d, 10.0)
    var active_volumes: Array = WeatherServer._get_active_rain_volumes(world_3d)
    WeatherServer.clear_weather_state(world_3d)

    assert_true(active_volumes.has(near_volume))
    assert_false(active_volumes.has(far_volume))
