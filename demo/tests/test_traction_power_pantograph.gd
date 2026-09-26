extends MaszynaGutTest

## Regression test for the pantograph -> Mover voltage layer (TractionPowerServer /
## RailVehicle3D._update_pantograph_power() / VehicleElectricEngine::set_pantograph_wire_voltage()).
## Bypasses scenery/geometry entirely (same style as test_train_electric_engine_power_source.gd)
## to isolate whether raising a pantograph with a wire voltage present actually reaches the
## mover's reported state - this is what a real EP07 on td.scn needs to work.

var train: VehicleController
var engine: VehicleElectricSeriesEngine


func before_each():
    train = build_vehicle("TestPantographTrain")
    train.battery_voltage = 110.0
    train.apply_configuration()
    engine = MoverVehicleElectricSeriesEngine.new()
    engine.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    engine.power_current_collector_physical_layout = 3 # both pantographs physically present
    engine.power_current_collector_max_voltage = 3600.0
    engine.power_current_collector_number_of_collectors = 2
    train.add_component(engine)
    await wait_idle_frames(2)


func test_raised_pantograph_with_wire_voltage_reaches_mover_state():
    # Only the two commands a real cabin click ever sends - no separate master-valve command,
    # since no cabin switch/keybind for that exists anywhere in this wrapper (see
    # VehicleElectricEngine::pantograph()'s own comment on why it opens the master valve itself).
    train.send_command("battery", true)
    await wait_idle_frames(2)
    train.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    await wait_idle_frames(2)

    assert_true(
            engine.get_state().get("current_collector/pantograph_first_active", false),
            "pantograph should report raised once battery is on and it's been raised")

    engine.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3600.0)
    await wait_idle_frames(2)

    assert_almost_eq(
            float(engine.get_state().get("current_collector/pantograph_first_voltage", 0.0)),
            3600.0, 1.0,
            "raised pantograph should read back the wire voltage fed in this frame")


func test_repeated_wire_voltage_updates_keep_reaching_the_mover():
    # Regression: set_pantograph_wire_voltage() used to only stash the value on the engine node;
    # VehicleElectricEngine::_do_update_internal_mover() (the only place that pushed it into the
    # mover) runs once at startup, so every voltage update after the first frame was silently
    # dropped. Calling this repeatedly, like RailVehicle3D does every frame, must keep working.
    train.send_command("battery", true)
    train.send_command("pantograph", VehicleElectricEngine.PANTOGRAPH_FIRST, true)
    await wait_idle_frames(2)

    for i in range(5):
        engine.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3000.0 + i * 100.0)
        await wait_idle_frames(1)

    assert_almost_eq(
            float(engine.get_state().get("current_collector/pantograph_first_voltage", 0.0)),
            3400.0, 1.0,
            "the mover should reflect the latest wire voltage, not just the first one ever set")


func test_lowered_pantograph_does_not_report_active():
    # current_collector/pantograph_first_voltage is a raw echo of the last wire voltage fed in,
    # not gated by is_active (pre-existing VehicleElectricEngine.cpp behavior, unrelated to this
    # feature) - is_active is the actual gate EnginePowerSourceVoltage()/PantFrontVolt use, so
    # that's what this asserts instead of the echoed value.
    engine.set_pantograph_wire_voltage(VehicleElectricEngine.PANTOGRAPH_FIRST, 3600.0)
    await wait_idle_frames(2)

    assert_false(engine.get_state().get("current_collector/pantograph_first_active", false))
