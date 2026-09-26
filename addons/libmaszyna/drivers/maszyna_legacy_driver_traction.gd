@tool
extends RefCounted
class_name MaszynaLegacyDriverTraction

## The original driver's tractive force (TController::control_tractive_force(), IncSpeed(),
## DecSpeed(), Driver.cpp:3406-3760, 7996-8063): power added while the trainset accelerates less
## than wanted and runs slower than wanted, taken off when it runs too fast or accelerates too much.
## Every change is a step of the cab's controllers, as a player makes it (CabinSystem.act()).
##
## Ported for the diesel-electric engine; the other engines have no power control yet, nor does
## anything here check the stretched couplers, the spring brake, the doors or the departure signal
## before adding power - see TODO.md, "Drivers".

const MASTER_CONTROLLER:StringName = MaszynaLegacyDriverHints.MASTER_CONTROLLER
const SECOND_CONTROLLER:StringName = MaszynaLegacyDriverHints.SECOND_CONTROLLER
## A limit of exactly this [km/h] is driven up to without the margin - under it a train would never
## move off (Driver.cpp:8008-8010)
const CRAWL_VELOCITY:float = 1.0
## Going uphill harder than this [m/s2], power is taken off only when the driver wants to slow down
## (Driver.cpp:8036)
const UPHILL_GRAVITY:float = -0.01
## The acceleration over the one wanted that takes power off on the flat (Driver.cpp:8037)
const EXCESS_ACCELERATION:float = 10.05


## One decision of the driver about the power (control_tractive_force(), Driver.cpp:7996-8040)
static func control(
    vehicle:RID, cab:int, speed:MaszynaLegacyDriverSpeed, trainset:MaszynaLegacyDriverTrainset,
    directional_speed:float
) -> void:
    var velocity_desired:float = speed.velocity_desired
    var acceleration_desired:float = speed.acceleration_desired
    if acceleration_desired > MaszynaLegacyDriverSpeed.NO_ACCELERATION and trainset.acceleration < acceleration_desired:
        var margin:float = 0.0 if velocity_desired == CRAWL_VELOCITY else MaszynaLegacyDriverSpeed.VELOCITY_MINUS
        # no speed table yet: nothing ahead holds it back (ActualProximityDist > fMaxProximityDist)
        if directional_speed < velocity_desired - margin:
            increase(vehicle, cab, trainset)
    if acceleration_desired <= MaszynaLegacyDriverSpeed.NO_ACCELERATION:
        MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
    elif directional_speed > velocity_desired or (acceleration_desired < 0.0 if trainset.gravity_acceleration < UPHILL_GRAVITY
            else trainset.acceleration > acceleration_desired + EXCESS_ACCELERATION):
        decrease(vehicle, cab)


## IncSpeed() (Driver.cpp:3406): a step of power; true when the controllers moved
static func increase(vehicle:RID, cab:int, trainset:MaszynaLegacyDriverTrainset) -> bool:
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(vehicle)
    match int(state.get("engine_type", VehicleEngine.NONE)):
        VehicleEngine.DIESEL_ELECTRIC:
            # Driver.cpp:3563-3575: not with the overload relay or the pressure switch tripped; past
            # the first position only once the line contactors closed
            if state.get("fuse_active", false) or state.get("pressure_switch_tripped", false):
                return false
            if not (state.get("main_no_power_pos", false) or state.get("line_contactor_closed", false)):
                return false
            if not trainset.ready:
                return false
            return _step(vehicle, cab, MASTER_CONTROLLER, &"increase", "controller_main_position") \
                    or _step(vehicle, cab, SECOND_CONTROLLER, &"increase", "controller_second_position")
    return false


## DecSpeed() (Driver.cpp:3690): power off - the second controller to zero first, then the master
## controller down by a few positions; true when the controllers moved
static func decrease(vehicle:RID, cab:int) -> bool:
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(vehicle)
    match int(state.get("engine_type", VehicleEngine.NONE)):
        VehicleEngine.DIESEL_ELECTRIC:
            var second:int = int(state.get("controller_second_position", 0))
            if second > 0:
                for _step:int in second:
                    CabinSystem.act(vehicle, cab, SECOND_CONTROLLER, &"decrease")
                return true
            # DecMainCtrl(min(MainCtrlPowerPos(), 2 + MainCtrlPowerPos() / 2)), Driver.cpp:3708
            var main:int = int(state.get("controller_main_position", 0))
            var steps:int = mini(main, 2 + floori(main / 2.0))
            for _step:int in steps:
                CabinSystem.act(vehicle, cab, MASTER_CONTROLLER, &"decrease")
            return steps > 0
    return false


## A step of a controller; true when its position moved
static func _step(vehicle:RID, cab:int, control:StringName, action:StringName, state_key:String) -> bool:
    var before:int = int(CabinSystem.vehicle_state_value(vehicle, state_key, 0))
    CabinSystem.act(vehicle, cab, control, action)
    return not int(CabinSystem.vehicle_state_value(vehicle, state_key, 0)) == before
