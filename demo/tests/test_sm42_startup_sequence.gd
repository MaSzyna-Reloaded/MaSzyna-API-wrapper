extends MaszynaGutTest

var train: VehicleController

func before_each():
    # A startup sequence is a driver operating the loco, so the cab is occupied. Without it
    # CabActive stays 0 and TMoverParameters::ComputeTotalForce() switches the physics off once
    # LastSwitchingTime passes 5 s (Mover.cpp:4485) - the engine runs and the vehicle never moves.
    var physics_node: VehiclePhysicsNode = VehiclePhysicsNode.new()
    physics_node.train_id = "TestTrain"
    physics_node.driver_type = VehicleController.DRIVER_HEAD
    physics_node.set_model(load("res://tests/fixtures/sm42_vehicle.tres"))
    add_child_autofree(physics_node)
    train = physics_node.get_controller()
    await wait_idle_frames(2)
    train.send_command("battery", true)
    await wait_idle_frames(2)

func test_successful_enabling_oil_pump():
    train.send_command("oil_pump", true)
    await wait_idle_frames(2)
    assert_true(train.state["oil_pump_active"], "Oil pump should be active")

func test_successful_enabling_fuel_pump():
    train.send_command("fuel_pump", true)
    await wait_idle_frames(2)
    assert_true(train.state["fuel_pump_active"], "Fuel pump should be active")

func test_successful_pumping_the_oil():
    var before = train.state["oil_pump_pressure"]
    train.send_command("oil_pump", true)
    await wait_seconds(2)
    var after = train.state["oil_pump_pressure"]
    assert_true(after > before, "There should be a oil pump pressure increase after 2 seconds")

func test_successful_turning_engine_on():
    train.send_command("fuel_pump", true)
    await wait_seconds(1)
    train.send_command("oil_pump", true)
    await wait_seconds(5)
    train.send_command("oil_pump", false)
    train.send_command("fuel_pump", false)
    train.send_command("main_switch", true)
    await wait_seconds(1)
    assert_true(train.state["engine_rpm"] > 0, "Engine RPM should be > 0")

func test_successful_brake_releasing():
    train.send_command("brake_level_set_position", "drive")
    await wait_idle_frames(2)
    train.send_command("brake_releaser", true)
    await wait_seconds(10)
    var value = train.state["brake_air_pressure"]
    assert_true(value < 0.1, "Brake air pressure should be < 0.1, got %s" % value)

func test_successful_moving_on():
    train.send_command("fuel_pump", true)
    train.send_command("brake_level_set_position", "drive")
    train.send_command("brake_releaser", true)
    train.send_command("oil_pump", true)
    await wait_seconds(5)
    train.send_command("oil_pump", false)
    train.send_command("fuel_pump", false)
    train.send_command("main_switch", true)
    await wait_seconds(5)
    train.send_command("direction_increase")
    train.send_command("main_controller_increase", true)
    await wait_seconds(1)
    train.send_command("main_controller_increase", true)
    await wait_seconds(2)
    assert_true(train.state["engine_rpm"] > 0, "Engine RPM should be > 0")  # recheck
    assert_true(train.state["brake_air_pressure"] < 0.1, "Brake air pressure should be < 0.1")  # recheck
    assert_true(train.state["speed"] > 0, "Speed should be > 0")
