extends MaszynaGutTest

const Order = MaszynaLegacyAIDriver.Order
const MAX_WAIT:float = 5.0
## A water heater that switches itself off at this [C] (HeaterMaxTemperature)
const WATER_HEATER_MAX_TEMPERATURE:float = 60.0
const SM42:VehicleController = preload("res://tests/fixtures/sm42_vehicle.tres")
const SHUNT_SPEED:float = 40.0


func test_a_driver_is_a_person_aboard_a_vehicle() -> void:
    var vehicle:RID = build_vehicle("DriverTest", null, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD).get_rid()
    var driver:RID = get_vehicle_driver(vehicle)

    DriverSystem.driver_attach_delegate(driver, UpdateCounter.new())

    assert_eq(VehicleServer.person_get_vehicle(driver), vehicle)
    assert_eq(DriverSystem.vehicle_get_driver(vehicle), driver)
    PersonServer.person_free(driver)
    assert_false(DriverSystem.vehicle_get_driver(vehicle).is_valid(), "a freed driver leaves its vehicle")


func test_the_drivers_are_listed_and_their_vehicles_announced() -> void:
    var vehicle:RID = build_vehicle("DriverListTest").get_rid()
    watch_signals(DriverSystem)
    watch_signals(VehicleServer)
    var driver:RID = PersonServer.person_create()

    RailVehicleServer.person_enter_front_cabin(driver, vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER)
    DriverSystem.driver_attach_delegate(driver, UpdateCounter.new())
    assert_has(DriverSystem.driver_get_rids(), driver)
    assert_signal_emitted_with_parameters(VehicleServer, "cabin_person_entered",
            [RailVehicleServer.vehicle_get_front_cabin(vehicle), driver, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER])
    PersonServer.person_free(driver)
    assert_signal_emitted_with_parameters(DriverSystem, "driver_freed", [driver])
    assert_does_not_have(DriverSystem.driver_get_rids(), driver)


func test_a_vehicle_is_driven_by_its_driver_or_a_player() -> void:
    var vehicle:RID = build_vehicle("DrivenTest").get_rid()
    var front:RID = RailVehicleServer.vehicle_get_front_cabin(vehicle)
    watch_signals(RailVehicleServer)
    var driver:RID = PersonServer.person_create()

    assert_false(VehicleServer.vehicle_has_person_role(vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER), "nobody drives it yet")
    RailVehicleServer.person_enter_front_cabin(driver, vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER)
    DriverSystem.driver_attach_delegate(driver, UpdateCounter.new())
    assert_true(VehicleServer.vehicle_has_person_role(vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER))
    assert_signal_emitted_with_parameters(RailVehicleServer, "vehicle_driver_cabin_changed", [vehicle, front])
    PlayerServer.player_take_over_vehicle(vehicle)
    assert_true(VehicleServer.vehicle_has_person_role(vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER), "a player taking over changes nothing")
    assert_eq(RailVehicleServer.vehicle_get_driver_cabin(vehicle), front, "a player taking over changes nothing")
    PersonServer.person_free(driver)
    assert_true(VehicleServer.vehicle_has_person_role(vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER), "the player still drives it")
    PlayerServer.player_leave_vehicle()
    assert_false(VehicleServer.vehicle_has_person_role(vehicle, VehiclePersonRole.VEHICLE_PERSON_ROLE_DRIVER), "the player left, and it has no driver")
    assert_signal_emitted_with_parameters(RailVehicleServer, "vehicle_driver_cabin_changed", [vehicle, RID()])


func test_a_driver_state_comes_from_its_delegate() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)
    var bare:RID = _create_driver(UpdateCounter.new())

    assert_has(DriverSystem.driver_get_state(driver), "orders")
    assert_eq(DriverSystem.driver_get_state(bare), {}, "a delegate without a state of its own, no state")


