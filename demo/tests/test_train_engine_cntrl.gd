extends MaszynaGutTest

var train: TrainController
var engine: TrainDieselEngine

func before_each():
    train = TrainController.new()
    train.train_id = "TestTrain"
    add_child(train)

    engine = TrainDieselEngine.new()
    train.add_child(engine)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_defaults():
    assert_eq(engine.get("main_controller_position_count"), 0)
    assert_eq(engine.get("auto_relay_mode"), TrainEngine.AUTO_RELAY_NO)
    assert_false(engine.get("coupled_controllers"))
    assert_false(engine.get("has_camshaft"))
    assert_eq(engine.get("motor_blowers_start_mode"), TrainEngine.START_MODE_MANUAL)
    assert_eq(engine.get("fuel_pump_start_mode"), TrainEngine.START_MODE_MANUAL)
    assert_eq(engine.get("oil_pump_start_mode"), TrainEngine.START_MODE_MANUAL)
    assert_eq(engine.get("water_pump_start_mode"), TrainEngine.START_MODE_MANUAL)

func test_round_trip_and_update_without_crashing():
    engine.set("main_controller_position_count", 5)
    engine.set("shunt_controller_position_count", 3)
    engine.set("direction_change_max_position", 1)
    engine.set("eim_control_additional_zeros", true)
    engine.set("eim_control_emergency", true)
    engine.set("eim_control_type", TrainEngine.EIM_CONTROL_TYPE_2)
    engine.set("auto_relay_mode", TrainEngine.AUTO_RELAY_YES)
    engine.set("coupled_controllers", true)
    engine.set("has_camshaft", true)
    engine.set("series_shunt_on_series_position", true)
    engine.set("initial_controller_delay", 1.5)
    engine.set("controller_step_delay", 0.5)
    engine.set("controller_step_down_delay", 0.3)
    engine.set("fast_series_circuit", true)
    engine.set("fuel_pump_start_mode", TrainEngine.START_MODE_AUTOMATIC)
    engine.set("oil_pump_start_mode", TrainEngine.START_MODE_AUTOMATIC)
    engine.set("water_pump_start_mode", TrainEngine.START_MODE_BATTERY)
    await wait_idle_frames(2)

    assert_eq(engine.get("main_controller_position_count"), 5)
    assert_eq(engine.get("auto_relay_mode"), TrainEngine.AUTO_RELAY_YES)
    assert_true(engine.get("has_camshaft"))
    assert_eq(engine.get("fuel_pump_start_mode"), TrainEngine.START_MODE_AUTOMATIC)
    assert_true(train.state.has("main_switch_enabled"), "TrainEngine should keep functioning after configuring the Cntrl. section")


func test_cntrl_oil_start_is_applied_to_diesel_engine():
    FizTrainEngineCommon.apply_cntrl_engine_subset(engine, {"OilStart": "Automatic"})
    assert_eq(engine.get_oil_pump_start_mode(), TrainEngine.START_MODE_AUTOMATIC)
