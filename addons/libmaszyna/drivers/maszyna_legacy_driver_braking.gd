@tool
extends RefCounted
class_name MaszynaLegacyDriverBraking

## The original driver's brakes (TController::control_braking_force(), IncBrake(), DecBrake(),
## Driver.cpp:3014-3366, 8065-8190), operated through the cab as a player does (CabinSystem.act()).
## One per driver: it keeps the driver's own position of the train brake handle (BrakeCtrlPosition,
## the gbh_* scale, Driver.h:196-200) and the delay before the next adjustment (fBrakeTime); the
## position reaches the vehicle's handle the way its type takes it (SetTimeControllers(),
## Driver.cpp:4047-4097).
##
## Not ported yet: the braking table (fBrake_a0/a1 - zero, as the original leaves it for shunting;
## train running needs it), the electro-pneumatic and the EMU/DMU braking, the time-controlled
## handles (MHZ_K5P, MHZ_6P, M394, H14K1, St113, H1405 - moved every frame in the original), the
## manual brake, the pipe unlocking before the releaser - see TODO.md, "Drivers".

## BrakeCtrlPosition: lap, charging, running, and the range (gbh_*, Driver.h:196-200)
const POSITION_LAP:float = -2.0
const POSITION_CHARGING:float = -1.0
const POSITION_RUNNING:float = 0.0
const POSITION_MIN:float = -2.0
const POSITION_MAX:float = 6.0
## Uncoupling brakes the train at this position before it presses the buffers (trainbrakeapply,
## driverhints.cpp:810-817)
const POSITION_UNCOUPLING:float = 3.0
## BrakingInitialLevel, BrakingLevelIncrease (Driver.h:449-450)
const BRAKING_INITIAL_LEVEL:float = 1.0
const BRAKING_LEVEL_INCREASE:float = 0.25
## Below this BrakeCtrlPosition the handle goes back to running (Driver.cpp:3283)
const RUNNING_RETURN_POSITION:float = 0.74
## A step of the handle that releases (Driver.cpp:3279)
const RELEASE_STEP:float = -0.25
## Harder than this [m/s2] at the last position is an emergency: one more (Driver.cpp:3101)
const EMERGENCY_ACCELERATION:float = -1.5
## The deepest position a second step of braking still goes to (Driver.cpp:3151)
const DEEPEST_DOUBLE_STEP:float = 5.0
## The local brake: its positions (LocalBrakePosNo, hamulce.h:39) and a step of one
const LOCAL_BRAKE:StringName = &"localbrake"
const LOCAL_BRAKE_POSITIONS:float = 10.0
const LOCAL_BRAKE_RELEASED:float = 0.0
const LOCAL_BRAKE_APPLIED:float = 1.0
## Released below this local brake position (independentbrakerelease, driverhints.cpp)
const LOCAL_BRAKE_OFF:float = 0.05
## DecLocalBrakeLevel(2) of the pneumatic release (Driver.cpp:3286)
const LOCAL_RELEASE_STEPS:int = 2
const TRAIN_BRAKE:StringName = &"brakectrl"
## The vehicle's handle counts as at a position this close (is_equal(..., 0.2), Driver.cpp:8182)
const HANDLE_TOLERANCE:float = 0.2
## control_braking_force() (Driver.cpp:8116-8155): braking starts past the gravity by these
## [m/s2]; standing uphill and rolling back faster than ROLLING_BACK_SPEED [km/h] brakes too
const BRAKING_MARGIN:float = 0.1
const SUDDEN_BRAKING_MARGIN:float = 0.5
const RELEASING_MARGIN:float = 0.05
const RELEASING_TABLE_FACTOR:float = 0.51
const RELEASING_EXCESS:float = 0.05
const ROLLING_BACK_GRAVITY:float = -0.05
const ROLLING_BACK_SPEED:float = -0.1
## Flat enough for the independent brake alone at a stop (Driver.cpp:8176)
const FLAT_GRAVITY:float = 0.01
## The brake's reaction the next adjustment waits for [s] (Driver.cpp:8121-8130, 8144-8149)
const BRAKE_DELAY_BASE:float = 3.0
const BRAKE_DELAY_SHARE:float = 0.5
const RELEASE_DELAY_SHARE:float = 3.0
## The goods delay setting (bdelay_G, hamulce.h:49), and which of BrakeDelay[] the original reads
## for applying and releasing past it (P, R) and at it
const DELAY_SETTING_G:int = 1
const DELAY_RELEASE_P:int = 0
const DELAY_APPLY_P:int = 1
const DELAY_RELEASE_G:int = 2
const DELAY_APPLY_G:int = 3
## The releaser (control_releaser(), Driver.cpp:8191-8250): the handles that have one, and the
## pressures [bar] it is pressed at - an empty pipe, a released cylinder, a charged control
## reservoir - and never past an overcharged pipe
const RELEASER:StringName = &"releaser_bt"
const RELEASER_HANDLES:Array[int] = [
    VehicleBrake.BRAKE_HANDLE_TYPE_FV4A, VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_6P, VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_K5P,
    VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_K8P, VehicleBrake.BRAKE_HANDLE_TYPE_M394,
]
const EMPTY_PIPE_PRESSURE:float = 3.0
const RELEASED_BRAKE_PRESSURE:float = 0.4
const CHARGED_CONTROL_RESERVOIR:float = 4.9
const OVERCHARGED_PIPE_PRESSURE:float = 5.2
## The positions of MHZ_K8P and MHZ_EN57 (Driver.cpp:4072-4092)
const K8P_FULL_POSITION:float = 10.0
const K8P_FULL_FROM:float = 4.5
const K8P_STRONG_POSITION:float = 9.0
const K8P_STRONG_FROM:float = 3.70
const K8P_SCALE:float = 0.4
const K8P_OFFSET:float = 0.1
const K8P_STEP:float = 0.15
## A powered vehicle (Power > 1, Driver.cpp:3076-3081)
const POWERED:float = 1.0
## The control reservoir a vehicle charges to (Driver.cpp:3114)
const FULL_CONTROL_RESERVOIR:float = 5.0
const POSITION_CORRECTION_SCALE:float = 2.5
const FV4A_CONTROL_PRESSURE_SHARE:float = 0.2
## deltaAcc of the braking table positions: 4 x per full position (Driver.cpp:3126)
const TABLE_STEPS:float = 4.0

