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
## The braking table (fBrake_a0/a1) is worked out of the trainset's own brakes as the original does
## (CheckVehicles(), Driver.cpp:2154-2340): the deceleration of the whole train at a quarter and at
## full pressure over its speeds, the threshold braking starts at (fAccThreshold), the reaction of
## its brakes and the brake setting each vehicle gets (G, P, R).
##
## Not ported yet: the electro-pneumatic and the EMU/DMU braking, the time-controlled handles
## (MHZ_K5P, MHZ_6P, M394, H14K1, St113, H1405 - moved every frame in the original), the manual
## brake and the spring brake let off by the crew, the pipe unlocking before the releaser, the
## weather's friction - see TODO.md, "Drivers".

## BrakeCtrlPosition: lap, charging, running, and the range (gbh_*, Driver.h:196-200)
const POSITION_LAP:float = -2.0
const POSITION_CHARGING:float = -1.0
const POSITION_RUNNING:float = 0.0
const POSITION_MIN:float = -2.0
const POSITION_MAX:float = 6.0
## Uncoupling brakes the train at this position before it presses the buffers (trainbrakeapply,
## driverhints.cpp:810-817)
const POSITION_UNCOUPLING:float = 3.0
## BrakingInitialLevel, BrakingLevelIncrease (Driver.h:449-450): of a goods train 1.25 to start
## (Driver.cpp:2306-2314)
const BRAKING_INITIAL_LEVEL:float = 1.0
const CARGO_INITIAL_LEVEL:float = 1.25
const BRAKING_LEVEL_INCREASE:float = 0.25
## The braking table (BrakeAccTableSize, Driver.h:194; CheckVehicles(), Driver.cpp:2284-2300):
## steps of half the top speed, the brake force at a quarter and at full pressure, the rolling
## resistance per km/h, and the full positions the handle's twelve quarter steps make
const TABLE_SIZE:int = 20
const TABLE_SPEED_SHARE:float = 0.5
const TABLE_LOW_PRESSURE:float = 0.25
const TABLE_FULL_PRESSURE:float = 1.0
const TABLE_ROLLING:float = 0.001
const TABLE_FULL_STEPS:float = 12.0
## fAccThreshold of shunting (Driver.cpp:2266-2276) - an EMU earlier, and one with induction motors
## earlier still; of a train out of the table with the passenger's 4 and the goods' 1 full step
## (Driver.cpp:2315-2338), an EMU's or DMU's at most these
const SHUNT_THRESHOLD:float = -0.2
const EMU_SHUNT_THRESHOLD:float = -0.55
const DMU_SHUNT_THRESHOLD:float = -0.45
const INDUCTION_EMU_EARLIER:float = 0.10
const PASSENGER_THRESHOLD_STEPS:float = 4.0
const GOODS_THRESHOLD_STEPS:float = 1.0
const EMU_THRESHOLD_STEPS:float = 4.0
const EP_THRESHOLD_STEPS:float = 8.0
const DMU_THRESHOLD_STEPS:float = 8.0
const EMU_THRESHOLD_MAX:float = -0.75
const INDUCTION_EMU_THRESHOLD_MAX:float = -0.60
## An EMU's or DMU's threshold moves with its speed (Driver.cpp:6028-6031)
const MULTIPLE_UNIT_SPEED_SHARE:float = 0.015
const MULTIPLE_UNIT_SPEED_MIN:float = 0.5
## fBrakeReaction [s per km/h]: 1 plus this per metre of a passenger's or goods train; an EMU's
## or DMU's 0.25 (Driver.cpp:2325-2337)
const BRAKE_REACTION:float = 1.0
const MULTIPLE_UNIT_REACTION:float = 0.25
const PASSENGER_REACTION_PER_METRE:float = 0.004
const GOODS_REACTION_PER_METRE:float = 0.005
## The train is a passenger one with fewer goods wagons than this and than its others; a goods
## one under these lengths [m] and masses [kg] braked P, then GP, else G; GP puts this many
## wagons behind the engine on G (Driver.cpp:2166-2231)
const PASSENGER_GOODS_LIMIT:int = 4
const GOODS_P_LENGTH:float = 300.0
const GOODS_P_MASS:float = 600000.0
const GOODS_GP_LENGTH:float = 500.0
const GOODS_GP_MASS:float = 1300000.0
const GP_WAGONS_ON_G:int = 5
## IsHeavyCargoTrain (Driver.cpp:2304): past this a0 of the first step and mass per vehicle [kg]
const HEAVY_CARGO_A0:float = 0.4
const HEAVY_CARGO_MASS:float = 50000.0
## BrakeAccFactor() (Driver.cpp:2716-2733): the reaction counts 1.5 times with the handle under this
const FACTOR_RELEASED_POSITION:float = 0.5
const FACTOR_RELEASED_REACTION:float = 1.5
## braking_distance_multiplier() (Driver.cpp:1731-1771): above FAST_TARGET [km/h] none; under
## STOP_TARGET a goods train or one downhill past DOWNHILL_GRAVITY, braking harder than
## MULTIPLIER_A0, up to twice; between, up to MOST times for the slowest
const FAST_TARGET:float = 65.0
const STOP_TARGET:float = 5.0
const DOWNHILL_GRAVITY:float = 0.025
const MULTIPLIER_A0:float = 0.2
const STOP_MULTIPLIER:float = 2.0
const MOST_MULTIPLIER:float = 3.0
const MULTIPLIER_SPAN:float = 60.0
## A DMU's automatic gearbox brakes the last leg to a stop earlier: 1 + half per vehicle, 2-4
## times, easing out towards DMU_EASING_SPEED [km/h]
const DMU_MULTIPLIER_PER_VEHICLE:float = 0.5
const DMU_MULTIPLIER_MIN:float = 2.0
const DMU_MULTIPLIER_MAX:float = 4.0
const DMU_EASING_SPEED:float = 40.0
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
## fAccThreshold [m/s2]: braking starts past it
var acceleration_threshold:float = SHUNT_THRESHOLD
## IsCargoTrain, IsHeavyCargoTrain
var cargo:bool = false
var heavy_cargo:bool = false
## fBrake_a0[0], fBrake_a1[0] - the braking table at the current speed
var _table_a0:float = 0.0
var _table_a1:float = 0.0
## fBrake_a0[1..], fBrake_a1[1..]
var _a0:PackedFloat64Array = []
var _a1:PackedFloat64Array = []
## fNominalAccThreshold, fBrakeReaction, BrakingInitialLevel
var _nominal_threshold:float = SHUNT_THRESHOLD
var _reaction:float = BRAKE_REACTION
var _initial_level:float = BRAKING_INITIAL_LEVEL
## The trainset and the order the table was worked out for; a DMU (its gearbox)
var _checked:int = 0
var _dmu:bool = false


