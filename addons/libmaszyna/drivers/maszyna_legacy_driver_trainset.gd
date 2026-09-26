@tool
extends RefCounted
class_name MaszynaLegacyDriverTrainset

## What the original's driver reads of its trainset on every update (TController::UpdateSituation(),
## Driver.cpp:6033-6190): the vehicles from the front the way it drives, whether their brakes let
## it start (Ready, fReady, IsConsistBraked), the pull of the slope along the track (fAccGravity)
## and the acceleration of the whole trainset (AbsAccS). The original sums every vehicle with the
## sign of its orientation; here each vehicle's front is taken along the way the driver drives,
## which is what those signs mean.
##
## Not read yet: the stretched couplers, the doors, the light, the relays of the other vehicles
## under control; the individual release of an overcharged vehicle (Driver.cpp:6059-6078) - see
## TODO.md, "Drivers".

## Original engine: MOVER.h:87 g [m/s2]
const GRAVITY:float = 9.81
## A vehicle slower than this [km/h] is starting, and its brake must be released below
## RELEASED_BRAKE_PRESSURE; a moving one only must not brake harder than MOVING_BRAKE_FORCE
## (Driver.cpp:6053-6055)
const STARTING_SPEED:float = 1.0
const RELEASED_BRAKE_PRESSURE:float = 0.4
## [kN]
const MOVING_BRAKE_FORCE:float = 10.0
const NEWTONS_PER_KILONEWTON:float = 1000.0
## The train brake counts as applied below this pipe pressure [bar] (Driver.cpp:6038)
const BRAKED_PIPE_PRESSURE:float = 3.9
const BRAKED_PIPE_MARGIN:float = 0.1
## A train that would roll back uphill starts with its brakes not quite released (Driver.cpp:6168)
const ROLLING_BACK_GRAVITY:float = -0.05
const ROLLING_BACK_BRAKE_PRESSURE:float = 0.8
## A diesel engine counts as started past this share of its idle speed, a vehicle moving faster
## than MOVEMENT_SPEED [km/h] as running (EU07_AI_MOVEMENT, Driver.cpp:6185-6189)
const ENGINE_STARTED_RATIO:float = 0.8
const MOVEMENT_SPEED:float = 1.0
## A vehicle slower than this [km/h] stands (EU07_AI_NOMOVEMENT, Driver.h:25)
const NO_MOVEMENT_SPEED:float = 0.05
## The vehicle's couplers (end::front, end::rear)
const FRONT_END:int = 0
const REAR_END:int = 1

## From the front, the way the driver drives
var vehicles:Array[RID] = []
## fMass [kg]
var mass:float = 0.0
## Ready - no brake of the trainset holds it back
var ready:bool = false
## fReady - the highest brake cylinder pressure of the trainset [bar]
var brake_pressure_max:float = 0.0
## IsConsistBraked - the train brake is applied
var braked:bool = false
## fAccGravity - the slope's pull along the way the driver drives [m/s2]
var gravity_acceleration:float = 0.0
## AbsAccS - the trainset's acceleration along the way the driver drives [m/s2]
var acceleration:float = 0.0


## Reads the trainset of the vehicle the driver drives, `direction` +1 or -1 along the vehicle
func update(vehicle:RID, direction:int, diesel_driven:bool) -> void:
    vehicles = RailVehicleServer.vehicle_get_coupled(
            vehicle, FRONT_END if direction >= 0 else REAR_END, VehicleController.COUPLING_ELEMENT_COUPLER)
    var driving:Vector3 = -RailVehicleServer.vehicle_get_transform(vehicle).basis.z * direction
    var driven:Dictionary = RailVehicleServer.vehicle_dump_state(vehicle)
    ready = true
    brake_pressure_max = 0.0
    braked = float(driven.get("pipe_pressure", 0.0)) < BRAKED_PIPE_PRESSURE + BRAKED_PIPE_MARGIN
    mass = 0.0
    var gravity_force:float = 0.0
    var momentum_change:float = 0.0
    var moving:bool = float(driven.get("speed", 0.0)) > NO_MOVEMENT_SPEED
    for other:RID in vehicles:
        var state:Dictionary = RailVehicleServer.vehicle_dump_state(other)
        var brake_pressure:float = maxf(0.0, float(state.get("brake_air_pressure", 0.0)))
        if ready and (state.get("brake_is_holding", false) or state.get("brake_is_braking", false)
                or (brake_pressure > RELEASED_BRAKE_PRESSURE if float(state.get("speed", 0.0)) < STARTING_SPEED
                else float(state.get("brake_force", 0.0)) / NEWTONS_PER_KILONEWTON > MOVING_BRAKE_FORCE)):
            ready = false
        brake_pressure_max = maxf(brake_pressure, brake_pressure_max)
        var vehicle_mass:float = float(state.get("mass_total", 0.0))
        mass += vehicle_mass
        # the vehicle's front along the way the driver drives: its slope and its acceleration count
        # with that sign
        var front:Vector3 = -RailVehicleServer.vehicle_get_transform(other).basis.z
        var along:float = signf(front.dot(driving))
        gravity_force -= vehicle_mass * GRAVITY * front.y * along
        momentum_change += vehicle_mass * float(state.get("acceleration", 0.0)) * along
    gravity_acceleration = gravity_force / mass if mass > 0.0 else 0.0
    acceleration = (momentum_change / mass if mass > 0.0 else 0.0) if moving else gravity_acceleration
    if not ready and gravity_acceleration < ROLLING_BACK_GRAVITY and brake_pressure_max < ROLLING_BACK_BRAKE_PRESSURE:
        ready = true
    # a diesel is ready once every live engine of the trainset has started
    if not diesel_driven:
        return
    for other:RID in vehicles:
        if not ready:
            return
        var state:Dictionary = RailVehicleServer.vehicle_dump_state(other)
        var idle:float = float(RailVehicleServer.vehicle_dump_config(other).get("engine_idle_rpm_count", 0.0))
        ready = float(state.get("speed", 0.0)) > MOVEMENT_SPEED or not state.get("main_switch_enabled", false) \
                or float(state.get("engine_rpm_count", 0.0)) > ENGINE_STARTED_RATIO * idle
