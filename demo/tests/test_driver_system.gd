extends MaszynaGutTest

const Order = MaszynaLegacyAIDriver.Order
const MAX_WAIT:float = 5.0


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


func _create_driver(ai:MaszynaLegacyAIDriver) -> RID:
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_vehicle(driver, build_vehicle("AIDriverTest").get_rid())
    DriverSystem.driver_attach_delegate(driver, ai)
    return driver