func _init() -> void:
    _a0.resize(TABLE_SIZE + 1)
    _a1.resize(TABLE_SIZE + 1)


## What the driver makes of its trainset's brakes (CheckVehicles(), Driver.cpp:2154-2340), again
## only when the trainset or the kind of order changed; then the table at the current speed
## (UpdateSituation(), Driver.cpp:6023-6031). `in_control`: it may set the brakes of its own vehicle.
func read_trainset(vehicle:RID, order:int, trainset:MaszynaLegacyDriverTrainset, in_control:bool) -> void:
    var config:Dictionary = RailVehicleServer.vehicle_dump_config(vehicle)
    var train_type:int = int(config.get("train_type", VehicleController.TRAIN_TYPE_DEFAULT))
    var emu:bool = train_type == VehicleController.TRAIN_TYPE_EZT
    var dmu:bool = train_type == VehicleController.TRAIN_TYPE_DMU
    _dmu = dmu
    var induction:bool = int(RailVehicleServer.vehicle_dump_state(vehicle).get("engine_type", VehicleEngine.NONE)) \
            == VehicleEngine.ELECTRIC_INDUCTION_MOTOR
    var velocity_max:float = float(config.get("max_speed", 0.0))
    var train:bool = order & (MaszynaLegacyAIDriver.Order.OBEY_TRAIN | MaszynaLegacyAIDriver.Order.BANK)
    var shunt:bool = order & (MaszynaLegacyAIDriver.Order.SHUNT | MaszynaLegacyAIDriver.Order.LOOSE_SHUNT)
    var checked:int = hash([trainset.vehicles, train, shunt])
    if not checked == _checked:
        _checked = checked
        var passenger:bool = _set_brake_delays(vehicle, trainset, in_control)
        _a0.fill(0.0)
        _a1.fill(0.0)
        if shunt:
            _nominal_threshold = EMU_SHUNT_THRESHOLD if emu else (DMU_SHUNT_THRESHOLD if dmu else SHUNT_THRESHOLD)
            if emu and induction:
                _nominal_threshold += INDUCTION_EMU_EARLIER
            acceleration_threshold = _nominal_threshold
        if train and trainset.mass > 0.0 and velocity_max > 0.0:
            _build_table(trainset, velocity_max)
            cargo = int(RailVehicleServer.vehicle_dump_state(vehicle).get("brake_delay_setting", 0)) == DELAY_SETTING_G
            var engines:int = 0
            for other:RID in trainset.vehicles:
                if float(RailVehicleServer.vehicle_dump_config(other).get("power", 0.0)) > POWERED:
                    engines += 1
            heavy_cargo = cargo and _a0[1] > HEAVY_CARGO_A0 and trainset.vehicles.size() - engines > 0 \
                    and trainset.mass / trainset.vehicles.size() > HEAVY_CARGO_MASS
            _initial_level = CARGO_INITIAL_LEVEL if cargo else BRAKING_INITIAL_LEVEL
            var last:int = TABLE_SIZE
            if emu:
                var steps:float = EP_THRESHOLD_STEPS if int(config.get("brake_system", 0)) == VehicleBrake.BRAKE_SYSTEM_ELECTRO_PNEUMATIC \
                        else EMU_THRESHOLD_STEPS
                _nominal_threshold = maxf(INDUCTION_EMU_THRESHOLD_MAX if induction else EMU_THRESHOLD_MAX,
                        -_a0[last] - steps * _a1[last])
                _reaction = MULTIPLE_UNIT_REACTION
            elif dmu:
                _nominal_threshold = maxf(EMU_THRESHOLD_MAX, -_a0[last] - DMU_THRESHOLD_STEPS * _a1[last])
                _reaction = MULTIPLE_UNIT_REACTION
            elif passenger:
                _nominal_threshold = -_a0[last] - PASSENGER_THRESHOLD_STEPS * _a1[last]
                _reaction = BRAKE_REACTION + trainset.length * PASSENGER_REACTION_PER_METRE
            else:
                _nominal_threshold = -_a0[last] - GOODS_THRESHOLD_STEPS * _a1[last]
                _reaction = BRAKE_REACTION + trainset.length * GOODS_REACTION_PER_METRE
            acceleration_threshold = _nominal_threshold
    # the table at the current speed
    var speed:float = float(RailVehicleServer.vehicle_dump_state(vehicle).get("speed", 0.0))
    var index:int = clampi(int(TABLE_SIZE * speed / velocity_max) if velocity_max > 0.0 else 1, 1, TABLE_SIZE)
    _table_a0 = _a0[index]
    _table_a1 = _a1[index]
    if emu or dmu:
        var share:float = clampf(speed * MULTIPLE_UNIT_SPEED_SHARE, MULTIPLE_UNIT_SPEED_MIN, 1.0)
        acceleration_threshold = _nominal_threshold * share - _a0[TABLE_SIZE] * (1.0 - share)