## BrakeCtrlPosition
var position:float = POSITION_RUNNING
## fBrakeTime [s]
var delay:float = 0.0
## fBrake_a0[0], fBrake_a1[0] - the braking table at the current speed, zero until ported
var _table_a0:float = 0.0
var _table_a1:float = 0.0


## One decision of the driver about the brakes, `elapsed` [s] after the last one
## (control_braking_force(), Driver.cpp:8065-8190)
func control(
    vehicle:RID, cab:int, order:int, speed:MaszynaLegacyDriverSpeed, trainset:MaszynaLegacyDriverTrainset,
    directional_speed:float, elapsed:float
) -> void:
    delay -= elapsed
    var acceleration:float = speed.acceleration_desired
    var gravity:float = trainset.gravity_acceleration
    var disconnecting:bool = order == MaszynaLegacyAIDriver.Order.DISCONNECT
    # accelerating, it does not brake - but not while uncoupling
    if acceleration > 0.0 and not disconnecting:
        _release(vehicle, cab, acceleration, trainset)
    if (acceleration < gravity - BRAKING_MARGIN and trainset.acceleration > acceleration + _table_a1) \
            or (gravity < ROLLING_BACK_GRAVITY and directional_speed < ROLLING_BACK_SPEED):
        if delay < 0.0 or acceleration < gravity - SUDDEN_BRAKING_MARGIN or position <= POSITION_RUNNING:
            if _increase(vehicle, cab, acceleration, trainset):
                delay = (BRAKE_DELAY_BASE + BRAKE_DELAY_SHARE
                        * (_brake_delay(vehicle, DELAY_APPLY_P, DELAY_APPLY_G) - BRAKE_DELAY_BASE)) * BRAKE_DELAY_SHARE
    if acceleration < gravity - RELEASING_MARGIN \
            and (acceleration - _table_a1 * RELEASING_TABLE_FACTOR) - trainset.acceleration > RELEASING_EXCESS \
            and not disconnecting and speed.velocity_desired > 0.0:
        if _decrease(vehicle, cab, acceleration, trainset):
            delay = _brake_delay(vehicle, DELAY_RELEASE_P, DELAY_RELEASE_G) / RELEASE_DELAY_SHARE * BRAKE_DELAY_SHARE
    # at a stop: the locomotive held by its own brake on the flat, the train released
    # (Driver.cpp:8166-8180)
    var standing:bool = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0)) < MaszynaLegacyDriverTrainset.NO_MOVEMENT_SPEED
    if standing and (speed.velocity_desired == 0.0 or acceleration <= MaszynaLegacyDriverSpeed.NO_ACCELERATION):
        var joining:int = (MaszynaLegacyAIDriver.Order.DISCONNECT | MaszynaLegacyAIDriver.Order.CONNECT
                | MaszynaLegacyAIDriver.Order.CHANGE_DIRECTION)
        if not order & joining and absf(gravity) < FLAT_GRAVITY:
            _apply_independent_brake_only(vehicle, cab)
        if order & MaszynaLegacyAIDriver.Order.CHANGE_DIRECTION:
            _set_local_brake(vehicle, cab, LOCAL_BRAKE_RELEASED)
    _apply_handle(vehicle, cab)
    _control_releaser(vehicle, cab, acceleration)


