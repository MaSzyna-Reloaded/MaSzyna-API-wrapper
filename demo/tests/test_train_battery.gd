extends MaszynaGutTest

var train: VehicleController

## The battery voltage is configuration the Mover reads while the vehicle is being built, so it
## is authored into the model - writing it afterwards does not reach the backend until a step
## (see test_battery_start_disabled_from_zero_voltage_blocks_switching).
func _model(battery_voltage:float) -> VehicleModel:
    var model:VehicleModel = VehicleModel.new()
    model.properties = {"battery_voltage": battery_voltage}
    return model


func before_each():
    train = build_vehicle("TestTrain", _model(110.0))

# Original engine: Battery defaults to false and CheckLocomotiveParameters() only turns it on
# for a vehicle spawned ready to depart (Mover.cpp:8943), i.e. with a non-zero scenery velocity
# (DynObj.cpp:1851, `driveractive = (fVel != 0.0)`).
func test_battery_starts_off_when_not_ready_to_depart():
    assert_false(train.state["battery_enabled"], "Battery should start off for initial_velocity == 0")

func test_battery_starts_on_when_ready_to_depart():
    var ready_train: VehicleController = build_vehicle("TestTrainReady", _model(110.0), 10.0)

    assert_true(ready_train.state["battery_enabled"], "Battery should start on for initial_velocity != 0")


func test_successful_battery_enabling():
    train.send_command("battery", true)
    assert_true(train.state["battery_enabled"], "Battery should be enabled")

func test_battery_start_disabled_from_zero_voltage_blocks_switching():
    # Original engine: CheckLocomotiveParameters() (Mover.cpp) forces BatteryStart to Disabled
    # when NominalBatteryVoltage is 0 - a load-time FIZ misconfiguration guard, not something a
    # real vehicle's voltage changes into at runtime. So this must be set before the vehicle
    # ever initializes, not mutated afterward (Battery itself, once on, isn't retroactively
    # switched off by a later voltage change - only BatterySwitch()'s manual-mode gate is).
    var disabled_train: VehicleController = build_vehicle("TestTrainZeroVoltage", _model(0.0))

    assert_false(disabled_train.state["battery_enabled"], "Battery should start off when BatteryStart is forced Disabled")
    disabled_train.send_command("battery", true)
    assert_false(disabled_train.state["battery_enabled"], "BatterySwitch should have no effect while BatteryStart is Disabled")


func test_successful_battery_voltage_drop_after_two_seconds():
    train.send_command("battery", true)
    await wait_idle_frames(2)
    var before = train.state["battery_voltage"]
    await wait_seconds(2)
    var after = train.state["battery_voltage"]

    assert_true(before > after, "There should be a battery voltage drop after 2 seconds")
