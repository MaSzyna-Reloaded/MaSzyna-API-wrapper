extends MaszynaGutTest

const Order = MaszynaLegacyAIDriver.Order
const MAX_WAIT:float = 5.0
const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")


func test_a_driver_is_attached_to_a_vehicle_by_rids() -> void:
    var vehicle:RID = build_vehicle("DriverTest").get_rid()
    var driver:RID = DriverSystem.driver_create()

    DriverSystem.driver_attach_vehicle(driver, vehicle)

    assert_eq(DriverSystem.driver_get_vehicle(driver), vehicle)
    assert_eq(DriverSystem.vehicle_get_driver(vehicle), driver)
    DriverSystem.driver_free(driver)
    assert_false(DriverSystem.vehicle_get_driver(vehicle).is_valid(), "a freed driver leaves its vehicle")


func test_shunt_velocity_starts_the_engine_and_then_shunts() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "ShuntVelocity", 40.0, -1.0)

    var state:Dictionary = ai.get_state(driver)
    assert_eq(state["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS, Order.PREPARE_ENGINE, Order.SHUNT]))
    assert_eq(state["order"], Order.PREPARE_ENGINE, "the engine first")
    assert_eq(state["shunt_velocity"], 40.0)
    assert_false(state["stop_here"])
    DriverSystem.driver_free(driver)


func test_set_velocity_zero_keeps_it_standing_and_a_speed_makes_it_a_train() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "SetVelocity", 0.0, 0.0)
    assert_true(ai.get_state(driver)["stop_here"])
    assert_eq(ai.get_state(driver)["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS]), "no order for a stop")

    DriverSystem.driver_send_command(driver, "SetVelocity", 60.0, 40.0)
    var state:Dictionary = ai.get_state(driver)
    assert_eq(state["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS, Order.PREPARE_ENGINE, Order.OBEY_TRAIN]))
    assert_eq(state["velocity"], 60.0)
    assert_eq(state["velocity_next"], 40.0)
    DriverSystem.driver_free(driver)


func test_no_timetable_means_shunting() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "Timetable:none", 30.0, 0.0)

    var state:Dictionary = ai.get_state(driver)
    assert_null(state["timetable"])
    assert_eq(state["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS, Order.PREPARE_ENGINE, Order.SHUNT]))
    assert_eq(state["order_position"], 1, "a speed starts it at the first order")
    assert_eq(state["velocity"], 30.0)
    DriverSystem.driver_free(driver)


func test_shunt_with_nothing_coupled_stands() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "Shunt", 0.0, 0.0)

    var state:Dictionary = ai.get_state(driver)
    assert_true(state["stop_here"], "nothing on either side: it stands")
    assert_eq(state["velocity"], 0.0)
    assert_eq(state["vehicle_count"], 0)
    assert_true(Order.SHUNT in state["orders"])
    DriverSystem.driver_free(driver)


func test_a_putvalues_order_reaches_the_activators_driver() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    var action:MaszynaLegacyVehicleCommandAction = MaszynaLegacyVehicleCommandAction.new()
    action.command = "Radio_channel"
    action.value1 = 4.0
    var event:RID = ScenarioEventServer.event_create()
    ScenarioEventServer.event_attach_action(event, action)

    ScenarioEventServer.event_queue(event, vehicle)
    await wait_until(func() -> bool: return not ScenarioEventServer.event_is_queued(event), MAX_WAIT)

    assert_eq(ai.get_state(driver)["radio_channel"], 4)
    ScenarioEventServer.event_free(event)
    DriverSystem.driver_free(driver)


func test_a_scheduled_update_reaches_the_delegate() -> void:
    var delegate:UpdateCounter = UpdateCounter.new()
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_delegate(driver, delegate)

    DriverSystem.driver_schedule_update(driver, 0.0)
    DriverSystem.driver_schedule_update(driver, 0.0)
    await wait_until(func() -> bool: return delegate.updates > 0, MAX_WAIT)
    await wait_idle_frames(2)

    assert_eq(delegate.updates, 1, "a later schedule replaces the pending one")
    DriverSystem.driver_free(driver)


func test_the_engine_is_prepared_and_released_through_the_cab() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverCabTest", SM42)
    train.battery_voltage = 110.0
    train.apply_configuration()
    var vehicle:RID = train.get_rid()
    # a cab with no controls of its own: every catalog control is there unmodelled
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cab:int) -> LegacyCabinControls: return controls))
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_vehicle(driver, vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)

    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return train.state["battery_enabled"], MAX_WAIT)
    assert_true(train.state["battery_enabled"], "the battery is switched on")
    assert_eq(CabinSystem.get_control(vehicle, 1, &"battery_sw"), true, "by its switch in the cab")

    DriverSystem.driver_send_command(driver, "Prepare_engine", 0.0, 0.0)
    await wait_until(func() -> bool: return not train.state["battery_enabled"], MAX_WAIT)
    assert_false(train.state["battery_enabled"], "put away, the battery is off")
    DriverSystem.driver_free(driver)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


func test_a_driver_not_in_control_touches_nothing() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverControlTest", SM42)
    train.battery_voltage = 110.0
    train.apply_configuration()
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cab:int) -> LegacyCabinControls: return controls))
    assert_false(DriverSystem.vehicle_is_control_active(vehicle), "nobody drives a vehicle without a driver")
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_vehicle(driver, vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    assert_true(DriverSystem.vehicle_is_control_active(vehicle), "its driver drives it")

    # a player in the cab (RailVehicle3D.enter_cabin())
    DriverSystem.vehicle_set_control_active(vehicle, false)
    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_seconds(1.0)
    assert_false(train.state["battery_enabled"], "the order is taken, the battery left alone")

    DriverSystem.vehicle_set_control_active(vehicle, true)
    await wait_until(func() -> bool: return train.state["battery_enabled"], MAX_WAIT)
    assert_true(train.state["battery_enabled"], "back in control, it carries the order out")
    DriverSystem.driver_free(driver)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


func test_the_driver_reads_its_trainset() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverTrainsetTest", SM42)
    var vehicle:RID = train.get_rid()
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_vehicle(driver, vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)

    await wait_until(func() -> bool: return not ai.get_state(driver)["trainset_vehicles"].is_empty(), MAX_WAIT)

    var state:Dictionary = ai.get_state(driver)
    var alone:Array[RID] = [vehicle]
    assert_eq(state["trainset_vehicles"], alone, "a vehicle on its own is its whole trainset")
    assert_eq(state["trainset_mass"], float(train.state["mass_total"]))
    assert_almost_eq(state["trainset_gravity_acceleration"], 0.0, 0.0001, "on the flat nothing pulls")
    DriverSystem.driver_free(driver)


func _create_driver(ai:MaszynaLegacyAIDriver) -> RID:
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_vehicle(driver, build_vehicle("AIDriverTest").get_rid())
    DriverSystem.driver_attach_delegate(driver, ai)
    return driver


class UpdateCounter extends DriverDelegate:
    var updates:int = 0

    func _update(_driver:RID) -> void:
        updates += 1
