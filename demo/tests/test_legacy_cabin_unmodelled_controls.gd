extends MaszynaGutTest

## Cab logic of a cab that models none of the catalog controls (dynamic/pkp/e186_v2 lacks
## main_on_bt, dirkey, shp_reset_bt, cabactivation_sw, ...): the original runs every OnCommand_*
## without its gauge, here LegacyCabinUnmodelledControls registers them in CabinSystem.

var train: VehicleController
var cabin: Node3D


func before_each():
    train = VehicleController.new()
    train.train_id = "TestUnmodelledControls"
    train.battery_voltage = 110.0
    # the reverser does not move on a vehicle without a main controller (Mover.cpp DirectionForward)
    var engine: VehicleElectricSeriesEngine = VehicleElectricSeriesEngine.new()
    engine.cntrl_main_controller_position_count = 4
    train.add_child(engine)
    add_child(train)
    cabin = Node3D.new()
    add_child(cabin)
    var logic: LegacyCabinLogicDelegate = LegacyCabinLogicDelegate.new()
    logic.controller = train
    logic.cab = 1
    cabin.add_child(logic)
    await wait_idle_frames(2)


func after_each():
    remove_child(cabin)
    cabin.free()
    remove_child(train)
    train.free()


func test_catalog_controls_with_keys_are_registered_without_widgets():
    for control: StringName in [&"dirkey", &"main_on_bt", &"main_off_bt", &"shp_reset_bt", &"battery_sw"]:
        assert_true(CabinSystem.has_control(train.train_id, 1, control), "%s should be registered" % control)


func test_controls_sharing_keys_are_registered_once():
    # mainctrl and jointctrl both take main_controller_increase/decrease
    var registered: int = 0
    for control: StringName in [&"mainctrl", &"jointctrl"]:
        registered += 1 if CabinSystem.has_control(train.train_id, 1, control) else 0
    assert_eq(registered, 1)


func test_unmodelled_dirkey_steps_the_reverser():
    train.send_command("battery", true)
    train.send_command("cab_activation", true)
    await wait_idle_frames(2)

    CabinSystem.act(train.train_id, 1, &"dirkey", &"increase")
    await wait_idle_frames(2)
    assert_eq(train.state["direction"], 1)


func test_reverser_buttons_set_the_direction():
    train.send_command("battery", true)
    train.send_command("cab_activation", true)
    await wait_idle_frames(2)

    CabinSystem.act(train.train_id, 1, &"dirbackward_bt", &"hold")
    await wait_idle_frames(2)
    assert_eq(train.state["direction"], -1)

    CabinSystem.act(train.train_id, 1, &"dirforward_bt", &"hold")
    await wait_idle_frames(2)
    assert_eq(train.state["direction"], 1)

    CabinSystem.act(train.train_id, 1, &"dirneutral_bt", &"hold")
    await wait_idle_frames(2)
    assert_eq(train.state["direction"], 0)


## "cabin ... toggle" of the developer console sends no value
func test_toggle_without_value_does_not_fail():
    CabinSystem.act(train.train_id, 1, &"main_on_bt", &"toggle")
    assert_eq(CabinSystem.get_control(train.train_id, 1, &"main_on_bt"), true)
    CabinSystem.act(train.train_id, 1, &"main_on_bt", &"toggle")
    assert_eq(CabinSystem.get_control(train.train_id, 1, &"main_on_bt"), false)
    CabinSystem.act(train.train_id, 1, &"dirforward_bt", &"toggle")
    assert_eq(CabinSystem.get_control(train.train_id, 1, &"dirforward_bt"), true)
