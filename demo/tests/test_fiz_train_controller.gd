extends MaszynaGutTest

const TEST_GAME_DIR: String = "user://gut/fiz_train_controller"
const FIXTURE_SOURCE_PATH := "res://tests/fixtures/test_vehicle.fiz"
const FIXTURE_DATA_PATH := "fixtures"
const FIXTURE_FILENAME := "test_vehicle"

var node: FIZTrainController
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

    node = FIZTrainController.new()
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

    var controller: TrainController = node.get_controller()
    assert_not_null(controller)
    # default (non-editable) children are added INTERNAL, so they don't show up in the plain
    # (non-internal) child count or the Scene dock - see FIZTrainController.editable_in_editor.
    assert_eq(node.get_child_count(), 0, "the built TrainController should be internal by default")
    assert_eq(node.get_child_count(true), 1, "the built TrainController should still be reachable as an internal child")
    assert_eq(controller.get_mass(), 74000.0)
    assert_not_null(controller.get_node_or_null("TrainWheels"))
    assert_not_null(controller.get_node_or_null("TrainBrake"))
    assert_not_null(controller.get_node_or_null("TrainDoors"))
    assert_not_null(controller.get_node_or_null("TrainBuffCoupl"))


func test_native_mover_still_updates():
    _set_fixture_path()
    await wait_idle_frames(3)

    var controller: TrainController = node.get_controller()
    assert_true(controller.state.has("velocity"), "TrainController's native state dictionary should populate")
    assert_true(controller.state.has("brake_air_pressure"), "TrainBrake's mover state should be live")


func test_editable_in_editor_toggles_internal_mode():
    _set_fixture_path()
    await wait_idle_frames(2)
    assert_eq(node.get_child_count(), 0, "should be internal by default")

    node.editable_in_editor = true
    await wait_idle_frames(2)
    assert_eq(node.get_child_count(), 1, "should no longer be internal once editable")
    assert_not_null(node.get_controller(), "reload triggered by the toggle should preserve the controller")


func test_reassigning_fiz_filename_clears_previous_controller():
    _set_fixture_path()
    await wait_idle_frames(2)
    assert_not_null(node.get_controller())

    node.fiz_filename = ""
    await wait_idle_frames(2)
    assert_null(node.get_controller())
    assert_eq(node.get_child_count(), 0)