## BrakeAccFactor() (Driver.cpp:2716-2733): braking deeper the faster and the nearer the stop, and
## the more the train lags behind the deceleration wanted
func factor(
    speed:MaszynaLegacyDriverSpeed, route:MaszynaLegacyDriverRoute, trainset:MaszynaLegacyDriverTrainset,
    vehicle_speed:float
) -> float:
    var acceleration:float = speed.acceleration_desired
    if acceleration_threshold == 0.0 or acceleration >= 0.0:
        return 1.0
    if not (speed.proximity_distance > route.min_proximity or vehicle_speed > speed.velocity_desired + route.velocity_plus):
        return 1.0
    var reaction:float = _reaction * (FACTOR_RELEASED_REACTION if position < FACTOR_RELEASED_POSITION else 1.0)
    return 1.0 + reaction * vehicle_speed / (maxf(0.0, speed.proximity_distance) + 1.0) \
            * ((acceleration - trainset.acceleration) / acceleration_threshold)


## braking_distance_multiplier() (Driver.cpp:1731-1771): how much longer than the braking distance
## the train needs to reach `target` [km/h] - the slower the target, the longer; a goods train or
## one downhill braking hard needs up to twice as long to stop
func distance_multiplier(target:float, vehicle_speed:float, trainset:MaszynaLegacyDriverTrainset) -> float:
    if target > FAST_TARGET:
        return 1.0
    if target < STOP_TARGET:
        if _dmu and vehicle_speed < DMU_EASING_SPEED and target == 0.0:
            var most:float = clampf(1.0 + trainset.vehicles.size() * DMU_MULTIPLIER_PER_VEHICLE, DMU_MULTIPLIER_MIN, DMU_MULTIPLIER_MAX)
            return lerpf(most, 1.0, vehicle_speed / DMU_EASING_SPEED)
        if _a0[1] > MULTIPLIER_A0 and (cargo or trainset.gravity_acceleration > DOWNHILL_GRAVITY):
            return lerpf(1.0, STOP_MULTIPLIER, clampf((_a0[1] - MULTIPLIER_A0) / MULTIPLIER_A0, 0.0, 1.0))
        return 1.0
    return lerpf(MOST_MULTIPLIER, 1.0, (target - STOP_TARGET) / MULTIPLIER_SPAN)


