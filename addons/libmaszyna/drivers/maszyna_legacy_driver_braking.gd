@tool
extends RefCounted
class_name MaszynaLegacyDriverBraking

## The original driver's brakes (TController::control_braking_force(), Driver.cpp:8065-8190),
## operated through the cab as a player does (CabinSystem.act()).
##
## Only the release is ported: a driver that wants to accelerate does not brake. Braking to a
## speed or to a stop, the AI's own handle position (BrakeCtrlPosition), the brake table
## (fBrake_a0/a1), the releaser and the independent brake at a stop come next - see TODO.md,
## "Drivers".

const LOCAL_BRAKE:StringName = &"localbrake"
## The local brake knob's released position (value_min of its catalog entry)
const LOCAL_BRAKE_RELEASED:float = 0.0


## One decision of the driver about the brakes
static func control(vehicle:RID, cab:int, order:int, speed:MaszynaLegacyDriverSpeed) -> void:
    # accelerating, it does not brake - but not while uncoupling (Driver.cpp:8073-8079)
    if speed.acceleration_desired > 0.0 and not order == MaszynaLegacyAIDriver.Order.DISCONNECT:
        release(vehicle, cab)


## brakingforcesetzero (driverhints.cpp): the train brake to its driving position, the local brake
## off
static func release(vehicle:RID, cab:int) -> void:
    MaszynaLegacyDriverHints.release_train_brake(vehicle, cab)
    if float(CabinSystem.vehicle_state_value(vehicle, "brake_local_position_normalized", 0.0)) > LOCAL_BRAKE_RELEASED:
        CabinSystem.act(vehicle, cab, LOCAL_BRAKE, &"set", LOCAL_BRAKE_RELEASED)
