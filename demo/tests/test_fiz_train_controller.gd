extends MaszynaGutTest

const TEST_GAME_DIR: String = "user://gut/fiz_train_controller"
const FIXTURE_SOURCE_PATH := "res://tests/fixtures/test_vehicle.fiz"
const FIXTURE_DATA_PATH := "fixtures"
const FIXTURE_FILENAME := "test_vehicle"

var node: FizVehiclePhysicsNode
var _previous_game_dir: String = ""
var _temp_fiz_dir: String
var _temp_fiz_path: String


func before_each():
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    DirAccess.make_dir_recursive_absolute(TEST_GAME_DIR)
    UserSettings.save_maszyna_game_dir(TEST_GAME_DIR)
    _temp_fiz_dir = TEST_GAME_DIR.path_join(FIXTURE_DATA_PATH)
    DirAccess.make_dir_recursive_absolute(_temp_fiz_dir)
    _temp_fiz_path = _temp_fiz_dir.path_join(FIXTURE_FILENAME + ".fiz")
    var src := FileAccess.open(FIXTURE_SOURCE_PATH, FileAccess.READ)
    var dst := FileAccess.open(_temp_fiz_path, FileAccess.WRITE)
    dst.store_buffer(src.get_buffer(src.get_length()))
    src.close()
    dst.close()

    node = FizVehiclePhysicsNode.new()
    add_child(node)
    await wait_idle_frames(2)


func after_each():
    remove_child(node)
    node.free()
    if FileAccess.file_exists(_temp_fiz_path):
        DirAccess.remove_absolute(_temp_fiz_path)
    DirAccess.remove_absolute(_temp_fiz_dir)
    DirAccess.remove_absolute(TEST_GAME_DIR)
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func _set_fixture_path() -> void:
    node.data_path = FIXTURE_DATA_PATH
    node.fiz_filename = FIXTURE_FILENAME


func test_builds_child_controller_from_data_path_and_filename():
    _set_fixture_path()
    await wait_idle_frames(2)

    var controller: VehicleController = node.get_controller()
    assert_not_null(controller)
    assert_eq(node.get_child_count(), 0, "the vehicle is the node's own, not a child of it")
    assert_eq(controller.mass, 74000.0)
    assert_not_null(controller.get_component(VehicleComponentType.COMPONENT_WHEELS))
    assert_not_null(controller.get_component(VehicleComponentType.COMPONENT_BRAKES))
    assert_not_null(controller.get_component(VehicleComponentType.COMPONENT_DOORS))
    assert_not_null(controller.get_component(VehicleComponentType.COMPONENT_BUFFERS))


func test_native_mover_still_updates():
    _set_fixture_path()
    await wait_idle_frames(3)

    var controller: VehicleController = node.get_controller()
    assert_true(controller.state.has("velocity"), "VehicleController's native state dictionary should populate")
    assert_true(controller.state.has("brake_air_pressure"), "VehicleBrake's mover state should be live")


## Clearing the file leaves the node holding an empty vehicle rather than the previous one -
## a VehiclePhysicsNode always has a vehicle, it just has nothing in it.
func test_clearing_the_fiz_filename_empties_the_vehicle():
    _set_fixture_path()
    await wait_idle_frames(2)
    assert_not_null(node.get_controller().get_component(VehicleComponentType.COMPONENT_BRAKES))

    node.fiz_filename = ""
    await wait_idle_frames(2)
    assert_not_null(node.get_controller(), "the vehicle is still there")
    assert_null(
        node.get_controller().get_component(VehicleComponentType.COMPONENT_BRAKES),
        "with nothing in it"
    )