## CheckVehicles() 4. (Driver.cpp:2284-2300): the whole train's deceleration by the brakes at a
## quarter and at full pressure, step by step of half its top speed
func _build_table(trainset:MaszynaLegacyDriverTrainset, velocity_max:float) -> void:
    var step:float = velocity_max * TABLE_SPEED_SHARE / TABLE_SIZE
    for other:RID in trainset.vehicles:
        var brake:VehicleBrake = RailVehicleServer.vehicle_component_get(other, VehicleComponentType.COMPONENT_BRAKES) as VehicleBrake
        if not brake:
            continue
        for index:int in TABLE_SIZE:
            var velocity:float = step * (1 + 2 * index)
            _a0[index + 1] += brake.get_force_at(TABLE_LOW_PRESSURE, velocity)
            _a1[index + 1] += brake.get_force_at(TABLE_FULL_PRESSURE, velocity)
    for index:int in TABLE_SIZE:
        _a1[index + 1] -= _a0[index + 1]
        _a0[index + 1] /= trainset.mass
        _a0[index + 1] += TABLE_ROLLING * step * (1 + 2 * index)
        _a1[index + 1] /= TABLE_FULL_STEPS * trainset.mass


## CheckVehicles() 1.-3. (Driver.cpp:2154-2237): the brake setting the train needs, from its
## wagons - a passenger train P or R, a goods train P, GP or G by its length and mass - put on
## every vehicle by the crew (`auto_rewident`, the rewident's own lever); the driver's vehicle only
## while the driver drives it. True for a passenger train (ustaw > 16).
func _set_brake_delays(vehicle:RID, trainset:MaszynaLegacyDriverTrainset, in_control:bool) -> bool:
    var fast:int = 0
    var goods:int = 0
    var passengers:int = 0
    for other:RID in trainset.vehicles:
        if float(RailVehicleServer.vehicle_dump_config(other).get("power", 0.0)) >= 1.0:
            continue
        var delays:int = int(RailVehicleServer.vehicle_dump_config(other).get("brake_delays", 0))
        if delays & VehicleBrake.BRAKE_DELAY_R:
            fast += 1
        elif delays & VehicleBrake.BRAKE_DELAY_G:
            goods += 1
        else:
            passengers += 1
    var passenger:bool = true
    var setting:int = VehicleBrake.BRAKE_DELAY_R
    if fast + goods + passengers > 0:
        passenger = goods < mini(PASSENGER_GOODS_LIMIT, fast + passengers)
        if passenger:
            setting = VehicleBrake.BRAKE_DELAY_P if goods and fast < goods + passengers else VehicleBrake.BRAKE_DELAY_R
        elif trainset.length < GOODS_P_LENGTH and trainset.mass < GOODS_P_MASS:
            setting = VehicleBrake.BRAKE_DELAY_P
        elif trainset.length < GOODS_GP_LENGTH and trainset.mass < GOODS_GP_MASS:
            # GP, in the original's own numbering the R of a goods train
            setting = VehicleBrake.BRAKE_DELAY_R
        else:
            setting = VehicleBrake.BRAKE_DELAY_G
    var behind_engine:int = 0
    for other:RID in trainset.vehicles:
        # the driver's own vehicle only while it drives it, and only a vehicle with a brake to set
        if other == vehicle and not in_control \
                or not RailVehicleServer.vehicle_component_get(other, VehicleComponentType.COMPONENT_BRAKES):
            continue
        var powered:bool = float(RailVehicleServer.vehicle_dump_config(other).get("power", 0.0)) > POWERED
        var delays:int = int(RailVehicleServer.vehicle_dump_config(other).get("brake_delays", 0))
        var own:int
        if passenger:
            own = VehicleBrake.BRAKE_DELAY_R if setting == VehicleBrake.BRAKE_DELAY_R and delays & VehicleBrake.BRAKE_DELAY_R \
                    else VehicleBrake.BRAKE_DELAY_P
        elif setting == VehicleBrake.BRAKE_DELAY_P:
            own = VehicleBrake.BRAKE_DELAY_G if powered else VehicleBrake.BRAKE_DELAY_P
        elif setting == VehicleBrake.BRAKE_DELAY_G:
            own = VehicleBrake.BRAKE_DELAY_G if delays & VehicleBrake.BRAKE_DELAY_G else VehicleBrake.BRAKE_DELAY_P
        else:
            if powered:
                behind_engine = 0
                own = VehicleBrake.BRAKE_DELAY_G
            else:
                behind_engine += 1
                own = VehicleBrake.BRAKE_DELAY_G if behind_engine <= GP_WAGONS_ON_G else VehicleBrake.BRAKE_DELAY_P
        RailVehicleServer.vehicle_send_command(other, "auto_rewident", own)
    return passenger