func test_shunt_velocity_starts_the_engine_and_then_shunts() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "ShuntVelocity", 40.0, -1.0)

    var state:Dictionary = DriverSystem.driver_get_state(driver)
    assert_eq(state["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS, Order.PREPARE_ENGINE, Order.SHUNT]))
    assert_eq(state["order"], Order.PREPARE_ENGINE, "the engine first")
    assert_eq(state["shunt_velocity"], 40.0)
    assert_false(state["stop_here"])


func test_set_velocity_zero_keeps_it_standing_and_a_speed_makes_it_a_train() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "SetVelocity", 0.0, 0.0)
    assert_true(DriverSystem.driver_get_state(driver)["stop_here"])
    assert_eq(DriverSystem.driver_get_state(driver)["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS]), "no order for a stop")

    DriverSystem.driver_send_command(driver, "SetVelocity", 60.0, 40.0)
    var state:Dictionary = DriverSystem.driver_get_state(driver)
    assert_eq(state["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS, Order.PREPARE_ENGINE, Order.OBEY_TRAIN]))
    assert_eq(state["velocity"], 60.0)
    assert_eq(state["velocity_next"], 40.0)


func test_no_timetable_means_shunting() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "Timetable:none", 30.0, 0.0)

    var state:Dictionary = DriverSystem.driver_get_state(driver)
    assert_null(state["timetable"])
    assert_eq(state["orders"], PackedInt32Array([Order.WAIT_FOR_ORDERS, Order.PREPARE_ENGINE, Order.SHUNT]))
    assert_eq(state["order_position"], 1, "a speed starts it at the first order")
    assert_eq(state["velocity"], 30.0)


func test_shunt_with_nothing_coupled_stands() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    DriverSystem.driver_send_command(driver, "Shunt", 0.0, 0.0)

    var state:Dictionary = DriverSystem.driver_get_state(driver)
    assert_true(state["stop_here"], "nothing on either side: it stands")
    assert_eq(state["velocity"], 0.0)
    assert_eq(state["vehicle_count"], 0)
    assert_true(Order.SHUNT in state["orders"])


func test_a_putvalues_order_reaches_the_activators_driver() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)
    var vehicle:RID = VehicleServer.person_get_vehicle(driver)
    var action:MaszynaLegacyVehicleCommandAction = MaszynaLegacyVehicleCommandAction.new()
    action.command = "Radio_channel"
    action.value1 = 4.0
    var event:RID = ScenarioEventServer.event_create()
    ScenarioEventServer.event_attach_action(event, action)

    ScenarioEventServer.event_queue(event, vehicle)
    await wait_until(func() -> bool: return not ScenarioEventServer.event_is_queued(event), MAX_WAIT)

    assert_eq(DriverSystem.driver_get_state(driver)["radio_channel"], 4)
    ScenarioEventServer.event_free(event)


func test_a_scheduled_update_reaches_the_delegate() -> void:
    var delegate:UpdateCounter = UpdateCounter.new()
    var driver:RID = _create_driver(delegate)

    DriverSystem.driver_schedule_update(driver, 0.0)
    DriverSystem.driver_schedule_update(driver, 0.0)
    await wait_until(func() -> bool: return delegate.updates > 0, MAX_WAIT)
    await wait_idle_frames(2)

    assert_eq(delegate.updates, 1, "a later schedule replaces the pending one")


func test_the_engine_is_prepared_and_released_through_the_cab() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverCabTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    # a cab with no controls of its own: every catalog control is there unmodelled
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)

    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return train.get_state()["battery_enabled"], MAX_WAIT)
    assert_true(train.get_state()["battery_enabled"], "the battery is switched on")
    assert_eq(CabinSystem.get_control(RailVehicleServer.vehicle_get_front_cabin(vehicle), &"battery_sw"), true, "by its switch in the cab")

    DriverSystem.driver_send_command(driver, "Prepare_engine", 0.0, 0.0)
    await wait_until(func() -> bool: return not train.get_state()["battery_enabled"], MAX_WAIT)
    assert_false(train.get_state()["battery_enabled"], "put away, the battery is off")
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


## FINDINGS.md 2026-09-29: EP07-329's line breaker tripped at a switch and the driver, taking the
## engine for ready still, rolled on without power - the consist's state is what takes it away
func test_the_trainset_shows_a_powered_vehicles_line_breaker_open() -> void:
    var train:VehicleController = build_vehicle("AIDriverBreakerTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var trainset:MaszynaLegacyDriverTrainset = MaszynaLegacyDriverTrainset.new()

    trainset.update(train.get_rid(), 1, true)

    assert_false(train.get_state()["main_switch_enabled"], "a cold engine's line breaker is open")
    assert_true(trainset.line_breaker_open, "and the trainset's state shows it")


func test_a_driver_not_in_control_touches_nothing() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverControlTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    assert_false(DriverSystem.vehicle_is_control_active(vehicle), "nobody drives a vehicle without a driver")
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    assert_true(DriverSystem.vehicle_is_control_active(vehicle), "its driver drives it")

    # a player in the cab (MaszynaPlayer)
    PlayerServer.player_take_over_vehicle(vehicle)
    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return _hinted(driver, MaszynaLegacyDriverHints.Hint.BATTERY_ON), MAX_WAIT)
    assert_false(train.get_state()["battery_enabled"], "the order is taken, the battery left alone")
    assert_eq(_hint_count(driver, MaszynaLegacyDriverHints.Hint.BATTERY_ON), 1,
            "the player is hinted to switch the battery on, once (cue_action(), driverhints.cpp:83)")

    PlayerServer.player_leave_vehicle()
    await wait_until(func() -> bool: return train.get_state()["battery_enabled"], MAX_WAIT)
    assert_true(train.get_state()["battery_enabled"], "back in control, it carries the order out")
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


func test_a_player_gets_the_next_hint_once_one_is_done() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverHintTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    PlayerServer.player_take_over_vehicle(vehicle)
    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return _hinted(driver, MaszynaLegacyDriverHints.Hint.BATTERY_ON), MAX_WAIT)

    CabinSystem.act(RailVehicleServer.vehicle_get_driver_cabin(vehicle), &"battery_sw", &"toggle", true)
    await wait_until(func() -> bool: return not _hinted(driver, MaszynaLegacyDriverHints.Hint.BATTERY_ON), MAX_WAIT)

    assert_true(train.get_state()["battery_enabled"], "the player switched the battery on")
    assert_false(_hinted(driver, MaszynaLegacyDriverHints.Hint.BATTERY_ON), "its hint is gone")
    assert_true(_hinted(driver, MaszynaLegacyDriverHints.Hint.OIL_PUMP_ON), "the diesel's oil pump stays hinted")
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    assert_false(engine.get_oil_pump_enabled(), "and left for the player")
    PlayerServer.player_leave_vehicle()
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


