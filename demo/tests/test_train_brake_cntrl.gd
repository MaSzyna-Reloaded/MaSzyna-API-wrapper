extends MaszynaGutTest

var train: VehicleController
var brake: VehicleBrake

func before_each():
    train = VehicleController.new()
    train.train_id = "TestTrain"
    add_child(train)

    brake = MoverVehicleBrake.new()
    train.add_child(brake)
    await wait_idle_frames(2)

func after_each():
    remove_child(train)
    train.free()

func test_defaults_match_original_mover():
    assert_eq(brake.cntrl_brake_system, VehicleBrake.BRAKE_SYSTEM_PNEUMATIC)
    assert_eq(brake.cntrl_brake_ctrl_position_count, 6)
    assert_eq(brake.cntrl_brake_delay_1, 15.0)
    assert_eq(brake.cntrl_brake_delay_2, 3.0)
    assert_eq(brake.cntrl_brake_delay_3, 36.0)
    assert_eq(brake.cntrl_brake_delay_4, 22.0)
    assert_eq(brake.cntrl_brake_delays, VehicleBrake.BRAKE_DELAY_GP)
    assert_eq(brake.cntrl_brake_op_modes, VehicleBrake.BRAKE_OP_MODE_PNEPMED)
    assert_eq(brake.cntrl_brake_handle_type, VehicleBrake.BRAKE_HANDLE_TYPE_FV4A)
    assert_eq(brake.cntrl_local_brake_handle_type, VehicleBrake.BRAKE_HANDLE_TYPE_FD1)
    assert_eq(brake.cntrl_anti_skid_brake_type, VehicleBrake.ANTI_SKID_BRAKE_MANUAL)
    assert_eq(brake.cntrl_local_brake_type, VehicleBrake.LOCAL_BRAKE_TYPE_PNEUMATIC)
    assert_false(brake.cntrl_manual_brake_present)
    assert_true(brake.cntrl_spring_brake_cuts_off_drive)

func test_round_trip_and_update_without_crashing():
    brake.cntrl_brake_system = VehicleBrake.BRAKE_SYSTEM_ELECTRO_PNEUMATIC
    brake.cntrl_brake_ctrl_position_count = 8
    brake.cntrl_brake_delays = VehicleBrake.BRAKE_DELAY_GPR_MG
    brake.cntrl_brake_op_modes = VehicleBrake.BRAKE_OP_MODE_PN
    brake.cntrl_brake_handle_type = VehicleBrake.BRAKE_HANDLE_TYPE_KNORR
    brake.cntrl_local_brake_handle_type = VehicleBrake.BRAKE_HANDLE_TYPE_WESTINGHOUSE
    brake.cntrl_anti_skid_brake_type = VehicleBrake.ANTI_SKID_BRAKE_AUTOMATIC
    brake.cntrl_local_brake_type = VehicleBrake.LOCAL_BRAKE_TYPE_HYDRAULIC
    brake.cntrl_manual_brake_present = false
    brake.cntrl_dynamic_brake_type = VehicleBrake.DYNAMIC_BRAKE_AUTOMATIC
    brake.cntrl_local_brake_traxx = true
    brake.cntrl_release_parking_by_spring_brake = true
    brake.cntrl_release_parking_by_spring_brake_when_door_open = true
    brake.cntrl_spring_brake_cuts_off_drive = false
    brake.cntrl_spring_brake_drive_emergency_velocity = 5.0
    await wait_idle_frames(2)

    assert_eq(brake.cntrl_brake_system, VehicleBrake.BRAKE_SYSTEM_ELECTRO_PNEUMATIC)
    assert_eq(brake.cntrl_brake_ctrl_position_count, 8)
    assert_eq(brake.cntrl_brake_delays, VehicleBrake.BRAKE_DELAY_GPR_MG)
    assert_eq(brake.cntrl_dynamic_brake_type, VehicleBrake.DYNAMIC_BRAKE_AUTOMATIC)
    assert_true(brake.cntrl_local_brake_traxx)
    assert_true(train.state.has("brake_air_pressure"), "VehicleBrake should keep functioning after configuring the Cntrl. section")
