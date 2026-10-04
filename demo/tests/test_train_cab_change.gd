extends MaszynaGutTest

## Cab switching - original engine: TTrain::CabChange() (Train.cpp:8516) stepping
## TMoverParameters::ChangeCab() (Mover.cpp:779) 1 -> 0 (machine room) -> -1.

var train:VehicleController


func before_each():
    train = build_vehicle("TestCabChangeTrain", null, 0.0, VehicleController.DRIVER_HEAD)
    # the active cab is the master controller's - a vehicle with a cab has one
    train.add_component(MoverRailVehicleMasterController.new())


## No cab is active until the crew switches it on (CabActive = 0, MOVER.h:2090; Train.cpp:2430).
func test_unmanned_vehicle_starts_in_cab_one_with_inactive_cab():
    assert_eq(train.get_state()["cabin_occupied"], 1)
    assert_eq(train.get_state()["cabin"], 0)

    train.send_command("cab_activation", true)
    train.update_state()
    assert_eq(train.get_state()["cabin"], 1)
    assert_true(train.get_state()["cabin_controleable"])

    train.send_command("cab_activation", false)
    train.update_state()
    assert_eq(train.get_state()["cabin"], 0)


func test_cab_change_backward_goes_through_machine_room():
    watch_signals(train)
    train.send_command("cab_change", -1)
    train.update_state()
    assert_eq(train.get_state()["cabin_occupied"], 0)
    assert_eq(train.get_state()["cabin"], 0)
    assert_signal_emitted_with_parameters(train, "cabin_occupied_changed", [0])

    train.send_command("cab_change", -1)
    train.update_state()
    assert_eq(train.get_state()["cabin_occupied"], -1)
    assert_eq(train.get_state()["cabin"], -1)
    assert_signal_emitted_with_parameters(train, "cabin_occupied_changed", [-1])


func test_cab_change_stops_at_vehicle_end():
    train.send_command("cab_change", 1)
    train.update_state()
    assert_eq(train.get_state()["cabin_occupied"], 1)

    for i in range(3):
        train.send_command("cab_change", -1)
    train.update_state()
    assert_eq(train.get_state()["cabin_occupied"], -1)

    train.send_command("cab_change", 1)
    train.send_command("cab_change", 1)
    train.update_state()
    assert_eq(train.get_state()["cabin_occupied"], 1)
    assert_eq(train.get_state()["cabin"], 1)


func test_starts_in_cab_two_for_rear_driver():
    var physics_node: RailVehiclePhysicsNode = RailVehiclePhysicsNode.new()
    physics_node.vehicle_id = "TestCabChangeRearTrain"
    physics_node.driver_type = VehicleController.DRIVER_REAR
    add_child_autofree(physics_node)
    var rear_train: VehicleController = VehicleServer.vehicle_get_controller(physics_node.get_vehicle_rid())
    rear_train.add_component(MoverRailVehicleMasterController.new())

    assert_eq(rear_train.get_state()["cabin_occupied"], -1)
    assert_eq(rear_train.get_state()["cabin"], 0)

    rear_train.send_command("cab_activation", true)
    rear_train.update_state()
    assert_eq(rear_train.get_state()["cabin"], -1)