## control_releaser() (Driver.cpp:8191-8250): the releaser held while the driver wants to go and
## the pipe is empty or its own brake overcharged - a locomotive whose control reservoir stayed
## fuller than the pipe keeps braking otherwise
func _control_releaser(vehicle:RID, cab:int, acceleration:float) -> void:
    var config:Dictionary = RailVehicleServer.vehicle_dump_config(vehicle)
    var train_type:int = int(config.get("train_type", VehicleController.TRAIN_TYPE_DEFAULT))
    if not int(config.get("brake_system", VehicleBrake.BRAKE_SYSTEM_PNEUMATIC)) == VehicleBrake.BRAKE_SYSTEM_PNEUMATIC \
            or train_type == VehicleController.TRAIN_TYPE_EZT or train_type == VehicleController.TRAIN_TYPE_DMU \
            or not int(config.get("brake_handle_type", 0)) in RELEASER_HANDLES:
        return
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(vehicle)
    var pipe:float = float(state.get("pipe_pressure", 0.0))
    var actuate:bool = acceleration > MaszynaLegacyDriverSpeed.NO_ACCELERATION and (pipe < EMPTY_PIPE_PRESSURE
            or (float(state.get("brake_air_pressure", 0.0)) > RELEASED_BRAKE_PRESSURE
                and float(state.get("brake_control_reservoir_pressure", 0.0)) > CHARGED_CONTROL_RESERVOIR))
    if pipe > OVERCHARGED_PIPE_PRESSURE:
        actuate = false
    var releasing:bool = state.get("brake_releaser_active", false)
    if actuate:
        # some vehicles take the releaser only with the master controller at zero
        MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
        if not releasing:
            CabinSystem.act(vehicle, cab, RELEASER, &"hold")
    elif releasing:
        CabinSystem.act(vehicle, cab, RELEASER, &"release")


## trainbrakeapply (driverhints.cpp:810-825): the train brake applied to uncouple; the handle takes
## it on the next control(). The electro-pneumatic brake's own position is not ported (TODO.md).
func apply_train_brake() -> void:
    position = POSITION_UNCOUPLING


## independentbrakerelease (driverhints.cpp:901-910): the local brake off, to press the buffers
func release_local_brake(vehicle:RID, cab:int) -> void:
    _set_local_brake(vehicle, cab, LOCAL_BRAKE_RELEASED)


## brakingforcesetzero (driverhints.cpp): DecBrake() until nothing is left to release
func _release(vehicle:RID, cab:int, acceleration:float, trainset:MaszynaLegacyDriverTrainset) -> void:
    while _decrease(vehicle, cab, acceleration, trainset):
        pass