## One decision of the driver about the brakes, `elapsed` [s] after the last one
## (control_braking_force(), Driver.cpp:8065-8190)
func control(
    vehicle:RID, cab:int, order:int, speed:MaszynaLegacyDriverSpeed, trainset:MaszynaLegacyDriverTrainset,
    route:MaszynaLegacyDriverRoute, directional_speed:float, elapsed:float
) -> void:
    delay -= elapsed
    var acceleration:float = speed.acceleration_desired
    var brake_factor:float = factor(speed, route, trainset, absf(directional_speed))
    var gravity:float = trainset.gravity_acceleration
    var disconnecting:bool = order == MaszynaLegacyAIDriver.Order.DISCONNECT
    # accelerating, it does not brake - but not while uncoupling
    if acceleration > 0.0 and not disconnecting:
        _release(vehicle, cab, acceleration, trainset)
    if (acceleration < gravity - BRAKING_MARGIN and trainset.acceleration > acceleration + _table_a1) \
            or (gravity < ROLLING_BACK_GRAVITY and directional_speed < ROLLING_BACK_SPEED):
        if delay < 0.0 or acceleration < gravity - SUDDEN_BRAKING_MARGIN or position <= POSITION_RUNNING:
            if _increase(vehicle, cab, acceleration, brake_factor, trainset):
                delay = (BRAKE_DELAY_BASE + BRAKE_DELAY_SHARE
                        * (_brake_delay(vehicle, DELAY_APPLY_P, DELAY_APPLY_G) - BRAKE_DELAY_BASE)) * BRAKE_DELAY_SHARE
    if acceleration < gravity - RELEASING_MARGIN \
            and (acceleration - _table_a1 * RELEASING_TABLE_FACTOR) - trainset.acceleration > RELEASING_EXCESS \
            and not disconnecting and speed.velocity_desired > 0.0:
        if _decrease(vehicle, cab, acceleration, brake_factor, trainset):
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
    while _decrease(vehicle, cab, acceleration, 1.0, trainset):
        pass


## IncBrake() (Driver.cpp:3014-3170) for the individual and the pneumatic brake; true when a
## control moved
func _increase(
    vehicle:RID, cab:int, acceleration:float, brake_factor:float, trainset:MaszynaLegacyDriverTrainset
) -> bool:
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
    var excess:float = -acceleration * brake_factor - (_table_a0 + TABLE_STEPS * (position - 1.0 - correction) * _table_a1)
    if excess <= _table_a1:
        return false
    if position < 0.1:
        return _add_position(_initial_level)
    var moved:bool = _add_position(BRAKING_LEVEL_INCREASE)
    if excess > 2.0 * _table_a1 and position + BRAKING_LEVEL_INCREASE <= DEEPEST_DOUBLE_STEP:
        _add_position(BRAKING_LEVEL_INCREASE)
    return moved


## DecBrake() (Driver.cpp:3254-3300) for the individual and the pneumatic brake; true when a
## control moved
func _decrease(
    vehicle:RID, cab:int, acceleration:float, brake_factor:float, trainset:MaszynaLegacyDriverTrainset
) -> bool:
    var system:int = int(RailVehicleServer.vehicle_dump_config(vehicle).get("brake_system", VehicleBrake.BRAKE_SYSTEM_PNEUMATIC))
    if system == VehicleBrake.BRAKE_SYSTEM_INDIVIDUAL:
        return _step_local_brake(vehicle, cab, -(1 + floori(0.5 + absf(acceleration))))
    # without a braking table it always releases
    var shortfall:float = -1.0
    if not _table_a0 == 0.0 or not _table_a1 == 0.0:
        shortfall = -acceleration * brake_factor - (_table_a0 + TABLE_STEPS * (position - 1.0) * _table_a1)
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
