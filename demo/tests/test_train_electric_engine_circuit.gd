extends MaszynaGutTest

var train: VehicleController
var engine: VehicleElectricSeriesEngine

func before_each():
    train = VehicleController.new()
    train.train_id = "TestTrain"
    add_child(train)

    engine = MoverVehicleElectricSeriesEngine.new()
    engine.power_source = VehicleController.POWER_SOURCE_ACCUMULATOR
    train.add_component(engine)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_circuit_defaults():
    assert_eq(engine.circuit_resistance, 0.0)
    assert_eq(engine.circuit_imax_low, 0)
    assert_eq(engine.circuit_imax_high, 0)
    assert_eq(engine.circuit_tuhex_sum, 750.0)
    assert_eq(engine.circuit_tuhex_diff, 10.0)
    assert_eq(engine.circuit_tuhex_min_current, 60.0)
    assert_eq(engine.circuit_tuhex_max_current, 400.0)
    assert_eq(engine.circuit_tuhex_stages, 0)
    assert_eq(engine.power_current_collector_physical_layout, 0)

func test_circuit_round_trip_and_update():
    engine.circuit_resistance = 0.35
    engine.circuit_imax_low = 600
    engine.circuit_imax_high = 900
    engine.circuit_imin_low = 100
    engine.circuit_imin_high = 150
    engine.circuit_tuhex_sum = 800.0
    engine.circuit_tuhex_diff = 12.0
    engine.circuit_tuhex_stages = 3
    engine.circuit_tuhex_sum_1 = 800.0
    engine.circuit_tuhex_sum_2 = 700.0
    engine.circuit_tuhex_sum_3 = 600.0
    await wait_idle_frames(2)

    assert_eq(engine.circuit_resistance, 0.35)
    assert_eq(engine.circuit_imax_low, 600)
    assert_eq(engine.circuit_imax_high, 900)
    assert_eq(engine.circuit_tuhex_stages, 3)
    assert_true(train.state.has("main_switch_enabled"), "VehicleElectricEngine should keep functioning after configuring the Circuit section")

func test_physical_layout_updates_without_crashing():
    engine.power_source = VehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    engine.power_current_collector_physical_layout = 3 # front and rear
    await wait_idle_frames(2)

    assert_eq(engine.power_current_collector_physical_layout, 3)