## A pump the vehicle starts by itself needs no hint (driverhints.cpp:198) - the operator's report of
## 2026-10-06, "Switch on fuel pump" hanging for good
func test_a_pump_not_switched_by_hand_is_never_hinted() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverPumpHintTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    engine.fuel_pump_start_mode = RailVehicleController.START_MODE_AUTOMATIC
    engine.oil_pump_start_mode = RailVehicleController.START_MODE_MANUAL
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    PlayerServer.player_take_over_vehicle(vehicle)
    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return _hinted(driver, MaszynaLegacyDriverHints.Hint.BATTERY_ON), MAX_WAIT)

    assert_false(engine.get_fuel_pump_enabled(), "the fuel pump is off")
    assert_false(_hinted(driver, MaszynaLegacyDriverHints.Hint.FUEL_PUMP_ON), "but it starts by itself: no hint")
    assert_true(_hinted(driver, MaszynaLegacyDriverHints.Hint.OIL_PUMP_ON), "the oil pump, switched by hand, is hinted")
    PlayerServer.player_leave_vehicle()
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


## PrepareHeating() (Driver.cpp:5032-5085): a diesel whose water heater switches itself off is
## always heated - the player is hinted to start the water pump first, and nothing is touched
func test_a_diesel_with_a_water_heater_is_hinted_its_water_pump() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverHeatingHintTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            vehicle, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    engine.cooling_heater_max_temperature = WATER_HEATER_MAX_TEMPERATURE
    engine.water_pump_start_mode = RailVehicleController.START_MODE_MANUAL
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    PlayerServer.player_take_over_vehicle(vehicle)
    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return _hinted(driver, MaszynaLegacyDriverHints.Hint.WATER_PUMP_ON), MAX_WAIT)

    assert_true(_hinted(driver, MaszynaLegacyDriverHints.Hint.WATER_PUMP_BREAKER_ON), "the water pump's breaker first")
    assert_true(_hinted(driver, MaszynaLegacyDriverHints.Hint.WATER_PUMP_ON), "then the water pump")
    assert_false(engine.get_water_pump_enabled(), "left for the player")
    PlayerServer.player_leave_vehicle()
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


