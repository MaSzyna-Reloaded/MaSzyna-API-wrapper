extends MaszynaGutTest

## Cab switching - original engine: TTrain::CabChange() (Train.cpp:8516) stepping
## TMoverParameters::ChangeCab() (Mover.cpp:779) 1 -> 0 (machine room) -> -1.

var train:TrainController


func before_each():
    train = TrainController.new()
    train.train_id = "TestCabChangeTrain"
    add_child(train)


func after_each():
    remove_child(train)
    train.free()


func test_starts_in_cab_one_when_no_cab_is_occupied():
    assert_eq(train.state["cabin_occupied"], 1)
    assert_eq(train.state["cabin"], 1)


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
    var rear_train := TrainController.new()
    rear_train.train_id = "TestCabChangeRearTrain"
    rear_train.cabin_number = -1
    add_child(rear_train)

    assert_eq(rear_train.state["cabin_occupied"], -1)
    assert_eq(rear_train.state["cabin"], -1)

    remove_child(rear_train)
    rear_train.free()
