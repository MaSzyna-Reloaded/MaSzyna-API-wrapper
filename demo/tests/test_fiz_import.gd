extends MaszynaGutTest

const FIXTURE_PATH := "res://tests/fixtures/test_vehicle.fiz"

var controller: VehicleController


func before_each():
    controller = FizTrainControllerInstancer.build(FIXTURE_PATH)
    add_child(controller)
    await wait_idle_frames(2)


func after_each():
    remove_child(controller)
    controller.free()


func test_param_and_dimensions():
    assert_eq(controller.mass, 74000.0)
    assert_eq(controller.reduced_mass, 2000.0)
    assert_eq(controller.max_velocity, 90.0)
    assert_eq(controller.power, 590.0)
    assert_eq(controller.category, VehicleController.CATEGORY_TRAIN)
    assert_eq(controller.train_type, VehicleController.TRAIN_TYPE_DEFAULT)
    assert_eq(controller.dimensions_length, 16.6)
    assert_eq(controller.dimensions_height, 4.28)
    assert_eq(controller.dimensions_width, 3.07)
    assert_eq(controller.dimensions_drag_coefficient, 0.5)


func test_cntrl_general_subset():
    assert_true(controller.cntrl_automatic_cab_activation)
    assert_eq(controller.cntrl_battery_start_mode, VehicleController.START_MODE_MANUAL)
    assert_eq(controller.cntrl_ground_relay_start_mode, VehicleController.START_MODE_MANUAL)


func test_wheels():
    var wheels: VehicleWheels = controller.get_node("VehicleWheels")
    assert_not_null(wheels)
    assert_eq(wheels.powered_wheel_diameter, 1.1)
    assert_eq(wheels.front_rolling_wheel_diameter, 1.1) # defaults to powered diameter
    assert_eq(wheels.track_width, 1.435)
    assert_eq(wheels.axle_arrangement, "Bo'Bo'")
    assert_eq(wheels.bogie_axle_spacing, 2.6)
    assert_eq(wheels.bogie_pivot_spacing, 7.524)


func test_brake_and_bpt_table():
    var brake: VehicleBrake = controller.get_node("VehicleBrake")
    assert_not_null(brake)
    assert_eq(brake.brake_force_max, 250.0)
    assert_eq(brake.max_cylinder_pressure, 3.8)
    assert_eq(brake.cylinder_count, 4)
    assert_eq(brake.rig_effectiveness, 0.85)
    assert_eq(brake.valve_type, VehicleBrake.BRAKE_VALVE_W_LU_L)
    assert_eq(brake.cntrl_brake_system, VehicleBrake.BRAKE_SYSTEM_PNEUMATIC)
    assert_eq(brake.cntrl_brake_ctrl_position_count, 6)
    assert_eq(brake.cntrl_brake_delay_1, 15.0)
    assert_eq(brake.cntrl_brake_handle_type, VehicleBrake.BRAKE_HANDLE_TYPE_FV4A)
    assert_true(brake.cntrl_manual_brake_present)

    var bpt: Array = brake.brake_pressure_table
    assert_eq(bpt.size(), 3)
    var row0: BrakePressureTableItem = bpt[0]
    assert_eq(row0.handle_position, -1)
    assert_eq(row0.pipe_pressure, 0.0)
    assert_eq(row0.brake_cylinder_pressure, -1.0)
    var row2: BrakePressureTableItem = bpt[2]
    assert_eq(row2.handle_position, 3)
    assert_eq(row2.pipe_pressure, 3.5)


func test_doors():
    var doors: VehicleDoors = controller.get_node("VehicleDoors")
    assert_not_null(doors)
    assert_eq(doors.open_time, 3.0)
    assert_eq(doors.max_shift, 3.0) # DoorMaxShiftR
    assert_eq(doors.type, VehicleDoors.TYPE_ROTATE)
    assert_eq(doors.voltage, VehicleDoors.VOLTAGE_24)


func test_buff_coupl():
    var coupler: VehicleBuffCoupl = controller.get_node("VehicleBuffCoupl")
    assert_not_null(coupler)
    assert_eq(coupler.coupler_type, VehicleBuffCoupl.COUPLER_TYPE_SCREW)
    assert_eq(coupler.coupler_stiffness_k, 2.5) # kC in kN/m, converted to N/m by VehicleBuffCoupl
    assert_eq(coupler.coupler_max_tension_tolerance, 1000.0) # FmaxC in kN
    assert_eq(coupler.buffer_location, VehicleBuffCoupl.BUFFER_LOCATION_BOTH)
    assert_eq(coupler.allowed_flag, 63)
