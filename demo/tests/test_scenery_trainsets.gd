extends MaszynaGutTest

## A loaded scenery places its trainsets and vehicles through the servers, without a node for
## either: MaszynaLegacyVehicleSystem builds the vehicles, RailVehicleServer stands each trainset
## on its track and couples it, and the scenery holds their handles until it unloads.

const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
## A trainset "express" of `first` (with a driver) and `second` on main_track, `lone` on
## side_track, and `road_car` on a track the scenery does not have
const SCENERY:String = "trainsets.scn"
const LOAD_TIMEOUT:float = 30.0
## How exactly the second vehicle stands a vehicle's length behind the first [m]
const PLACEMENT_TOLERANCE:float = 0.01

var _previous_game_dir:String = ""
var _scenery:MaszynaSceneryNode


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)
    _scenery = MaszynaSceneryNode.new()
    _scenery.filename = SCENERY
    _scenery.autoload = false
    add_child(_scenery)


func after_each() -> void:
    _scenery.free()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_a_loaded_scenery_has_no_node_for_a_vehicle_or_a_trainset() -> void:
    await _scenery.load()

    assert_eq(_scenery.find_children("", "RailVehicle3D", true, false).size(), 0)
    assert_eq(_scenery.find_children("", "TrainSet3D", true, false).size(), 0)
    assert_eq(_names(_scenery.get_vehicles()), ["first", "second", "lone"],
            "its vehicles in the order of the file, without the one on a track that is not built")
    assert_eq(_scenery.get_trainsets().size(), 2, "the trainset, and the dynamic outside one")
    for vehicle:RID in _scenery.get_vehicles():
        assert_true(RailVehicleRenderingServer.vehicle_is_attached(vehicle), "drawn without a node")


func test_the_trainset_stands_its_vehicles_in_order_and_couples_them() -> void:
    await _scenery.load()

    var trainset:RID = _scenery.get_trainsets()[0]
    assert_eq(RailVehicleServer.trainset_get_name(trainset), "express")
    var vehicles:Array[RID] = []
    vehicles.assign(RailVehicleServer.trainset_get_vehicles(trainset))
    assert_eq(_names(vehicles), ["first", "second"])
    var coupled:Array[RID] = []
    coupled.assign(RailVehicleServer.vehicle_get_coupled(
            vehicles[0], RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER))
    assert_eq(coupled.size(), 2, "the two are coupled")
    assert_almost_eq(
            RailVehicleServer.vehicle_get_transform(vehicles[0]).origin.distance_to(
                    RailVehicleServer.vehicle_get_transform(vehicles[1]).origin),
            VehicleServer.vehicle_get_dimensions(vehicles[0]).z, PLACEMENT_TOLERANCE,
            "the second stands right behind the first")


func test_the_vehicle_with_somebody_aboard_gets_its_driver() -> void:
    await _scenery.load()

    assert_eq(_scenery.first_train_id, "first", "the player belongs in the vehicle with a driver")
    assert_true(DriverSystem.vehicle_get_driver(VehicleServer.vehicle_get_rid_by_name("first")).is_valid())
    assert_false(DriverSystem.vehicle_get_driver(VehicleServer.vehicle_get_rid_by_name("second")).is_valid())


func test_a_skin_chosen_for_the_load_is_that_vehicles_alone() -> void:
    _scenery.skin_overrides = {"first": "chosen"} as Dictionary[String, String]
    await _scenery.load()

    var vehicles:Array[RID] = _scenery.get_vehicles()
    assert_eq(MaszynaLegacyVehicleSystem.vehicle_get_dynamic(vehicles[0]).skin, "chosen")
    assert_eq(MaszynaLegacyVehicleSystem.vehicle_get_dynamic(vehicles[1]).skin, "none")


func test_unloading_frees_the_vehicles_and_the_trainsets() -> void:
    await _scenery.load()
    var vehicles:Array[RID] = _scenery.get_vehicles().duplicate()

    _scenery.filename = ""
    await _scenery.load()

    assert_eq(_scenery.get_vehicles().size(), 0)
    assert_eq(_scenery.get_trainsets().size(), 0)
    for vehicle:RID in vehicles:
        assert_false(MaszynaLegacyVehicleSystem.vehicle_exists(vehicle))
    assert_false(VehicleServer.vehicle_get_rid_by_name("first").is_valid())


func _names(vehicles:Array[RID]) -> Array:
    return vehicles.map(func(vehicle:RID) -> String: return VehicleServer.vehicle_get_name(vehicle))