func test_a_driver_created_for_a_vehicle_a_player_drives_touches_nothing() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverLateDriverTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var driver:RID = get_vehicle_driver(vehicle)
    # the player takes the cab while the scenery still loads, before the drivers are built
    PlayerServer.player_take_over_vehicle(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    assert_false(DriverSystem.vehicle_is_control_active(vehicle), "the player drives it, not its new driver")

    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_seconds(1.0)
    assert_false(train.get_state()["battery_enabled"], "the order is taken, the battery left alone")

    PlayerServer.player_leave_vehicle()
    await wait_until(func() -> bool: return train.get_state()["battery_enabled"], MAX_WAIT)
    assert_true(train.get_state()["battery_enabled"], "the player gone, it carries the order out")
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


func test_the_driver_reads_its_trainset() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverTrainsetTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)

    await wait_until(func() -> bool: return not DriverSystem.driver_get_state(driver)["trainset_vehicles"].is_empty(), MAX_WAIT)

    var state:Dictionary = DriverSystem.driver_get_state(driver)
    var alone:Array[RID] = [vehicle]
    assert_eq(state["trainset_vehicles"], alone, "a vehicle on its own is its whole trainset")
    assert_eq(state["trainset_mass"], float(train.get_state()["mass_total"]))
    assert_almost_eq(state["trainset_gravity_acceleration"], 0.0, 0.0001, "on the flat nothing pulls")


func test_a_new_driver_is_told_to_drive_the_way_it_faces() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)

    var state:Dictionary = DriverSystem.driver_get_state(driver)
    assert_ne(state["direction_order"], 0, "never told to drive no way (Driver.cpp:1872)")
    assert_eq(state["direction_order"], state["direction"])


func test_taking_control_back_takes_the_way_of_the_cab_left() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverTakeoverTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    # a cab with no controls of its own: every catalog control is there unmodelled
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    PlayerServer.player_take_over_vehicle(vehicle)
    var cabin:RID = RailVehicleServer.vehicle_get_driver_cabin(vehicle)
    # the player puts the reverser backwards and leaves the vehicle
    CabinSystem.act(cabin, MaszynaLegacyDriverHints.REVERSER, &"decrease")
    var left_with:int = int(train.get_state()["direction"])

    PlayerServer.player_leave_vehicle()

    var way:int = DriverSystem.driver_get_state(driver)["direction"]
    var cab_active:int = int(train.get_state()["cabin"])
    assert_eq(left_with, -1, "the player left the reverser backwards")
    assert_ne(cab_active, 0, "the driver switches its cab on")
    assert_eq(way, cab_active, "standing, the driver drives the way of the cab")
    assert_eq(int(train.get_state()["direction"]), way * cab_active, "and puts the reverser that way")
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


## FINDINGS.md 2026-10-06: on Stary Jawor the player changed the cab to drive back to an open dwarf
## and the driving aid kept reading the way left - the player's reverser turns the driver
## (DirectionChange(), Train.cpp:2670-2842)
func test_a_players_reverser_turns_the_driver() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverReverserTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    PlayerServer.player_take_over_vehicle(vehicle)
    var cabin:RID = RailVehicleServer.vehicle_get_driver_cabin(vehicle)
    VehicleServer.vehicle_send_command(vehicle, "cab_activation", true)
    var cab_active:int = int(train.get_state()["cabin"])
    var way:int = DriverSystem.driver_get_state(driver)["direction"]

    # the reverser stepped all the way against the way the driver drives
    while not int(train.get_state()["direction"]) == -way * cab_active:
        var before:int = int(train.get_state()["direction"])
        CabinSystem.act(cabin, MaszynaLegacyDriverHints.REVERSER, &"decrease" if way * cab_active > 0 else &"increase")
        if int(train.get_state()["direction"]) == before:
            break

    assert_ne(cab_active, 0, "the cab is on")
    assert_eq(int(train.get_state()["direction"]), -way * cab_active, "the player set the reverser the other way")
    assert_eq(DriverSystem.driver_get_state(driver)["direction"], -way, "and the driver drives that way")
    PlayerServer.player_leave_vehicle()
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


