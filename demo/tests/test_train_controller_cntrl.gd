extends MaszynaGutTest

var train: VehicleController

func before_each():
    train = build_vehicle("TestTrain")
    await wait_idle_frames(2)

func test_defaults():
    assert_eq(train.cntrl_battery_start_mode, VehicleController.START_MODE_MANUAL)
    assert_true(train.cntrl_automatic_cab_activation)
    assert_eq(train.cntrl_inactive_cab_flag, 0)

func test_round_trip_and_update_without_crashing():
    train.cntrl_battery_start_mode = VehicleController.START_MODE_AUTOMATIC
    train.cntrl_ground_relay_start_mode = VehicleController.START_MODE_AUTOMATIC
    train.cntrl_compartment_lights_start_mode = VehicleController.START_MODE_MANUAL
    train.cntrl_automatic_cab_activation = false
    train.cntrl_inactive_cab_flag = 1 | 32 # emergency brake + apply spring brake
    await wait_idle_frames(2)

    assert_eq(train.cntrl_battery_start_mode, VehicleController.START_MODE_AUTOMATIC)
    assert_false(train.cntrl_automatic_cab_activation)
    assert_eq(train.cntrl_inactive_cab_flag, 33)
    assert_true(train.state.has("mass_total"), "VehicleController should keep functioning after configuring the Cntrl. section")
