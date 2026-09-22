extends MaszynaGutTest

## Cab switching - original engine: TTrain::CabChange() (Train.cpp:8516) stepping
## TMoverParameters::ChangeCab() (Mover.cpp:779) 1 -> 0 (machine room) -> -1.

var train:VehicleController


func before_each():
    train = build_vehicle("TestCabChangeTrain")


## An unmanned vehicle keeps its cab inactive (the original activates only a driven one,
## Driver.cpp:2126) until the driver activates it (Train.cpp:2430).
func test_unmanned_vehicle_starts_in_cab_one_with_inactive_cab():
    assert_eq(train.state["cabin_occupied"], 1)
    assert_eq(train.state["cabin"], 0)

    train.send_command("cab_activation", true)
    train.update_state()
    assert_eq(train.state["cabin"], 1)
    assert_true(train.state["cabin_controleable"])

    train.send_command("cab_activation", false)
    train.update_state()
    assert_eq(train.state["cabin"], 0)


func test_cab_change_backward_goes_through_machine_room():
    watch_signals(train)
    train.send_command("cab_change", -1)
    train.update_state()
    assert_eq(train.state["cabin_occupied"], 0)
    assert_eq(train.state["cabin"], 0)
    assert_signal_emitted_with_parameters(train, "cabin_occupied_changed", [0])

    train.send_command("cab_change", -1)
    train.update_state()
    assert_eq(train.state["cabin_occupied"], -1)
    assert_eq(train.state["cabin"], -1)
    assert_signal_emitted_with_parameters(train, "cabin_occupied_changed", [-1])


func test_cab_change_stops_at_vehicle_end():
    train.send_command("cab_change", 1)
    train.update_state()
    assert_eq(train.state["cabin_occupied"], 1)

    for i in range(3):
        train.send_command("cab_change", -1)
    train.update_state()
    assert_eq(train.state["cabin_occupied"], -1)

    train.send_command("cab_change", 1)
    train.send_command("cab_change", 1)
    train.update_state()
    assert_eq(train.state["cabin_occupied"], 1)
    assert_eq(train.state["cabin"], 1)


func test_starts_in_cab_two_for_rear_driver():
    var physics_node: VehiclePhysicsNode = VehiclePhysicsNode.new()
    physics_node.train_id = "TestCabChangeRearTrain"
    physics_node.cabin_number = -1
    add_child_autofree(physics_node)
    var rear_train: VehicleController = physics_node.get_controller()

    assert_eq(rear_train.state["cabin_occupied"], -1)
    assert_eq(rear_train.state["cabin"], -1)

