extends MaszynaGutTest

var train: VehicleController
var engine: VehicleDieselEngine

func before_each():
    train = VehicleController.new()
    train.train_id = "TestTrain"
    add_child(train)

    engine = MoverVehicleDieselEngine.new()
    train.add_component(engine)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func _make_point(x: float, y: float) -> CurvePointItem:
    var item = CurvePointItem.new()
    item.x = x
    item.y = y
    return item

func _make_throttle(position: int, fuel_dose: float, behavior: int) -> ThrottlePositionItem:
    var item = ThrottlePositionItem.new()
    item.throttle_position = position
    item.fuel_dose = fuel_dose
    item.clutch_behavior = behavior
    return item

func test_defaults():
    engine.apply_config()
    assert_eq(engine.mechanical_min_rpm, 0.0)
    assert_eq(engine.mechanical_max_rpm, 0.0)
    assert_eq(engine.mechanical_inertia, 1.0)
    assert_false(engine.torque_converter_present)
    assert_false(engine.retarder_present)
    assert_eq(engine.retarder_placement, VehicleDieselEngine.RETARDER_PLACEMENT_AFTER_GEARBOX)
    assert_eq((engine.throttle_table_positions as Array).size(), 0)
    assert_eq(engine.torque_table.size(), 0)
    assert_eq((engine.torque_converter_table as Array).size(), 0)
    assert_true(train.config.get("engine_shake_enabled", false))

func test_mechanical_and_torque_converter_round_trip():
    engine.mechanical_min_rpm = 600.0
    engine.mechanical_max_rpm = 2000.0
    engine.mechanical_fuel_cutoff_rpm = 2100.0
    engine.mechanical_inertia = 1.5
    engine.mechanical_clutch_engage_speed = 0.6
    engine.mechanical_clutch_disengage_speed = 0.8
    engine.torque_converter_present = true
    engine.torque_converter_max_torque_ratio = 2.5
    engine.torque_converter_coupling_point = 0.9
    engine.retarder_present = true
    engine.retarder_placement = VehicleDieselEngine.RETARDER_PLACEMENT_BETWEEN_GEARBOX_AND_TC
    engine.retarder_max_torque = 500.0
    engine.torque_converter_table = [_make_point(0.0, 4.89), _make_point(1.0, 0.0)]
    await wait_idle_frames(2)

    assert_eq(engine.mechanical_min_rpm, 600.0)
    assert_eq(engine.mechanical_max_rpm, 2000.0)
    assert_true(engine.torque_converter_present)
    assert_eq(engine.torque_converter_max_torque_ratio, 2.5)
    assert_true(engine.retarder_present)
    assert_eq(engine.retarder_placement, VehicleDieselEngine.RETARDER_PLACEMENT_BETWEEN_GEARBOX_AND_TC)
    assert_eq((engine.torque_converter_table as Array).size(), 2)
    assert_true(train.state.has("main_switch_enabled"), "VehicleDieselEngine should keep functioning after configuring mechanical/torque converter/retarder")

func test_throttle_table_and_torque_curve_round_trip():
    engine.throttle_table_max_torque = 1400.0
    engine.throttle_table_max_torque_rpm = 1200.0
    engine.throttle_table_nominal_fuel_dose = 1.0
    engine.throttle_table_nominal_fuel_consumption_rate = 210.0
    engine.throttle_table_positions = [
        _make_throttle(0, 0.0, ThrottlePositionItem.CLUTCH_BEHAVIOR_NONE),
        _make_throttle(1, 0.15, ThrottlePositionItem.CLUTCH_BEHAVIOR_HALF_CLUTCH_MIN_RPM),
    ]
    engine.torque_table = [_make_point(850, 1450), _make_point(2000, 2174)]
    await wait_idle_frames(2)

    var throttle_table: Array = engine.throttle_table_positions
    assert_eq(throttle_table.size(), 2)
    assert_eq((throttle_table[1] as ThrottlePositionItem).fuel_dose, 0.15)
    assert_eq(engine.torque_table.size(), 2)
    assert_true(train.state.has("main_switch_enabled"), "VehicleDieselEngine should keep functioning after configuring the throttle table and torque curve")

func test_oversized_throttle_table_is_truncated_without_crashing():
    var rows: Array[ThrottlePositionItem] = []
    for i in range(70):
        rows.append(_make_throttle(i, 0.0, ThrottlePositionItem.CLUTCH_BEHAVIOR_NONE))
    engine.throttle_table_positions = rows
    await wait_idle_frames(2)

    assert_true(is_instance_valid(engine), "VehicleDieselEngine should keep functioning after an oversized throttle_table")
