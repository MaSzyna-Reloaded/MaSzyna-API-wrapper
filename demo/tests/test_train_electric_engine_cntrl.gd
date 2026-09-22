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

func test_defaults():
    assert_eq(engine.cntrl_converter_start_mode, VehicleEngine.START_MODE_MANUAL)
    assert_false(engine.cntrl_pantograph_auto_valve)
    assert_eq(engine.cntrl_main_switch_start_mode, VehicleEngine.START_MODE_MANUAL)

func test_round_trip_and_update_without_crashing():
    engine.cntrl_converter_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    engine.cntrl_converter_start_delay = 2.0
    engine.cntrl_converter_overload_relay_start_mode = VehicleEngine.START_MODE_CONVERTER
    engine.cntrl_converter_overload_relay_off_when_main_is_off = true
    engine.cntrl_pantograph_compressor_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    engine.cntrl_pantograph_auto_valve = true
    engine.cntrl_main_switch_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    await wait_idle_frames(2)

    assert_eq(engine.cntrl_converter_start_mode, VehicleEngine.START_MODE_AUTOMATIC)
    assert_true(engine.cntrl_pantograph_auto_valve)
    assert_true(train.state.has("main_switch_enabled"), "VehicleElectricEngine should keep functioning after configuring the Cntrl. section")
