extends MaszynaStartupTest

## EN57-636ra (PKP/EN57_V2, fixtures/scenery/startup_en57-636ra.scn) started from its cab with the keyboard
## and moved off; its trailer cars brake with the load-weighing valve's empty-car pressure, and its
## FVel6 handle takes a whole position per key press

## The train brake handle's normalized setting for FVel6's position 3, the EP brake applied
## (pos_table, hamulce.cpp:34: -1 to 6)
const EP_BRAKE_LEVEL:float = 4.0 / 7.0
## Simulated seconds the cylinders are given to fill
const BRAKE_FILL_SECONDS:float = 10.0
## [bar]
const PRESSURE_TOLERANCE:float = 0.1
## [handle position]
const HANDLE_TOLERANCE:float = 0.001


func test_starts_and_moves_off() -> void:
    await run_startup("startup_en57-636ra.scn", "EN57-636ra", Kind.ELECTRIC_MULTIPLE_UNIT)


## MaxBPMass=52 with TareMaxBP=2.5: an empty 34 t car's cylinders stop at the empty-car pressure, not
## at MaxBP=4 (MBPM, Mover.cpp:10771; TEStEP2::PLC(), hamulce.cpp:1264) - at 4 bar its wheels locked
func test_empty_trailer_brakes_at_its_empty_car_pressure() -> void:
    await run_startup("startup_en57-636ra.scn", "EN57-636ra", Kind.ELECTRIC_MULTIPLE_UNIT)
    VehicleServer.vehicle_send_command(occupied, "brake_level_set", EP_BRAKE_LEVEL)
    await wait_simulated(BRAKE_FILL_SECONDS)
    var brake:RailVehicleBrake = _brake(occupied)
    assert_eq(brake.cntrl_max_brake_pressure_mass, 52.0, "MaxBPMass read from the FIZ")
    assert_almost_eq(brake.get_air_pressure(), brake.max_tare_pressure, PRESSURE_TOLERANCE,
            "the cab car's cylinders at the empty-car pressure, below MaxBP %.1f" % brake.max_cylinder_pressure)


## A key press moves an FVel6 a position - the EP brake applies from position 1 (TFVel6::GetPF(),
## hamulce.cpp:4310) - where an FV4a moves while the key is held (Train.cpp:1960-1966)
func test_a_key_press_steps_the_brake_handle_a_position() -> void:
    await run_startup("startup_en57-636ra.scn", "EN57-636ra", Kind.ELECTRIC_MULTIPLE_UNIT)
    var brake:RailVehicleBrake = _brake(occupied)
    assert_eq(brake.handle_movement, RailVehicleBrake.BRAKE_HANDLE_MOVEMENT_STEPPED, "an FVel6 steps")
    var before:float = brake.get_controller_position()
    await key_tap(&"brake_level_increase")
    assert_almost_eq(brake.get_controller_position(), before + brake.handle_step, HANDLE_TOLERANCE,
            "one press, one position")

