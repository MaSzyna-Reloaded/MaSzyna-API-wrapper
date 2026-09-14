extends MaszynaGutTest

var train: TrainController

func before_each():
    train = TrainController.new()
    train.train_id = "TestTrain"
    train.battery_voltage = 110.0
    add_child(train)

func after_each():
    remove_child(train)
    train.free()

# Original engine: Mover.cpp's LoadFIZ sets Battery live at load time whenever BatteryStart
# isn't Disabled (TrainController's compiled default is Manual, matching the FIZ-absent
# fallback) - "Manual" means the player can turn it off, not that it starts off.
func test_battery_should_be_initially_turned_on_for_manual_start():
    assert_true(train.state["battery_enabled"], "Battery should start on for BatteryStart=Manual")

func test_successful_battery_enabling():
    train.send_command("battery", true)
    assert_true(train.state["battery_enabled"], "Battery should be enabled")

func test_battery_start_disabled_from_zero_voltage_blocks_switching():
    # Original engine: CheckLocomotiveParameters() (Mover.cpp) forces BatteryStart to Disabled
    # when NominalBatteryVoltage is 0 - a load-time FIZ misconfiguration guard, not something a
    # real vehicle's voltage changes into at runtime. So this must be set before the vehicle
    # ever initializes, not mutated afterward (Battery itself, once on, isn't retroactively
    # switched off by a later voltage change - only BatterySwitch()'s manual-mode gate is).
    var disabled_train := TrainController.new()
    disabled_train.train_id = "TestTrainZeroVoltage"
    disabled_train.battery_voltage = 0.0
    add_child(disabled_train)

    assert_false(disabled_train.state["battery_enabled"], "Battery should start off when BatteryStart is forced Disabled")
    disabled_train.send_command("battery", true)
    assert_false(disabled_train.state["battery_enabled"], "BatterySwitch should have no effect while BatteryStart is Disabled")

    remove_child(disabled_train)
    disabled_train.free()

func test_successful_battery_voltage_drop_after_two_seconds():
    var before = train.state["battery_voltage"]
    await wait_seconds(2)
    var after = train.state["battery_voltage"]

    assert_true(before > after, "There should be a battery voltage drop after 2 seconds")
