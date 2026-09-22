extends MaszynaGutTest

## LegacyCabinCabActivation: a cab whose MMD has no cabactivation_sw gauge (dynamic/pkp/e186_v2)
## is still activated through CabinSystem, as TTrain::OnCommand_cabactivationtoggle does regardless
## of the gauge (Train.cpp:3077).

var train: VehicleController
var cabin: Node3D


func before_each():
    train = build_vehicle("TestCabActivation")
    train.battery_voltage = 110.0
    train.apply_configuration()
    # a cabin with no controls at all
    cabin = Node3D.new()
    add_child(cabin)
    var logic: LegacyCabinLogicDelegate = LegacyCabinLogicDelegate.new()
    logic.train_id = train.train_id
    logic.cab = 1
    cabin.add_child(logic)
    await wait_idle_frames(2)


func after_each():
    remove_child(cabin)
    cabin.free()


func test_cab_without_the_gauge_registers_the_control():
    assert_true(CabinSystem.has_control(train.train_id, 1, &"cabactivation_sw"))


func test_toggle_activates_and_deactivates_the_cab():
    assert_eq(train.state["cabin"], 0)

    CabinSystem.act(train.train_id, 1, &"cabactivation_sw", &"toggle")
    await wait_idle_frames(2)
    assert_eq(train.state["cabin"], 1)

    CabinSystem.act(train.train_id, 1, &"cabactivation_sw", &"toggle")
    await wait_idle_frames(2)
    assert_eq(train.state["cabin"], 0)
