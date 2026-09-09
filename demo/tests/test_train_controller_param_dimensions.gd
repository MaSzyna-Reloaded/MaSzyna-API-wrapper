extends MaszynaGutTest

var train: TrainController

func before_each():
    train = TrainController.new()
    train.train_id = "TestTrain"
    add_child(train)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_param_and_dimensions_defaults():
    assert_eq(train.category, TrainController.CATEGORY_TRAIN)
    assert_eq(train.train_type, TrainController.TRAIN_TYPE_DEFAULT)
    assert_eq(train.reduced_mass, 0.0)
    assert_eq(train.sand_capacity, 0.0)
    assert_eq(train.heating_power, 0.0)
    assert_eq(train.light_power, 0.0)
    assert_eq(train.cabin_count, 0)
    assert_eq(train.length, 0.0)
    assert_eq(train.height, 0.0)
    assert_eq(train.width, 0.0)
    assert_eq(train.drag_coefficient, 0.0)
    assert_eq(train.floor_height, 0.96)

func test_param_and_dimensions_round_trip_and_update():
    train.category = TrainController.CATEGORY_ROAD
    train.train_type = TrainController.TRAIN_TYPE_ET22
    train.reduced_mass = 500.0
    train.sand_capacity = 300.0
    train.heating_power = 20.0
    train.light_power = 0.56
    train.length = 15.5
    train.height = 4.3
    train.width = 2.9
    train.drag_coefficient = 1.2
    train.floor_height = 1.1
    train.cabin_count = 2
    await wait_idle_frames(2)

    assert_eq(train.category, TrainController.CATEGORY_ROAD)
    assert_eq(train.train_type, TrainController.TRAIN_TYPE_ET22)
    assert_eq(train.reduced_mass, 500.0)
    assert_eq(train.sand_capacity, 300.0)
    assert_eq(train.heating_power, 20.0)
    assert_eq(train.light_power, 0.56)
    assert_eq(train.length, 15.5)
    assert_eq(train.height, 4.3)
    assert_eq(train.width, 2.9)
    assert_eq(train.drag_coefficient, 1.2)
    assert_almost_eq(train.floor_height, 1.1, 0.001)
    assert_eq(train.cabin_count, 2)
    assert_true(train.state.has("mass_total"), "TrainController should keep functioning after configuring Param/Dimensions")


func test_cabin_count_is_limited_to_supported_range():
    train.cabin_count = -1
    assert_eq(train.cabin_count, 0)
    train.cabin_count = 3
    assert_eq(train.cabin_count, 2)
