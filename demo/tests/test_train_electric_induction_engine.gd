extends MaszynaGutTest

var train: VehicleController
var engine: VehicleElectricInductionEngine

func before_each():
    train = build_vehicle("TestTrain")

    engine = MoverVehicleElectricInductionEngine.new()
    engine.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    train.add_component(engine)
    await wait_idle_frames(2)

func _make_point(x: float, y: float) -> CurvePointItem:
    var item = CurvePointItem.new()
    item.x = x
    item.y = y
    return item

func test_defaults():
    assert_eq(engine.slip_current_ratio, 0.0)
    assert_eq(engine.pole_pairs, 0.0)
    assert_eq(engine.max_power, 0.0)
    assert_eq(engine.motor_max_current, 0.0)
    assert_eq(engine.max_power_table.size(), 0)

func test_round_trip_and_update_without_crashing():
    engine.slip_current_ratio = 0.1
    engine.max_slip = 0.2
    engine.pole_pairs = 2.0
    engine.nominal_uf_ratio = 1.5
    engine.current_torque_ratio = 0.9
    engine.current_three_phase_ratio = 0.8
    engine.max_supply_voltage = 2800.0
    engine.max_supply_voltage_braking = 2400.0
    engine.inverter_voltage_drop = 10.0
    engine.no_load_current = 5.0
    engine.inverter_uf_setpoint = 1.0
    engine.inverter_uf_setpoint_braking = 0.9
    engine.initial_force = 200.0
    engine.force_drop_rate = 1.5
    engine.max_power = 1200.0
    engine.max_braking_force = 180.0
    engine.max_braking_power = 1000.0
    engine.braking_decay_velocity = 5.0
    engine.braking_decay_start_velocity = 20.0
    engine.motor_max_current = 600.0
    engine.max_power_table = [_make_point(0.0, 1200.0), _make_point(100.0, 600.0)]
    await wait_idle_frames(2)

    assert_eq(engine.slip_current_ratio, 0.1)
    assert_eq(engine.max_power, 1200.0)
    assert_eq(engine.max_power_table.size(), 2)
    assert_true(train.state.has("main_switch_enabled"), "VehicleElectricInductionEngine should keep functioning after configuring EIM parameters")

func test_line_breaker_stays_closed_under_the_nominal_wire_voltage():
    # Regression: CollectorParameters.MaxV (FIZ MaxVoltage, Mover.cpp:11622) was never set, so an
    # induction motor opened the line breaker above 0 + 200 V right after it closed. The cab is
    # occupied - an unmanned vehicle is not simulated and would never open it.
    var physics_node: VehiclePhysicsNode = VehiclePhysicsNode.new()
    physics_node.train_id = "TestEimTrain"
    physics_node.cabin_number = 1
    add_child_autofree(physics_node)
    var driven: VehicleController = physics_node.get_controller()
    driven.battery_voltage = 110.0
    # the breaker is checked against the voltage in TractionForce(), run only with Power > 0
    driven.power = 5600.0
    var eim: VehicleElectricInductionEngine = MoverVehicleElectricInductionEngine.new()
    eim.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    eim.cntrl_main_controller_position_count = 4
    eim.power_current_collector_max_voltage = 3900.0
    eim.power_current_collector_min_main_switch_voltage = 1900.0
    eim.power_current_collector_physical_layout = 3
    eim.power_current_collector_number_of_collectors = 2
    driven.add_component(eim)
    driven.apply_configuration()
    await wait_idle_frames(2)
    driven.send_command("battery", true)
    await wait_idle_frames(2)
    driven.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    for i in 10:
        eim.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0)
        await wait_idle_frames(1)
    await wait_seconds(1.0)
    eim.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0)
    assert_true(driven.state["main_switch_closable"], "the line breaker should be closable at 3000 V")

    driven.send_command("main_switch", true)
    for i in 5:
        eim.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0)
        await wait_idle_frames(1)

    assert_true(driven.state["main_switch_enabled"], "the line breaker should stay closed at 3000 V")


func test_apply_power_uses_canonical_current_collector_properties():
    var line: MaszynaParser = MaszynaParser.new()
    line.initialize("CollectorsNo=2 MaxVoltage=3000.0 MaxCurrent=800.0".to_utf8_buffer())
    var power_kv: Dictionary = FizLineUtil.read_key_values(line)
    FizTrainEngineCommon.apply_power(engine, power_kv)

    assert_eq(engine.power_current_collector_number_of_collectors, 2)
    assert_eq(engine.power_current_collector_max_voltage, 3000.0)
    assert_eq(engine.power_current_collector_max_current, 800.0)
