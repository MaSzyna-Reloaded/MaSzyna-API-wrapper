extends MaszynaGutTest

const TEST_GAME_DIR:String = "user://gut/vehicle_template"
const DATA_PATH:String = "fixtures"
const FILE_NAME:String = "test_vehicle"
var _previous_game_dir:String
var _vehicles:Array[RailVehicle3D] = []


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    var fixture_dir:String = TEST_GAME_DIR.path_join(DATA_PATH)
    DirAccess.make_dir_recursive_absolute(fixture_dir)
    var mmd:FileAccess = FileAccess.open(fixture_dir.path_join(FILE_NAME + ".mmd"), FileAccess.WRITE)
    mmd.store_string("models: test_vehicle.t3d\n")
    mmd.close()
    var fiz:FileAccess = FileAccess.open(fixture_dir.path_join(FILE_NAME + ".fiz"), FileAccess.WRITE)
    fiz.store_string(FileAccess.get_file_as_string("res://tests/fixtures/test_vehicle.fiz"))
    fiz.close()
    UserSettings.save_maszyna_game_dir(TEST_GAME_DIR)


func after_each() -> void:
    for vehicle:RailVehicle3D in _vehicles:
        vehicle.free()
    _vehicles.clear()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)
    await wait_idle_frames(2)


func test_cached_vehicles_have_independent_registered_sound_pools() -> void:
    var initial_banks:int = TrainSoundSystem._banks.size()
    for index:int in range(2):
        var vehicle:RailVehicle3D = DynamicRailVehicle3DManager.load(
            DATA_PATH, FILE_NAME, "", "template_test_%s" % index, 0.0, null)
        _vehicles.append(vehicle)
        var model:E3DModelInstance = vehicle.get_node("ExteriorModel")
        model.model = E3DModel.new()
        add_child(vehicle)
        await wait_idle_frames(3)
        assert_eq(TrainSoundSystem._banks.size(), initial_banks + 2 * (index + 1))
        for player_name:String in ["ExteriorSfxPlayer3D", "CabinSfxPlayer3D"]:
            var player:SfxPlayer3D = vehicle.get_node(player_name)
            assert_eq(player.get_child_count(), 0, "runtime voices should be allocated lazily")
            var registration:Variant = TrainSoundSystem._banks[player.get_instance_id()]
            assert_same(registration.vehicle, vehicle)
            assert_same(registration.controller, vehicle.get_controller())
        assert_eq(vehicle.get_controller().train_id, "template_test_%s" % index)
    var first_player:SfxPlayer3D = _vehicles[0].get_node("ExteriorSfxPlayer3D")
    var second_player:SfxPlayer3D = _vehicles[1].get_node("ExteriorSfxPlayer3D")
    assert_ne(first_player.bank, second_player.bank)
    for vehicle:RailVehicle3D in _vehicles:
        vehicle.free()
    _vehicles.clear()
    await wait_idle_frames(2)
    assert_eq(TrainSoundSystem._banks.size(), initial_banks)
