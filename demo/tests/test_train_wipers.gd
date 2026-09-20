extends MaszynaGutTest

## TrainWipers: the wiper switch (Train.cpp:2638-2661) and the movement of the wipers
## (DynObj.cpp:4048-4115), both kept in the node - the vendored Mover has neither.

var train: TrainController
var wipers: TrainWipers


func before_each():
    train = TrainController.new()
    train.train_id = "TestTrainWipers"
    train.battery_voltage = 110.0
    wipers = TrainWipers.new()
    # WiperList: of ep09_v2/104e-mod-dod-zal.fiz - mask, sweep time, interval, delay at the far end
    wipers.positions = [
        _item(0, 1.0, 0.0, 0.5),
        _item(3, 1.0, 5.0, 0.5),
        _item(3, 0.2, 0.0, 0.1),
    ]
    train.add_child(wipers)
    add_child(train)
    # a TrainPart publishes its state with its first processed frame
    await wait_idle_frames(2)


func after_each():
    remove_child(train)
    train.free()


func _item(mask: int, transit_time: float, period: float, return_delay: float) -> WiperListItem:
    var item: WiperListItem = WiperListItem.new()
    item.wiper_mask = mask
    item.transit_time = transit_time
    item.period = period
    item.return_delay = return_delay
    return item


func test_defaults():
    var defaults: TrainWipers = TrainWipers.new()

    assert_eq(defaults.angle, 0.0)
    assert_eq(defaults.default_position, 0)
    assert_eq(defaults.positions.size(), 0)
    defaults.free()


func test_round_trip_and_update_without_crashing():
    wipers.angle = 58.0
    wipers.default_position = 2
    wipers.positions = [
        _item(0, 0.7, 0.0, 0.5),
        _item(3, 0.7, 7.0, 0.5),
        _item(3, 0.7, 0.7, 0.0),
        _item(3, 0.5, 0.5, 0.0),
    ]
    await wait_idle_frames(2)

    assert_eq(wipers.angle, 58.0)
    assert_eq(wipers.positions.size(), 4)
    assert_eq((wipers.positions[1] as WiperListItem).period, 7.0)
    assert_true(is_instance_valid(train), "TrainController should keep functioning after configuring TrainWipers")


func test_switch_is_limited_to_the_wiper_list():
    assert_eq(train.state["wipers_switch_position"], 0)
    assert_eq(train.config["wipers_switch_position_max"], 2)

    for i in 5:
        train.send_command("wipers_switch_increase")
    await wait_idle_frames(2)
    assert_eq(train.state["wipers_switch_position"], 2)

    for i in 5:
        train.send_command("wipers_switch_decrease")
    await wait_idle_frames(2)
    assert_eq(train.state["wipers_switch_position"], 0)


func test_wiper_count_comes_from_the_masks():
    assert_eq(train.state["wiper_positions"].size(), 2)


func test_wipers_sweep_out_and_back_with_active_cab_and_battery():
    train.send_command("battery", true)
    train.send_command("cab_activation", true)
    train.send_command("wipers_switch_increase")
    train.send_command("wipers_switch_increase")

    var reached_out: bool = false
    var reached_return: bool = false
    # sweep 0.2 s + 0.1 s at the far end; polled by time, a headless frame takes no time at all
    for i in 40:
        await wait_seconds(0.05)
        var position: float = train.state["wiper_positions"][0]
        reached_out = reached_out or (position > 0.0 and position <= 1.0)
        reached_return = reached_return or position > 1.0
        if reached_out and reached_return:
            break

    assert_true(reached_out, "wiper should sweep out (0..1)")
    assert_true(reached_return, "wiper should come back (1..2)")


func test_wipers_stay_parked_without_battery():
    train.send_command("cab_activation", true)
    train.send_command("wipers_switch_increase")
    train.send_command("wipers_switch_increase")
    await wait_idle_frames(10)

    assert_eq(train.state["wiper_positions"][0], 0.0)


## e186_v2 has four wipers, two per end, and a list that switches wipers 1 and 2: from cab 1 these
## are the first two, the wipers of the other end stay parked (DynObj.cpp:4056-4063)
func test_wiper_count_of_the_model_limits_the_sweep_to_the_active_end():
    wipers.wiper_count = 4
    wipers.update_mover()
    train.send_command("battery", true)
    train.send_command("cab_activation", true)
    train.send_command("wipers_switch_increase")
    train.send_command("wipers_switch_increase")
    await wait_seconds(0.15)

    var positions: PackedFloat64Array = train.state["wiper_positions"]
    assert_eq(positions.size(), 4)
    assert_gt(positions[0], 0.0)
    assert_gt(positions[1], 0.0)
    assert_eq(positions[2], 0.0)
    assert_eq(positions[3], 0.0)
