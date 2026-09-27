extends MaszynaGutTest

## A control whose original handler branches on the kind of switch (TGaugeType) behaves by the type
## the cab's MMD gives it (LegacyCabinControls.button_type()), as TTrain branches on ggX.type().

const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")

var train: VehicleController
var logic: LegacyCabinLogic


func _build_cab(controls:Dictionary[StringName, CabinButton.ButtonType],
        model:VehicleModel = SM42, components:Array[VehicleComponent] = []) -> void:
    train = build_vehicle("TestButtonTypes", model)
    for component:VehicleComponent in components:
        train.add_component(component)
    train.battery_voltage = 110.0
    train.apply_configuration()
    var cab_controls: LegacyCabinControls = LegacyCabinControls.new()
    for control_id:StringName in controls:
        cab_controls.add_control(control_id, CabinButton, {}, controls[control_id])
    logic = LegacyCabinLogic.new(func(_cab: int) -> LegacyCabinControls: return cab_controls)
    logic.register(train.get_rid(), 1)
    await wait_idle_frames(2)


func after_each():
    logic.unregister()


# Train.cpp:3914 - an impulse fuel pump switch runs the pump while it is held
func test_push_fuel_pump_runs_only_while_held():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"fuelpump_sw": CabinButton.ButtonType.PUSH}
    await _build_cab(controls)
    CabinSystem.act(train.get_rid(), 1, &"fuelpump_sw", &"hold")
    assert_true(train.state["fuel_pump_enabled"], "held")
    CabinSystem.act(train.get_rid(), 1, &"fuelpump_sw", &"release")
    assert_false(train.state["fuel_pump_enabled"], "released")


# Train.cpp:3889-3900 - a two-state one flips on a press and ignores the release
func test_two_state_fuel_pump_flips_on_a_press():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"fuelpump_sw": CabinButton.ButtonType.TOGGLE}
    await _build_cab(controls)
    CabinSystem.act(train.get_rid(), 1, &"fuelpump_sw", &"hold")
    CabinSystem.act(train.get_rid(), 1, &"fuelpump_sw", &"release")
    assert_true(train.state["fuel_pump_enabled"], "stays on after the release")
    CabinSystem.act(train.get_rid(), 1, &"fuelpump_sw", &"hold")
    assert_false(train.state["fuel_pump_enabled"], "the next press turns it off")


# Train.cpp:2891, 2929 - an impulse battery switch flips the battery on a press, and its release
# only returns it to neutral
func test_push_battery_switch_flips_on_each_press():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"battery_sw": CabinButton.ButtonType.PUSH}
    await _build_cab(controls)
    CabinSystem.act(train.get_rid(), 1, &"battery_sw", &"hold")
    CabinSystem.act(train.get_rid(), 1, &"battery_sw", &"release")
    await wait_idle_frames(2)
    assert_true(train.state["battery_enabled"], "the first press switches it on")
    CabinSystem.act(train.get_rid(), 1, &"battery_sw", &"hold")
    CabinSystem.act(train.get_rid(), 1, &"battery_sw", &"release")
    await wait_idle_frames(2)
    assert_false(train.state["battery_enabled"], "the second one off")


# Train.cpp:6662 - the train heating switch does nothing in a cab that does not model it
func test_train_heating_without_its_gauge_does_nothing():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {}
    await _build_cab(controls)
    assert_null(CabinSystem.act(train.get_rid(), 1, &"trainheating_sw", &"toggle"))


# Train.cpp:3815-3824 - without main_on_bt the closing key moves an impulse main_sw up, and its
# release brings it back midway
func test_the_closing_key_moves_an_impulse_main_switch():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"main_sw": CabinButton.ButtonType.PUSH}
    await _build_cab(controls)
    CabinSystem.act(train.get_rid(), 1, &"main_on_bt", &"hold")
    assert_eq(CabinSystem.get_control(train.get_rid(), 1, &"main_sw"), LegacyCabinMainSwitch.LEVER_CLOSE)
    CabinSystem.act(train.get_rid(), 1, &"main_on_bt", &"release")
    assert_eq(CabinSystem.get_control(train.get_rid(), 1, &"main_sw"), LegacyCabinMainSwitch.LEVER_REST)


# Train.cpp:3474, 3455 - an impulse pantselected_sw goes up to raise and comes back midway
func test_an_impulse_pantograph_lever_goes_up_and_back_to_rest():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"pantselected_sw": CabinButton.ButtonType.PUSH}
    var components:Array[VehicleComponent] = [MoverVehicleElectricSeriesEngine.new()]
    await _build_cab(controls, null, components)
    CabinSystem.act(train.get_rid(), 1, &"pantselected_sw", &"hold")
    assert_eq(CabinSystem.get_control(train.get_rid(), 1, &"pantselected_sw"), LegacyCabinPantographSelected.LEVER_UP)
    CabinSystem.act(train.get_rid(), 1, &"pantselected_sw", &"release")
    assert_eq(CabinSystem.get_control(train.get_rid(), 1, &"pantselected_sw"), LegacyCabinPantographSelected.LEVER_REST)


func _electric_components(impulse:bool) -> Array[VehicleComponent]:
    var engine := MoverVehicleElectricSeriesEngine.new()
    engine.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    engine.power_current_collector_number_of_collectors = 2
    var switches := MoverVehicleSwitches.new()
    switches.pantograph_impulse = impulse
    var components:Array[VehicleComponent] = [engine, switches]
    return components


# Train.cpp:3218-3300 - with impulse pantograph switches a press opens one side of the valve and the
# release lets go; lowering needs the lowering button, which a cab may declare with no submodel
func test_impulse_pantograph_switch_raises_and_lowers_through_its_valve():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"pantfront_sw": CabinButton.ButtonType.TOGGLE, &"pantfrontoff_sw": CabinButton.ButtonType.TOGGLE}
    await _build_cab(controls, null, _electric_components(true))
    CabinSystem.act(train.get_rid(), 1, &"pantfront_sw", &"hold")
    assert_true(train.state["current_collector/pantograph_first_valve_enabled"], "held up")
    CabinSystem.act(train.get_rid(), 1, &"pantfront_sw", &"release")
    assert_false(train.state["current_collector/pantograph_first_valve_enabled"], "let go")
    CabinSystem.act(train.get_rid(), 1, &"pantfrontoff_sw", &"hold")
    assert_false(train.state["current_collector/pantograph_first_valve_enabled"])


# Train.cpp:3285 - an impulse type without the lowering button cannot lower from the cab
func test_impulse_pantograph_cannot_be_lowered_without_its_lowering_button():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"pantfront_sw": CabinButton.ButtonType.TOGGLE}
    await _build_cab(controls, null, _electric_components(true))
    assert_null(CabinSystem.act(train.get_rid(), 1, &"pantfrontoff_sw", &"hold"))


# Train.cpp:3239 - a two-state switch sets the valve and keeps it
func test_two_state_pantograph_switch_keeps_its_valve():
    var controls:Dictionary[StringName, CabinButton.ButtonType] = {
        &"pantfront_sw": CabinButton.ButtonType.TOGGLE}
    await _build_cab(controls, null, _electric_components(false))
    CabinSystem.act(train.get_rid(), 1, &"pantfront_sw", &"toggle", true)
    assert_true(train.state["current_collector/pantograph_first_valve_enabled"])
    CabinSystem.act(train.get_rid(), 1, &"pantfront_sw", &"toggle", false)
    assert_false(train.state["current_collector/pantograph_first_valve_enabled"])
