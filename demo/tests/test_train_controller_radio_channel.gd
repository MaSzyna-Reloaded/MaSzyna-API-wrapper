extends MaszynaGutTest

## Regression coverage for a real bug found via the in-game console: radio_channel_set had no
## ClassDB::bind_method at all (register_command("radio_channel_set", Callable(this,
## "radio_channel_set")) silently built an invalid Callable), and radio_channel_min/max defaulted
## to 0/0 - a range the original engine never actually varies per vehicle
## (OnCommand_radiochannelset hardcodes std::clamp(..., 1, 10) for every vehicle) - so
## radio_channel_increase/decrease/set all silently clamped to a permanent 0 on every vehicle,
## since nothing anywhere sets radio_channel_min/max.

var train: TrainController

func before_each():
    train = load("res://tests/sm42_controller.tscn").instantiate()
    train.train_id = "TestTrain"
    train.battery_voltage = 110.0
    add_child(train)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_defaults_match_the_original_engines_universal_1_to_10_range():
    assert_eq(train.get("radio_channel/min"), 1)
    assert_eq(train.get("radio_channel/max"), 10)

func test_radio_channel_set_is_a_valid_bound_command():
    train.send_command("radio_channel_set", 5)
    await wait_idle_frames(2)
    assert_eq(train.state["radio_channel"], 5)

func test_radio_channel_increase_actually_changes_state():
    train.send_command("radio_channel_set", 3)
    await wait_idle_frames(2)
    train.send_command("radio_channel_increase")
    await wait_idle_frames(2)
    assert_eq(train.state["radio_channel"], 4)

func test_radio_channel_decrease_actually_changes_state():
    train.send_command("radio_channel_set", 3)
    await wait_idle_frames(2)
    train.send_command("radio_channel_decrease")
    await wait_idle_frames(2)
    assert_eq(train.state["radio_channel"], 2)
