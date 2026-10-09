extends MaszynaStartupTest

## A unit the player started from cold and moved off is ready for its driver: the driver's
## readiness (PrepareEngine(), Driver.cpp:2893) lists nothing missing, so the driving aid does not
## stand at "Vehicle not ready". 36WEa-014A (pkp/impuls_v1, fixtures/scenery/startup_36wea-014a.scn):
## two units of three cars, the middle one with a power of its own and no engine
## (docs/findings-archive.md, 2026-10-06 Vehicle not ready with nothing missing), and the driver's
## hints ask for no pantograph the unit has not got.

## Simulated seconds the driver is given to update after the unit moved off
const DRIVER_UPDATE_SECONDS:float = 5.0


func test_a_unit_moved_off_is_ready_for_its_driver() -> void:
    await run_startup("startup_36wea-014a.scn", "36WEa-014A", Kind.ELECTRIC_MULTIPLE_UNIT, Pantographs.SELECTOR)
    var driver:RID = DriverServer.vehicle_get_driver(occupied)
    assert_true(driver.is_valid(), "the scenery's driver is seated in the unit")
    if not driver.is_valid():
        return
    await step(ticks(DRIVER_UPDATE_SECONDS))
    var state:Dictionary = DriverServer.driver_get_state(driver)
    var cars:Array[String] = []
    for car:RID in VehicleServer.vehicle_get_rids():
        var dump:Dictionary = VehicleServer.vehicle_dump_state(car)
        cars.append("%s power=%s main_switch=%s pantograph=%s/%s voltage=%s" % [
                VehicleServer.vehicle_get_name(car), VehicleServer.vehicle_get_controller(car).power,
                dump.get("main_switch_enabled"), dump.get("current_collector/pantograph_first_active"),
                dump.get("current_collector/pantograph_second_active"), dump.get("current_collector/voltage")])
    assert_eq(state.get("engine_missing", -1), 0, "nothing missing: %s" % "; ".join(cars))
    assert_true(state.get("engine_active", false), "the driver takes it ready")
    # the A car has one current collector (CollectorsNo=1): there is no pantograph B to raise
    # (MASZYNA_ORIGINAL_QUIRKS.md, "Pantograph B of a vehicle with one")
    var texts:Array[String] = []
    for hint:Dictionary in state.get("hints", []):
        texts.append(hint["text"])
    var unit:RID = RailVehicleServer.vehicle_find_pantograph_carrier(occupied)
    var source:RailVehicleEnginePowerSource = RailVehicleServer.vehicle_component_get(
            unit, RailVehicleComponentType.COMPONENT_ENGINE_POWER_SOURCE) as RailVehicleEnginePowerSource
    texts.append("unit %s collectors=%d first=%s/%.0fV second=%s/%.0fV" % [
            VehicleServer.vehicle_get_name(unit), source.current_collector_number_of_collectors,
            source.get_collector_pantograph_first_active(), source.get_collector_pantograph_first_voltage(),
            source.get_collector_pantograph_second_active(), source.get_collector_pantograph_second_voltage()])
    assert_false(MaszynaLegacyDriverHints.TEXTS[MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_ON] in texts,
            "no hint to raise a pantograph the unit has not got: %s" % [texts])
