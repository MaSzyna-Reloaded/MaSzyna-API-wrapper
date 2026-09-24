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
    physics_node.driver_type = VehicleController.DRIVER_HEAD
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


## A driven E186-like vehicle (the Engine: line of dynamic/pkp/e186_v2/p160dc.fiz, without InvNo)
## under 3000 V, with the line breaker closed and a direction set.
func _powered_up_eim(train_id: String) -> VehicleController:
    var physics_node: VehiclePhysicsNode = VehiclePhysicsNode.new()
    physics_node.train_id = train_id
    physics_node.driver_type = VehicleController.DRIVER_HEAD
    add_child_autofree(physics_node)
    var driven: VehicleController = physics_node.get_controller()
    driven.battery_voltage = 110.0
    driven.power = 5600.0
    driven.mass = 81000.0
    var wheels: VehicleWheels = MoverVehicleWheels.new()
    wheels.powered_wheel_diameter = 1.25
    wheels.axle_arrangement = "Bo'Bo'"
    driven.add_component(wheels)
    var eim: VehicleElectricInductionEngine = MoverVehicleElectricInductionEngine.new()
    eim.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    eim.cntrl_main_controller_position_count = 4
    eim.transmission_gear_teeth_motor = 48
    eim.transmission_gear_teeth_wheel = 251
    var line: MaszynaParser = MaszynaParser.new()
    line.initialize(("dfic=861 dfmax=1.84 p=2 cfu=43.7 cim=13.4 icif=0.679 Uzmax=2183 Uzh=2183 DU=20"
            + " I0=20 fcfu=43.7 F0=300 a1=0.4 Pmax=5600 Fh=150 Ph=2600 Vh0=5 Vh1=10 Imax=1950 abed=1"
            + " Flat=Yes").to_utf8_buffer())
    FizTrainElectricInductionEngineParser.new().apply_engine_fields(FizLineUtil.read_key_values(line), eim)
    eim.power_current_collector_max_voltage = 3900.0
    eim.power_current_collector_min_main_switch_voltage = 1900.0
    eim.power_current_collector_physical_layout = 3
    eim.power_current_collector_number_of_collectors = 2
    driven.add_component(eim)
    driven.apply_configuration()
    await wait_idle_frames(2)
    driven.send_command("battery", true)
    driven.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    for i in 10:
        eim.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0)
        await wait_idle_frames(1)
    await wait_seconds(1.0)
    driven.send_command("main_switch", true)
    driven.send_command("direction_increase")
    for i in 5:
        eim.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0)
        await wait_idle_frames(1)
    return driven


func test_powered_vehicle_without_inverter_count_does_not_turn_forces_into_nan():
    # Regression: without InvNo the Mover divides by InvertersNo (Mover.cpp:5627); the original
    # gives a powered EIM one inverter (Mover.cpp:11302), the wrapper left it at 0 and every force
    # of the vehicle became NaN as soon as a direction was set
    var driven: VehicleController = await _powered_up_eim("TestEimInverters")

    assert_true(driven.state["main_switch_enabled"], "the line breaker should be closed")
    assert_false(is_nan(float(driven.state["velocity"])), "velocity should not be NaN")
    assert_false(is_nan(float(driven.state["Ft"])), "traction force should not be NaN")


func test_driven_induction_motor_pulls_once_the_controller_moves():
    # Regression: the setpoint of an integrated controller is computed by DynObj.cpp:3246-3283
    # (CheckEIMIC), which the wrapper did not call - the controller moved and Ft stayed 0
    var driven: VehicleController = await _powered_up_eim("TestEimTraction")
    var engine: VehicleElectricEngine = driven.get_component(VehicleComponentType.COMPONENT_ENGINE)
    driven.send_command("main_controller_increase")
    for i in 30:
        engine.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0)
        await wait_idle_frames(1)

    assert_gt(float(driven.state["Ft"]), 0.0, "a driven induction motor should pull with the controller up")


func test_apply_power_uses_canonical_current_collector_properties():
    var line: MaszynaParser = MaszynaParser.new()
    line.initialize("CollectorsNo=2 MaxVoltage=3000.0 MaxCurrent=800.0".to_utf8_buffer())
    var power_kv: Dictionary = FizLineUtil.read_key_values(line)
    FizTrainEngineCommon.apply_power(engine, power_kv)

    assert_eq(engine.power_current_collector_number_of_collectors, 2)
    assert_eq(engine.power_current_collector_max_voltage, 3000.0)
    assert_eq(engine.power_current_collector_max_current, 800.0)
