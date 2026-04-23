extends MaszynaGutTest

const RAIL_VEHICLE_3D = preload("res://addons/libmaszyna/rail_vehicle_3d.gd")

var train: RailVehicle3D

func before_each():
    var controller:TrainController = load("res://tests/sm42_controller.tscn").instantiate()
    train = RAIL_VEHICLE_3D.new() as RailVehicle3D
    train.controller_path = NodePath(controller.name)
    train.train_id = "TestTrain"
    controller.battery_voltage = 110.0
    train.add_child(controller)
    add_child(train)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_successful_ep_fuse_enabling():
    train.send_command("switch_ep_fuse", true)
    await wait_idle_frames(2)
    print(train.state)
    assert_true(train.state["dcemued/ep_fuse"], "EP Fuse should be enabled")

func test_successful_ep_fuse_disabling():
    train.send_command("switch_ep_fuse", false)
    await wait_idle_frames(2)
    assert_false(train.state["dcemued/ep_fuse"], "EP Fuse should be disabled")
