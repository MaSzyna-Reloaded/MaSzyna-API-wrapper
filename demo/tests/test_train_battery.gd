extends MaszynaGutTest

var train: TrainNode

func before_each():
    train = TrainNode.new()
    train.train_id = "TestTrain"
    train.battery_voltage = 110.0
    add_child(train)

func after_each():
    remove_child(train)
    train.free()

# FIXME: BatteryStart mode is unhandled!!!
#func test_battery_should_be_initially_turned_off():
#    assert_false(train.state["battery_enabled"])

func test_successful_battery_enabling():
    train.send_command("battery", true)
    assert_true(train.state["battery_enabled"], "Battery should be enabled")

func test_not_enabling_battery_when_battery_voltage_is_zero():
    # This is a quite fun case: we're changing train's properties at runtime.
    # To make sure the original Mover is updating, you must wait two IDLE frames or more.
    # Alternatively you can call train.update_mover(), but this is an experimental method.
    #
    # This is not how tests should be written, but it shows how to handle similar cases.

    train.battery_voltage = 0.0
    await wait_idle_frames(2)

    train.send_command("battery", true)
    assert_false(train.state["battery_enabled"], "Battery should be disabled")

func test_successful_battery_voltage_drop_after_two_seconds():
    var before = train.state["battery_voltage"]
    await wait_seconds(2)
    var after = train.state["battery_voltage"]

    assert_true(before > after, "There should be a battery voltage drop after 2 seconds")