## IncBrake() (Driver.cpp:3014-3170) for the individual and the pneumatic brake; true when a
## control moved
func _increase(vehicle:RID, cab:int, acceleration:float, trainset:MaszynaLegacyDriverTrainset) -> bool:
    var local_steps:int = 1 + floori(0.5 + absf(acceleration))
    var system:int = int(RailVehicleServer.vehicle_dump_config(vehicle).get("brake_system", VehicleBrake.BRAKE_SYSTEM_PNEUMATIC))
    if system == VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL or _is_standalone(vehicle, trainset):
        # hamowanie lokalnym bo luzem jedzie - a trainset of engines alone brakes with its own brake
        return _step_local_brake(vehicle, cab, local_steps)
    if position + 1.0 == POSITION_MAX:
        if acceleration < EMERGENCY_ACCELERATION:
            return _add_position(1.0)
        return false
    # the wagons whose control reservoir is overcharged need the pipe lower (Driver.cpp:3107-3124)
    var correction:float = 0.0
    for other:RID in trainset.vehicles:
        var state:Dictionary = RailVehicleServer.vehicle_dump_state(other)
        if not state.get("brake_is_cut_off", false):
            correction -= (minf(FULL_CONTROL_RESERVOIR, float(state.get("brake_control_reservoir_pressure", 0.0)))
                    - FULL_CONTROL_RESERVOIR) * float(state.get("mass_total", 0.0))
    correction = correction / trainset.mass * POSITION_CORRECTION_SCALE if trainset.mass > 0.0 else 0.0
    if int(RailVehicleServer.vehicle_dump_config(vehicle).get("brake_handle_type", 0)) == VehicleBrake.BRAKE_HANDLE_TYPE_FV4A:
        correction += float(CabinSystem.vehicle_state_value(vehicle, "brake_handle_control_pressure", 0.0)) * FV4A_CONTROL_PRESSURE_SHARE
    var excess:float = -acceleration - (_table_a0 + TABLE_STEPS * (position - 1.0 - correction) * _table_a1)
    if excess <= _table_a1:
        return false
    if position < 0.1:
        return _add_position(BRAKING_INITIAL_LEVEL)
    var moved:bool = _add_position(BRAKING_LEVEL_INCREASE)
    if excess > 2.0 * _table_a1 and position + BRAKING_LEVEL_INCREASE <= DEEPEST_DOUBLE_STEP:
        _add_position(BRAKING_LEVEL_INCREASE)
    return moved


## DecBrake() (Driver.cpp:3254-3300) for the individual and the pneumatic brake; true when a
## control moved
func _decrease(vehicle:RID, cab:int, acceleration:float, trainset:MaszynaLegacyDriverTrainset) -> bool:
    var system:int = int(RailVehicleServer.vehicle_dump_config(vehicle).get("brake_system", VehicleBrake.BRAKE_SYSTEM_PNEUMATIC))
    if system == VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL:
        return _step_local_brake(vehicle, cab, -(1 + floori(0.5 + absf(acceleration))))
    # without a braking table it always releases
    var shortfall:float = -1.0
    if not _table_a0 == 0.0 or not _table_a1 == 0.0:
        shortfall = -acceleration - (_table_a0 + TABLE_STEPS * (position - 1.0) * _table_a1)
    var moved:bool = false
    if shortfall < 0.0 and position > POSITION_RUNNING:
        moved = _add_position(RELEASE_STEP)
        if position < RUNNING_RETURN_POSITION:
            position = POSITION_RUNNING
    if not moved:
        moved = _step_local_brake(vehicle, cab, -LOCAL_RELEASE_STEPS)
    return moved


## BrakeLevelAdd() (Driver.cpp:3836): false once it would leave the range
func _add_position(change:float) -> bool:
    position = clampf(position + change, POSITION_MIN, POSITION_MAX)
    return position < POSITION_MAX if change > 0.0 else position > POSITION_CHARGING


## Whether the trainset is only engines coupled to be driven together - it brakes with the local
## brake then (Driver.cpp:3068-3084)
func _is_standalone(vehicle:RID, trainset:MaszynaLegacyDriverTrainset) -> bool:
    var controlled:Array[RID] = RailVehicleServer.vehicle_get_coupled(
            vehicle, MaszynaLegacyDriverTrainset.FRONT_END, VehicleController.COUPLING_ELEMENT_CONTROL)
    if not controlled.size() == trainset.vehicles.size():
        return false
    for other:RID in trainset.vehicles:
        if float(RailVehicleServer.vehicle_dump_config(other).get("power", 0.0)) <= POWERED:
            return false
    return true