## FINDINGS.md 2026-10-06: the driver took the way of the cab left on a cab change, one change late -
## it read the active cab before the new one was switched on (TTrain::CabChange(), Train.cpp:10335)
func test_a_players_cab_change_turns_the_driver_the_way_of_the_new_cab() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var train:VehicleController = build_vehicle("AIDriverCabChangeTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD)
    var vehicle:RID = train.get_rid()
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cabin:RID) -> LegacyCabinControls: return controls))
    var driver:RID = get_vehicle_driver(vehicle)
    DriverSystem.driver_attach_delegate(driver, ai)
    PlayerServer.player_take_over_vehicle(vehicle)
    VehicleServer.vehicle_send_command(vehicle, "cab_activation", true)
    CabinSystem.act(RailVehicleServer.vehicle_get_driver_cabin(vehicle), MaszynaLegacyDriverHints.REVERSER, &"increase")
    var way:int = DriverSystem.driver_get_state(driver)["direction"]

    RailVehicleServer.person_change_cabin(PlayerServer.player_get_person(), RailVehicleServer.CABIN_CHANGE_BACKWARD)

    var absolute:int = int(train.get_state()["direction_absolute"])
    assert_eq(absolute, -way, "the new cab, its reverser forward, drives the other way")
    assert_eq(DriverSystem.driver_get_state(driver)["direction"], absolute, "and so does the driver at once")
    PlayerServer.player_leave_vehicle()
    DriverSystem.driver_attach_delegate(driver, null)
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


func test_a_turn_forgets_the_stop_of_a_signal_passed() -> void:
    var vehicle:RID = build_vehicle("RouteTurnTest", SM42, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD).get_rid()
    var trainset:MaszynaLegacyDriverTrainset = MaszynaLegacyDriverTrainset.new()
    var route:MaszynaLegacyDriverRoute = MaszynaLegacyDriverRoute.new()
    var timetable:MaszynaLegacyDriverTimetable = MaszynaLegacyDriverTimetable.new()
    trainset.update(vehicle, 1, true)
    route.update(vehicle, Order.SHUNT, false, SHUNT_SPEED, 0.0, MaszynaLegacyDriverSpeed.EASY_ACCELERATION,
            MaszynaLegacyDriverSpeed.NO_LIMIT, trainset, timetable, 0.0, SHUNT_SPEED, 0.0, false, MaszynaLegacyDriverBraking.new(), false)
    # a signal at stop passed, behind it now
    route.signal_velocity_last = 0.0

    trainset.update(vehicle, -1, true)
    route.update(vehicle, Order.SHUNT, false, SHUNT_SPEED, 0.0, MaszynaLegacyDriverSpeed.EASY_ACCELERATION,
            MaszynaLegacyDriverSpeed.NO_LIMIT, trainset, timetable, 0.0, SHUNT_SPEED, 0.0, false, MaszynaLegacyDriverBraking.new(), false)

    assert_eq(route.signal_velocity_last, MaszynaLegacyDriverSpeed.NO_LIMIT,
            "turned, the stop of the signal passed does not hold it (Driver.cpp:520-524)")


func test_the_timetable_state_comes_from_the_delegate() -> void:
    var ai:MaszynaLegacyAIDriver = MaszynaLegacyAIDriver.new()
    var driver:RID = _create_driver(ai)
    watch_signals(DriverSystem)

    DriverSystem.driver_send_command(driver, "Timetable:none", 30.0, 0.0)

    var state:Dictionary = DriverSystem.driver_get_timetable_state(driver)
    assert_null(state["timetable"])
    assert_eq(state["station_index"], 0)
    assert_eq(state["latency"], 0.0)
    assert_eq(state["delay"], 0.0)
    assert_false(state["arrived"])
    assert_signal_emitted_with_parameters(DriverSystem, "driver_timetable_changed", [driver])


func test_a_driver_without_a_timetable_has_no_timetable_state() -> void:
    var driver:RID = _create_driver(UpdateCounter.new())

    assert_eq(DriverSystem.driver_get_timetable_state(driver), {})


## The person build_vehicle() seats at the controls of a vehicle of its own, driven by the delegate
func _create_driver(delegate:DriverDelegate) -> RID:
    var driver:RID = get_vehicle_driver(build_vehicle("AIDriverTest", null, 0.0, MaszynaDynamicData.DriverType.DRIVER_HEAD).get_rid())
    DriverSystem.driver_attach_delegate(driver, delegate)
    return driver


class UpdateCounter extends DriverDelegate:
    var updates:int = 0

    func _update(_driver:RID) -> void:
        updates += 1


## The driver's hints of `hint` (DriverSystem.driver_get_state()["hints"])
func _hint_count(driver:RID, hint:MaszynaLegacyDriverHints.Hint) -> int:
    var count:int = 0
    for entry:Dictionary in DriverSystem.driver_get_state(driver).get("hints", []):
        if entry["hint"] == hint:
            count += 1
    return count


func _hinted(driver:RID, hint:MaszynaLegacyDriverHints.Hint) -> bool:
    return _hint_count(driver, hint) > 0
