extends MaszynaGutTest

var train: TrainController

func before_each():
    train = load("res://tests/sm42_controller.tscn").instantiate()
    add_child(train)
    await wait_idle_frames(2)
    train.send_command("battery", true)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_successful_activate_spring_brake():
    train.send_command("set_spring_brake_active", true)
    await wait_idle_frames(2)
    assert_true(train.state["spring_brake/active"], "Spring brake should be active")

func test_successful_deactivate_spring_brake():
    train.send_command("set_spring_brake_active", false)
    await wait_idle_frames(2)
    assert_false(train.state["spring_brake/active"], "Spring brake should not be active")

func test_successful_enable_spring_brake():
    train.send_command("set_spring_brake_enabled", true)
    await wait_idle_frames(2)
    assert_true(train.state["spring_brake/shut_off"], "Spring brake should be enabled")

func test_successful_disable_spring_brake():
    train.send_command("set_spring_brake_enabled", false)
    await wait_idle_frames(2)
    assert_false(train.state["spring_brake/shut_off"], "Spring brake should not be enabled")