## apply_independent_brake_only() (Driver.cpp:8178-8189): the local brake on if the train brake
## runs, otherwise the train brake to running first
func _apply_independent_brake_only(vehicle:RID, cab:int) -> void:
    var running:float = float(RailVehicleServer.vehicle_dump_config(vehicle).get("brakes_controller_position_drive", 0.0))
    if absf(float(CabinSystem.vehicle_state_value(vehicle, "brake_controller_position", 0.0)) - running) <= HANDLE_TOLERANCE:
        _set_local_brake(vehicle, cab, LOCAL_BRAKE_APPLIED)
    else:
        position = POSITION_RUNNING


## IncLocalBrakeLevel()/DecLocalBrakeLevel() through the knob; true when it moved
func _step_local_brake(vehicle:RID, cab:int, steps:int) -> bool:
    var current:float = float(CabinSystem.vehicle_state_value(vehicle, "brake_local_position_normalized", 0.0))
    var wanted:float = clampf(current + steps / LOCAL_BRAKE_POSITIONS, LOCAL_BRAKE_RELEASED, LOCAL_BRAKE_APPLIED)
    if wanted == current:
        return false
    _set_local_brake(vehicle, cab, wanted)
    return true


func _set_local_brake(vehicle:RID, cab:int, value:float) -> void:
    if not float(CabinSystem.vehicle_state_value(vehicle, "brake_local_position_normalized", 0.0)) == value:
        CabinSystem.act(vehicle, cab, LOCAL_BRAKE, &"set", value)


## SetTimeControllers() (Driver.cpp:4067-4092): the driver's position to the vehicle's handle, as
## the handle's type takes it - FV4a as it is, MHZ_K8P and MHZ_EN57 by their table
func _apply_handle(vehicle:RID, cab:int) -> void:
    var config:Dictionary = RailVehicleServer.vehicle_dump_config(vehicle)
    var handle_position:float
    match int(config.get("brake_handle_type", 0)):
        VehicleBrake.BRAKE_HANDLE_TYPE_FV4A:
            handle_position = position
        VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_K8P, VehicleBrake.BRAKE_HANDLE_TYPE_MHZ_EN57:
            if position == POSITION_RUNNING:
                handle_position = float(config.get("brakes_controller_position_drive", 0.0))
            elif position == POSITION_CHARGING:
                handle_position = float(config.get("brakes_controller_position_filling", 0.0))
            elif position == POSITION_LAP:
                handle_position = float(config.get("brakes_controller_position_cutoff", 0.0))
            elif position > K8P_FULL_FROM:
                handle_position = K8P_FULL_POSITION
            elif position > K8P_STRONG_FROM:
                handle_position = K8P_STRONG_POSITION
            else:
                handle_position = roundf((position * K8P_SCALE - K8P_OFFSET) / K8P_STEP)
        _:
            return
    if float(CabinSystem.vehicle_state_value(vehicle, "brake_controller_position", 0.0)) == handle_position:
        return
    var low:float = float(config.get("brakes_controller_position_min", 0.0))
    var high:float = float(config.get("brakes_controller_position_max", 0.0))
    if high > low:
        CabinSystem.act(vehicle, cab, TRAIN_BRAKE, &"set", (handle_position - low) / (high - low))


## The brake's delay [s] at the setting in use: the one read past G, or at G (Driver.cpp:8124, 8146)
func _brake_delay(vehicle:RID, past_g:int, at_g:int) -> float:
    var delays:PackedFloat64Array = RailVehicleServer.vehicle_dump_config(vehicle).get("brake_delay_times", PackedFloat64Array())
    if delays.size() <= maxi(past_g, at_g):
        return 0.0
    var setting:int = int(CabinSystem.vehicle_state_value(vehicle, "brake_delay_setting", DELAY_SETTING_G))
    return delays[past_g] if setting > DELAY_SETTING_G else delays[at_g]
