extends MaszynaGutTest

const FIXTURE_PATH := "res://tests/fixtures/test_vehicle.fiz"

var controller: TrainController


func before_each():
    controller = FizTrainControllerInstancer.build(FIXTURE_PATH)
    add_child(controller)
    await wait_idle_frames(2)


func after_each():
    remove_child(controller)
    controller.free()


func test_param_and_dimensions():
    assert_eq(controller.get_mass(), 74000.0)
    assert_eq(controller.get_reduced_mass(), 2000.0)
    assert_eq(controller.get_max_velocity(), 90.0)
    assert_eq(controller.get_power(), 590.0)
    assert_eq(controller.get_category(), TrainController.CATEGORY_TRAIN)
    assert_eq(controller.get_train_type(), TrainController.TRAIN_TYPE_DEFAULT)
    assert_eq(controller.get_length(), 16.6)
    assert_eq(controller.get_height(), 4.28)
    assert_eq(controller.get_width(), 3.07)
    assert_eq(controller.get_drag_coefficient(), 0.5)


func test_cntrl_general_subset():
    assert_true(controller.get_automatic_cab_activation())
    assert_eq(controller.get_battery_start_mode(), TrainController.START_MODE_MANUAL)
    assert_eq(controller.get_ground_relay_start_mode(), TrainController.START_MODE_MANUAL)


func test_wheels():
    var wheels: TrainWheels = controller.get_node("TrainWheels")
    assert_not_null(wheels)
    assert_eq(wheels.get_powered_wheel_diameter(), 1.1)
    assert_eq(wheels.get_front_rolling_wheel_diameter(), 1.1) # defaults to powered diameter
    assert_eq(wheels.get_track_width(), 1.435)
    assert_eq(wheels.get_axle_arrangement(), "Bo'Bo'")
    assert_eq(wheels.get_bogie_axle_spacing(), 2.6)
    assert_eq(wheels.get_bogie_pivot_spacing(), 7.524)


func test_brake_and_bpt_table():
    var brake: TrainBrake = controller.get_node("TrainBrake")
    assert_not_null(brake)
    assert_eq(brake.get_max_brake_force(), 250.0)
    assert_eq(brake.get_max_cylinder_pressure(), 3.8)
    assert_eq(brake.get_cylinder_count(), 4)
    assert_eq(brake.get_rig_effectiveness(), 0.85)
    assert_eq(brake.get_valve_type(), TrainBrake.BRAKE_VALVE_W_LU_L)
    assert_eq(brake.get_brake_system(), TrainBrake.BRAKE_SYSTEM_PNEUMATIC)
    assert_eq(brake.get_brake_ctrl_position_count(), 6)
    assert_eq(brake.get_brake_delay_1(), 15.0)
    assert_eq(brake.get_brake_handle_type(), TrainBrake.BRAKE_HANDLE_TYPE_FV4A)
    assert_true(brake.get_manual_brake_present())

    var bpt: Array = brake.get_brake_pressure_table()
    assert_eq(bpt.size(), 3)
    var row0: BrakePressureTableItem = bpt[0]
    assert_eq(row0.get_handle_position(), -1)
    assert_eq(row0.get_pipe_pressure(), 0.0)
    assert_eq(row0.get_brake_cylinder_pressure(), -1.0)
    var row2: BrakePressureTableItem = bpt[2]
    assert_eq(row2.get_handle_position(), 3)
    assert_eq(row2.get_pipe_pressure(), 3.5)


func test_doors():
    var doors: TrainDoors = controller.get_node("TrainDoors")
    assert_not_null(doors)
    assert_eq(doors.get_open_time(), 3.0)
    assert_eq(doors.get_max_shift(), 3.0) # DoorMaxShiftR
    assert_eq(doors.get_type(), TrainDoors.TYPE_ROTATE)
    assert_eq(doors.get_voltage(), TrainDoors.VOLTAGE_24)


func test_buff_coupl():
    var coupler: TrainBuffCoupl = controller.get_node("TrainBuffCoupl")
    assert_not_null(coupler)
    assert_eq(coupler.get_coupler_type(), TrainBuffCoupl.COUPLER_TYPE_SCREW)
    assert_eq(coupler.get_coupler_stiffness_k(), 2500.0) # kC=2.5 kN/m -> N/m
    assert_eq(coupler.get_coupler_max_tension_tolerance(), 1000000.0) # FmaxC=1000 kN -> N
    assert_eq(coupler.get_allowed_flag(), 63)
