extends MaszynaGutTest

var train: VehicleController
var brake: VehicleBrake

func before_each():
    train = VehicleController.new()
    train.train_id = "TestTrain"
    add_child(train)

    brake = MoverVehicleBrake.new()
    train.add_child(brake)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_defaults():
    assert_eq(brake.compressor_emergency_valve_area, 0.0)
    assert_eq(brake.universal_brake_button_1, 0)
    assert_eq(brake.universal_brake_button_2, 0)
    assert_eq(brake.universal_brake_button_3, 0)

func test_round_trip_and_update_without_crashing():
    brake.compressor_emergency_valve_area = 1.5
    brake.universal_brake_button_1 = 1  # releaser
    brake.universal_brake_button_2 = 16 # anti-skid brake
    brake.universal_brake_button_3 = 8  # assimilation
    await wait_idle_frames(2)

    assert_eq(brake.compressor_emergency_valve_area, 1.5)
    assert_eq(brake.universal_brake_button_1, 1)
    assert_eq(brake.universal_brake_button_2, 16)
    assert_eq(brake.universal_brake_button_3, 8)
    assert_true(train.state.has("brake_air_pressure"), "VehicleBrake should keep functioning after configuring universal brake buttons")
