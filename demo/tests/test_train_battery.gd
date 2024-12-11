extends MaszynaGutTest

const RAIL_VEHICLE_3D = preload("res://addons/libmaszyna/rail_vehicle_3d.gd")

var train: RailVehicle3D
var controller: TrainController

func before_each():
    train = RAIL_VEHICLE_3D.new() as RailVehicle3D
    controller = TrainController.new()
    controller.name = "Controller"
    controller.battery_voltage = 110.0
    train.controller_path = NodePath("Controller")
    train.train_id = "TestTrain"
    train.add_child(controller)
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

    controller.battery_voltage = 0.0
    await wait_idle_frames(2)

    train.send_command("battery", true)
    assert_false(train.state["battery_enabled"], "Battery should be disabled")

func test_successful_battery_voltage_drop_after_two_seconds():
    var before = train.state["battery_voltage"]
    await wait_seconds(2)
    var after = train.state["battery_voltage"]

    assert_true(before > after, "There should be a battery voltage drop after 2 seconds")
