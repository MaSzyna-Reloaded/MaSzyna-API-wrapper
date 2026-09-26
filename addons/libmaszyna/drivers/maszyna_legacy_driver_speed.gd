@tool
extends RefCounted
class_name MaszynaLegacyDriverSpeed

## The speed and the acceleration the original's driver wants on every update
## (TController::pick_optimal_speed(), Driver.cpp:7297-7400): the trainset's limit, the timetable's,
## the shunting speed, the speed allowed by the orders, the track's limit; then the acceleration
## towards it (adjust_desired_speed_for_target_speed(), adjust_desired_speed_for_current_speed()).
##
## Without a speed table yet: there is no next speed (VelNext) and no distance to it, nothing ahead
## (Obstacle), no load exchange and no waiting (fStopTime); the braking characteristic (fBrake_a0/a1) and the coupler
## strength limit come with the brakes - see TODO.md, "Drivers".

## AccPreferred of a calm driver [m/s2] (EasyAcceleration, Driver.cpp:152)
const EASY_ACCELERATION:float = 0.85
## Below this [m/s2] the driver does not ask for power (EU07_AI_NOACCELERATION, Driver.h:23)
const NO_ACCELERATION:float = -0.05
## The speed [km/h] a train may run over a limit before it brakes, and under it before it adds
## power (fVelPlus, fVelMinus, Driver.cpp:1882-1883)
const VELOCITY_PLUS:float = 5.0
const VELOCITY_MINUS:float = 5.0
## The deceleration a train starts braking at [m/s2] (fAccThreshold, Driver.cpp:1891-1894)
const ACCELERATION_THRESHOLD:float = -0.2
## Standing, it holds itself back this much (Driver.cpp:7425)
const STANDING_ACCELERATION:float = -0.01
## Too fast where it should stand: brake firmly (Driver.cpp:7730)
const STOP_ACCELERATION:float = -0.85
## The acceleration asked for is smoothed, braking is not (Driver.cpp:7383-7387)
const SMOOTHING_NEW:float = 0.2
const SMOOTHING_KEPT:float = 0.8
## A train's acceleration asked for stays within this [m/s2] (Driver.cpp:7395-7398)
const MAX_ACCELERATION:float = 0.9
## -1 is no limit (min_speed(), utilities.h:284)
const NO_LIMIT:float = -1.0

## VelDesired [km/h]
var velocity_desired:float = 0.0
## AccDesired [m/s2]
var acceleration_desired:float = 0.0
## fVelMax - the lowest top speed of the trainset [km/h]
var velocity_max:float = NO_LIMIT
## VelSignalNext - the speed at the next signal, from the speed table [km/h]
var signal_velocity_next:float = 0.0
## fAccDesiredAv
var _acceleration_average:float = 0.0


## `order` the current one; `velocity` the speed allowed (VelSignal), `timetable_velocity` the
## timetable's limit (TTVmax), `track_velocity` the track's (RunningTrack.Velmax), `speed` the
## vehicle's along the way it drives (DirectionalVel()) [km/h]
func pick(
    order:int, active:bool, stop_here:bool, velocity:float, shunt_velocity:float,
    timetable_velocity:float, track_velocity:float, speed:float, trainset:MaszynaLegacyDriverTrainset
) -> void:
    velocity_max = NO_LIMIT
    for vehicle:RID in trainset.vehicles:
        velocity_max = _min_speed(velocity_max, float(RailVehicleServer.vehicle_dump_config(vehicle).get("max_speed", NO_LIMIT)))
    velocity_desired = velocity_max
    acceleration_desired = EASY_ACCELERATION
    if order & MaszynaLegacyAIDriver.Order.OBEY_TRAIN and timetable_velocity > 0.0:
        velocity_desired = _min_speed(velocity_desired, timetable_velocity)
    # shunting is calm
    if not order & (MaszynaLegacyAIDriver.Order.OBEY_TRAIN | MaszynaLegacyAIDriver.Order.BANK):
        velocity = shunt_velocity
        velocity_desired = _min_speed(velocity_desired, shunt_velocity)
    if not active:
        velocity_desired = 0.0
        acceleration_desired = minf(acceleration_desired, NO_ACCELERATION)
        return
    # told to put the vehicle away or to turn: stop first
    if order & (MaszynaLegacyAIDriver.Order.RELEASE_ENGINE | MaszynaLegacyAIDriver.Order.CHANGE_DIRECTION):
        velocity = 0.0
    # adjust_desired_speed_for_limits() (Driver.cpp:7516-7576)
    if velocity >= 0.0:
        velocity_desired = _min_speed(velocity_desired, velocity)
    if track_velocity >= 0.0:
        velocity_desired = _min_speed(velocity_desired, track_velocity)
    var driving:int = (MaszynaLegacyAIDriver.Order.SHUNT | MaszynaLegacyAIDriver.Order.LOOSE_SHUNT
            | MaszynaLegacyAIDriver.Order.OBEY_TRAIN | MaszynaLegacyAIDriver.Order.BANK)
    if (order & driving and stop_here and absf(speed) < MaszynaLegacyDriverTrainset.NO_MOVEMENT_SPEED
            and signal_velocity_next == 0.0) or order == MaszynaLegacyAIDriver.Order.WAIT_FOR_ORDERS:
        velocity_desired = 0.0
    # adjust_desired_speed_for_target_speed() without a next speed (Driver.cpp:7412-7425)
    acceleration_desired = EASY_ACCELERATION if not velocity_desired == 0.0 else STANDING_ACCELERATION
    # adjust_desired_speed_for_current_speed() (Driver.cpp:7722-7760)
    if speed > velocity_desired:
        if velocity_desired == 0.0:
            acceleration_desired = minf(acceleration_desired, STOP_ACCELERATION)
        elif speed > velocity_desired + VELOCITY_PLUS:
            acceleration_desired = minf(acceleration_desired, NO_ACCELERATION + ACCELERATION_THRESHOLD)
        else:
            acceleration_desired = minf(acceleration_desired, maxf(0.0, EASY_ACCELERATION))
    # pick_optimal_speed() (Driver.cpp:7383-7398)
    if acceleration_desired > NO_ACCELERATION:
        _acceleration_average = SMOOTHING_NEW * acceleration_desired + SMOOTHING_KEPT * _acceleration_average
        acceleration_desired = _acceleration_average
    if velocity_desired == 0.0:
        acceleration_desired = minf(acceleration_desired, NO_ACCELERATION)
    acceleration_desired = clampf(minf(acceleration_desired, EASY_ACCELERATION), -MAX_ACCELERATION, MAX_ACCELERATION)


## min_speed() (utilities.h:284): the lower of two speeds, where NO_LIMIT is none
static func _min_speed(left:float, right:float) -> float:
    if left == NO_LIMIT:
        return right
    if right == NO_LIMIT:
        return left
    return minf(left, right)
