extends MaszynaGutTest

var train: VehicleController
var engine: VehicleDieselEngine

func before_each():
    train = VehicleController.new()
    train.train_id = "TestTrain"
    add_child(train)

    engine = MoverVehicleDieselEngine.new()
    train.add_child(engine)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_defaults():
    assert_eq(engine.cntrl_main_controller_position_count, 0)
    assert_eq(engine.cntrl_auto_relay_mode, VehicleEngine.AUTO_RELAY_NO)
    assert_false(engine.cntrl_coupled_controllers)
    assert_false(engine.cntrl_has_camshaft)
    assert_eq(engine.motor_blowers_start_mode, VehicleEngine.START_MODE_MANUAL)
    assert_eq(engine.fuel_pump_start_mode, VehicleEngine.START_MODE_MANUAL)
    assert_eq(engine.oil_pump_start_mode, VehicleEngine.START_MODE_MANUAL)
    assert_eq(engine.water_pump_start_mode, VehicleEngine.START_MODE_MANUAL)

func test_round_trip_and_update_without_crashing():
    engine.cntrl_main_controller_position_count = 5
    engine.cntrl_shunt_controller_position_count = 3
    engine.cntrl_direction_change_max_position = 1
    engine.cntrl_eim_control_additional_zeros = true
    engine.cntrl_eim_control_emergency = true
    engine.cntrl_eim_control_type = VehicleEngine.EIM_CONTROL_TYPE_2
    engine.cntrl_auto_relay_mode = VehicleEngine.AUTO_RELAY_YES
    engine.cntrl_coupled_controllers = true
    engine.cntrl_has_camshaft = true
    engine.cntrl_series_shunt_on_series_position = true
    engine.cntrl_initial_controller_delay = 1.5
    engine.cntrl_controller_step_delay = 0.5
    engine.cntrl_controller_step_down_delay = 0.3
    engine.cntrl_fast_series_circuit = true
    engine.fuel_pump_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    engine.oil_pump_start_mode = VehicleEngine.START_MODE_AUTOMATIC
    engine.water_pump_start_mode = VehicleEngine.START_MODE_BATTERY
    await wait_idle_frames(2)

    assert_eq(engine.cntrl_main_controller_position_count, 5)
    assert_eq(engine.cntrl_auto_relay_mode, VehicleEngine.AUTO_RELAY_YES)
    assert_true(engine.cntrl_has_camshaft)
    assert_eq(engine.fuel_pump_start_mode, VehicleEngine.START_MODE_AUTOMATIC)
    assert_true(train.state.has("main_switch_enabled"), "VehicleEngine should keep functioning after configuring the Cntrl. section")


func test_cntrl_oil_start_is_applied_to_diesel_engine():
    FizTrainEngineCommon.apply_cntrl_engine_subset(engine, {"OilStart": "Automatic"})
    assert_eq(engine.oil_pump_start_mode, VehicleEngine.START_MODE_AUTOMATIC)
