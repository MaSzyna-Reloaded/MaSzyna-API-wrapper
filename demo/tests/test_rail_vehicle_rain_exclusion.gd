extends MaszynaGutTest

const REAL_GAME_DIR: String = "/home/marcin/Games/Maszyna"

var _previous_game_dir: String


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()


func after_each() -> void:
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_fits_rain_exclusion_to_fiz_dimensions() -> void:
    var controller: VehicleController = autofree(VehicleController.new())
    var rain_volume: RainVolume = autofree(RainVolume.new())
    controller.dimensions_length = 14.24
    controller.dimensions_width = 3.1
    controller.dimensions_height = 4.4

    MaszynaRailVehicle3DInstancer._fit_rain_volume(controller, rain_volume)

    assert_eq(rain_volume.size, Vector3(3.1, 4.4, 14.24))
    assert_almost_eq(rain_volume.position.y, 2.2, 0.000001)
    assert_almost_eq(rain_volume.precipitation_delta, -1.0, 0.000001)


func test_real_vehicle_excludes_rain_over_its_body() -> void:
    if not DirAccess.dir_exists_absolute(REAL_GAME_DIR.path_join("dynamic/pkp/sm42_v1")):
        pending("real SM42 game data not available on this machine at %s" % REAL_GAME_DIR)
        return
    UserSettings.save_maszyna_game_dir(REAL_GAME_DIR)
    var vehicle: RailVehicle3D = DynamicRailVehicle3DManager.load(
        "dynamic/pkp/sm42_v1", "6da", "6d-907", "test_sm42_rain", 0.0, null
    )
    add_child_autofree(vehicle)
    var fiz_controller: FizVehiclePhysicsNode = vehicle.get_node("FizVehiclePhysicsNode") as FizVehiclePhysicsNode
    await wait_idle_frames(2)

    var controller: VehicleController = fiz_controller.get_controller()
    var rain_volume: RainVolume = vehicle.get_node("RainExclusion") as RainVolume
    assert_not_null(controller)
    assert_gt(controller.dimensions_length, 0.0)
    assert_eq(
        rain_volume.size,
        Vector3(
            controller.dimensions_width,
            controller.dimensions_height,
            controller.dimensions_length
        )
    )
    assert_almost_eq(rain_volume.precipitation_delta, -1.0, 0.000001)


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
