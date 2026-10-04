extends MaszynaGutTest

## The AI driver prepares the REAL EP07-424 (ep07.scn, its own .fiz and .mmd) for the road through
## its cab, to the end of the start sequence: low voltage, line breaker, converter
## (Driver.cpp PrepareEngine). The tester's report of 2026-10-04 had the driver stop at the line
## breaker; test_driver_system.gd checks the order only up to the battery, on a diesel.

## EP07-424 of td.scn on a cut of its line, with the EP07's own .fiz and .mmd (demo/tests/fixtures)
const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SCENERY:String = "ep07.scn"
## Real seconds the driver gets: pantographs, InitialCtrlDelay and the converter's start
const MAX_WAIT:float = 60.0

var _previous_game_dir:String = ""
var scenery:MaszynaSceneryNode
var vehicle_rid:RID


func before_each():
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)
    scenery = MaszynaSceneryNode.new()
    scenery.filename = SCENERY
    add_child(scenery)
    for i in range(30):
        vehicle_rid = VehicleServer.vehicle_get_rid_by_name("EP07-424")
        if VehicleServer.vehicle_is_simulation_ready(vehicle_rid):
            break
        await wait_seconds(0.5)


func after_each():
    scenery.free()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_the_driver_prepares_an_electric_locomotive_to_the_converter() -> void:
    var driver:RID = DriverSystem.vehicle_get_driver(vehicle_rid)
    assert_true(driver.is_valid(), "EP07-424 has its scenery driver (headdriver)")
    if not driver.is_valid():
        return
    var power_supply:RailVehiclePowerSupply = RailVehicleServer.vehicle_component_get(
            vehicle_rid, RailVehicleComponentType.COMPONENT_POWER_SUPPLY) as RailVehiclePowerSupply
    var engine:RailVehicleEngine = VehicleServer.vehicle_component_get(
            vehicle_rid, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleEngine
    assert_not_null(power_supply, "the EP07's FIZ (Light: LMaxVoltage) gives it a power supply")
    assert_not_null(engine, "and an engine")
    if not power_supply or not engine:
        return
    assert_false(engine.get_main_switch_enabled(), "the locomotive stands cold, the line breaker open")

    DriverSystem.driver_send_command(driver, "Prepare_engine", 1.0, 0.0)
    await wait_until(func() -> bool: return power_supply.get_converter_enabled(), MAX_WAIT)

    assert_true(power_supply.get_power24_available(), "the battery gives the low voltage")
    assert_true(engine.get_main_switch_enabled(), "the driver closes the line breaker")
    assert_true(power_supply.get_converter_enabled(), "and starts the converter")
    assert_true(power_supply.get_power110_available(), "which gives the 110 V circuits")
