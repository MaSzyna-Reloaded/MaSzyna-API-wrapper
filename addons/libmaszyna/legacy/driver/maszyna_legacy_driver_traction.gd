@tool
extends RefCounted
class_name MaszynaLegacyDriverTraction

## The original driver's tractive force (TController::control_tractive_force(),
## increase_tractive_force(), control_handles(), Driver.cpp:7996-8063, 6438-6482): power added
## while the trainset accelerates less than wanted and runs slower than wanted, taken off when it
## runs too fast or accelerates too much. Every change is a step of the cab's controllers, as a
## player makes it (CabinSystem.act()).
##
## How a step of power is made depends on the engine of the driver's vehicle (IncSpeed(),
## DecSpeed(), SpeedSet(), Driver.cpp:3406-3827, 3846-4010): one subclass per engine type,
## chosen by create(). This base class is the vehicle without an engine of its own to drive.
##
## Not ported: the doors closed and the departure signal switched off before adding power, the
## no-current sections of the track (fOverhead2, iOverheadZero) - see TODO.md, "Drivers".

const MASTER_CONTROLLER:StringName = MaszynaLegacyDriverHints.MASTER_CONTROLLER
const SECOND_CONTROLLER:StringName = MaszynaLegacyDriverHints.SECOND_CONTROLLER
const MOTOR_OVERLOAD_RESET:StringName = &"fuse_bt"
## A limit of exactly this [km/h] is driven up to without the margin - under it a train would never
## move off (Driver.cpp:8008-8010)
const CRAWL_VELOCITY:float = 1.0
## Within the distance kept to the next speed, power is added only this much [km/h] under it
## (Driver.cpp:8016)
const NEXT_VELOCITY_MARGIN:float = 1.0
## Going uphill harder than this [m/s2], power is taken off only when the driver wants to slow down
## (Driver.cpp:8036)
const UPHILL_GRAVITY:float = -0.01
## The acceleration over the one wanted that takes power off on the flat (Driver.cpp:8037)
const EXCESS_ACCELERATION:float = 10.05
## With the cruise control on and a target over SPEED_CONTROL_FROM [km/h], power comes off only
## SPEED_CONTROL_MARGIN over it (Driver.cpp:8027)
const SPEED_CONTROL_MARGIN:float = 3.0
const SPEED_CONTROL_FROM:float = 5.0
## The cruise control follows the next speed within this distance [m] at least (Driver.cpp:3605)
const SPEED_CONTROL_PROXIMITY:float = 50.0
## A target speed over this [km/h] is one the cruise control is set to (Driver.cpp:3613)
const SPEED_CONTROL_TARGET_FROM:float = 0.1
## fVoltage: the line's voltage the driver reckons with is the mean of the last one and the one read
## now; under the collector's lowest the driver waits WAIT_MIN plus up to WAIT_SPREAD [s]
## (Driver.cpp:6232-6240)
const VOLTAGE_SMOOTHING:float = 0.5
const LOW_VOLTAGE_WAIT_MIN:float = 2.0
const LOW_VOLTAGE_WAIT_SPREAD:float = 10.0
## Pressing the buffers to uncouple, power is added while the force is under this [N]
## (bufferscompress, driverhints.cpp:584)
const PRESSING_FORCE:float = 50000.0
## The sand used below this share of the high threshold of the motor overload relay
## (control_wheelslip(), Driver.cpp:6209)
const SANDING_CURRENT_SHARE:float = 0.75

## The positions of an EIM controller of a Traxx (EIMCtrlType 1) and of an Elf (2): driving, and
## the neutral one past which it brakes (Driver.cpp:3767-3780, 3801-3818)
const TRAXX_DRIVING_POSITION:int = 6
const TRAXX_NEUTRAL_POSITION:int = 4
const ELF_DRIVING_POSITION:int = 4
const ELF_NEUTRAL_POSITION:int = 2

## A universal controller's position takes power off faster than this (UniCtrlList[].SpeedDown,
## Driver.cpp:4152, 4232)
const UNIVERSAL_DECREASING_SPEED:float = 0.01

## Series mode is kept under this share of the collector's range of voltage, of a heavy goods train
## the lower (Driver.cpp:3456-3461, 6450-6454)
const SERIES_VOLTAGE_SHARE:float = 0.40
const HEAVY_SERIES_VOLTAGE_SHARE:float = 0.35

## The engine type the subclass drives (create())
var engine_type:RailVehicleEngine.EngineType = RailVehicleEngine.NONE
## fActionTime [s]: below zero the driver waits before it adds power; counts up with every update
var action_time:float = 0.0
## fVoltage [V]
var voltage:float = 0.0
## Need_TryAgain: power asked for without current coming - the controllers go back to zero on the
## next update (Driver.cpp:3553-3559, 7984-7993)
var retry:bool = false


## What one decision about the power works with: the driver's vehicle and cab, the engine its
## controls drive (MaszynaLegacyDriverTrainset.controlling) and what the driver made of its
## situation on this update
class Situation:
    var vehicle:RID
    var cab:int
    var controlling:RID
    var order:int
    var speed:MaszynaLegacyDriverSpeed
    var trainset:MaszynaLegacyDriverTrainset
    var route:MaszynaLegacyDriverRoute
    var braking:MaszynaLegacyDriverBraking
    ## DirectionalVel() [km/h]
    var directional_speed:float
    ## movePress: it presses the buffers to uncouple
    var pressing:bool
    ## moveConnect under a Connect order: it couples up
    var coupling:bool


## The traction of the driver's vehicle's engine (the switch of IncSpeed(), Driver.cpp:3409)
static func create(type:RailVehicleEngine.EngineType) -> MaszynaLegacyDriverTraction:
    var traction:MaszynaLegacyDriverTraction
    match type:
        RailVehicleEngine.ELECTRIC_SERIES_MOTOR:
            traction = MaszynaLegacyDriverSeriesMotorTraction.new()
        RailVehicleEngine.DIESEL_ELECTRIC:
            traction = MaszynaLegacyDriverDieselElectricTraction.new()
        RailVehicleEngine.ELECTRIC_INDUCTION_MOTOR:
            traction = MaszynaLegacyDriverInductionMotorTraction.new()
        RailVehicleEngine.DIESEL:
            traction = MaszynaLegacyDriverDieselTraction.new()
        RailVehicleEngine.NONE:
            traction = MaszynaLegacyDriverControlCarTraction.new()
        _:
            traction = MaszynaLegacyDriverTraction.new()
    traction.engine_type = type
    return traction


## The driver waits `seconds` before it adds power (fActionTime < 0)
func hold(seconds:float) -> void:
    action_time = -seconds


## What the driver reads of its engine on every update (UpdateSituation(), Driver.cpp:5984,
## 6232-6240): the time it waits runs on, and the line's voltage
func read(situation:Situation, elapsed:float) -> void:
    action_time += elapsed
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(situation.controlling)
    voltage = VOLTAGE_SMOOTHING * (voltage + float(state.get("current_collector/voltage", 0.0)))
    if voltage < float(state.get("current_collector/min_main_switch_voltage", 0.0)) \
            and action_time >= MaszynaLegacyAIDriver.PREPARE_TIME:
        action_time = -LOW_VOLTAGE_WAIT_MIN - randf() * LOW_VOLTAGE_WAIT_SPREAD


## The relays and the controllers before any power (control_tractive_and_braking_force(),
## control_relays(), control_motor_connectors(), control_wheelslip(), Driver.cpp:7925-7952,
## 7967-7993, 6200-6217); false when the power and the brakes are to be left alone
func prepare(situation:Situation) -> bool:
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(situation.controlling)
    if action_time >= 0.0:
        if situation.trainset.motor_overload_relay_open:
            zero(situation)
            # tractionnmotoroverloadreset: a press of the relay's reset button
            CabinSystem.act(situation.vehicle, situation.cab, MOTOR_OVERLOAD_RESET, &"hold")
            CabinSystem.act(situation.vehicle, situation.cab, MOTOR_OVERLOAD_RESET, &"release")
        if not state.get("relay_ground", true):
            zero(situation)
            RailVehicleServer.vehicle_send_command(situation.vehicle, "ground_relay_reset")
    if retry:
        zero(situation)
        retry = false
    # after a Radio-Stop the power only comes off, nothing else is touched
    if CabinSystem.vehicle_state_value(situation.vehicle, "radio_stop_active", false) \
            and float(CabinSystem.vehicle_state_value(situation.vehicle, "speed", 0.0)) > MaszynaLegacyDriverTrainset.NO_MOVEMENT_SPEED:
        zero(situation)
        return false
    var slipping:bool = state.get("slipping_wheels", false)
    var sanding:bool = state.get("sand_active", false)
    var engine:RailVehicleElectricEngine = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleElectricEngine
    var high_current:bool = engine != null \
            and absf(float(state.get("Im", 0.0))) > SANDING_CURRENT_SHARE * engine.circuit_imax_high
    if slipping or high_current:
        if not sanding:
            RailVehicleServer.vehicle_send_command(situation.controlling, "sand", true)
    elif sanding:
        RailVehicleServer.vehicle_send_command(situation.controlling, "sand", false)
    # slipping, the controls are left alone - the power off and the brakes eased first
    if slipping:
        decrease(situation)
        situation.braking.ease(situation)
        RailVehicleServer.vehicle_send_command(situation.controlling, "antislip")
        return false
    return true


## One decision of the driver about the power (control_tractive_force(), Driver.cpp:7996-8040)
func control(situation:Situation) -> void:
    var velocity:float = situation.directional_speed
    var speed:MaszynaLegacyDriverSpeed = situation.speed
    var velocity_desired:float = speed.velocity_desired
    var acceleration_desired:float = speed.acceleration_desired
    var speed_control:bool = CabinSystem.vehicle_state_value(situation.vehicle, "speed_control/active", false)
    var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
            situation.vehicle, VehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
    var full_power:bool = speed_control and control != null and velocity < control.full_power_velocity
    if acceleration_desired > MaszynaLegacyDriverSpeed.NO_ACCELERATION \
            and (situation.trainset.acceleration < acceleration_desired or full_power) and not situation.pressing:
        var margin:float = 0.0 if velocity_desired == CRAWL_VELOCITY else situation.route.velocity_minus
        # within the distance kept to the next speed, not over it
        # increase_tractive_force() (Driver.cpp:8043-8063): not while told to wait or with the train
        # stretched; the spring brake off first
        if velocity < velocity_desired - margin and (speed.proximity_distance > situation.route.max_proximity
                or velocity + NEXT_VELOCITY_MARGIN < speed.velocity_next) \
                and action_time >= 0.0 and not situation.trainset.coupler_stretched:
            if CabinSystem.vehicle_state_value(situation.vehicle, "spring_brake/active", false):
                RailVehicleServer.vehicle_send_command(situation.vehicle, "set_spring_brake_active", false)
            increase(situation)
    if not situation.pressing:
        var speed_margin:float = SPEED_CONTROL_MARGIN if speed_control and velocity_desired > SPEED_CONTROL_FROM else 0.0
        if acceleration_desired <= MaszynaLegacyDriverSpeed.NO_ACCELERATION:
            zero(situation)
        elif velocity > velocity_desired + speed_margin or situation.trainset.coupler_stretched \
                or (acceleration_desired < 0.0 if situation.trainset.gravity_acceleration < UPHILL_GRAVITY
                else situation.trainset.acceleration > acceleration_desired + EXCESS_ACCELERATION):
            decrease(situation)
    control_handles(situation)
    set_speed(situation)


## bufferscompress (driverhints.cpp:582-594): power against its own brakes, to press the buffers
func press(situation:Situation) -> void:
    if absf(float(RailVehicleServer.vehicle_dump_state(situation.controlling).get("Ft", 0.0))) < PRESSING_FORCE:
        increase(situation)


## IncSpeed(): a step of power; true when the controllers moved
func increase(_situation:Situation) -> bool:
    return false


## DecSpeed(): a step of power off (never braking); true when the controllers moved. `force`: at
## once, as at a cab's activation
func decrease(_situation:Situation, _force:bool = false) -> bool:
    return false


## ZeroSpeed() (Driver.cpp:3683): the power off altogether
func zero(situation:Situation, force:bool = false) -> void:
    while decrease(situation, force):
        pass


## control_handles() (Driver.cpp:6438-6482): the engine's own care of its controllers
func control_handles(_situation:Situation) -> void:
    pass


## SpeedSet() (Driver.cpp:3846-4010): the regulation made with every decision about the power
func set_speed(_situation:Situation) -> void:
    pass


## The engine's part of CheckTimeControllers() (Driver.cpp:4260-4322), on the driver's update before
## any decision: controllers held by time go back to their holding positions
func check_time_controllers(_situation:Situation) -> void:
    pass


## The engine's part of SetTimeControllers() (Driver.cpp:4047-4258), on the driver's update after
## its decisions: controllers held by time moved to where they work until the next update
func set_time_controllers(_situation:Situation) -> void:
    pass


## SpeedCntrl() (Driver.cpp:4011-4045): the cruise control set to `velocity` [km/h]; 0 turns it off
func set_cruise_control(situation:Situation, velocity:float) -> void:
    var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
    if control == null or not control.speed_control_enabled:
        return
    var second:int = controller_position(situation, "controller_second_position")
    var second_max:int = int(RailVehicleServer.vehicle_dump_config(situation.controlling).get("second_controller_position_max", 0))
    if engine_type == RailVehicleEngine.DIESEL:
        if velocity < SPEED_CONTROL_TARGET_FROM:
            set_second_controller(situation, 0)
            velocity = 0.0
        elif second < 1:
            set_second_controller(situation, 1)
        RailVehicleServer.vehicle_send_command(situation.vehicle, "speed_control_set", velocity)
    elif second_max == 1:
        set_second_controller(situation, 1)
        RailVehicleServer.vehicle_send_command(situation.vehicle, "speed_control_set", velocity)
    elif second_max > 1 and not control.impulse_lever:
        var velocity_max:float = float(RailVehicleServer.vehicle_dump_config(situation.controlling).get("max_speed", 0.0))
        set_second_controller(situation, 1 + int(second_max * ((velocity - 1.0) / velocity_max)))
    if control.power_step > 0.0 and controller_position(situation, "controller_second_position") > 0:
        while float(CabinSystem.vehicle_state_value(situation.vehicle, "speed_control/desired_power", 0.0)) < control.max_power:
            RailVehicleServer.vehicle_send_command(situation.vehicle, "speed_control_power_increase")


## The cruise control's speed on an increase (Driver.cpp:3601-3617): the speed wanted, or the next
## one close to it, set when the unit takes it, else the unit off
func cruise(situation:Situation) -> void:
    var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
    if control == null or not control.speed_control_enabled \
            or not RailVehicleServer.vehicle_dump_state(situation.controlling).get("main_switch_enabled", false):
        return
    var speed:MaszynaLegacyDriverSpeed = situation.speed
    var velocity:float = speed.velocity_desired \
            if speed.proximity_distance > maxf(SPEED_CONTROL_PROXIMITY, situation.route.max_proximity) or situation.coupling \
            else MaszynaLegacyDriverSpeed.min_speed(speed.velocity_desired, speed.velocity_next)
    var over:float = velocity - control.min_velocity
    if velocity >= control.min_velocity and over == snappedf(over, control.velocity_step):
        set_cruise_control(situation, velocity)
    elif velocity > SPEED_CONTROL_TARGET_FROM:
        set_cruise_control(situation, 0.0)
        set_second_controller(situation, 0)


## A controller's position of the engine the controls drive (mvControlling->MainCtrlPos) - the
## cab's controllers act on it (CabinState.CONTROLLED_COMMANDS)
static func controller_position(situation:Situation, key:String) -> int:
    return int(CabinSystem.vehicle_state_value(situation.controlling, key, 0))


## A step of a controller; true when its position moved
static func step(situation:Situation, control:StringName, action:StringName, key:String) -> bool:
    var before:int = controller_position(situation, key)
    CabinSystem.act(situation.vehicle, situation.cab, control, action)
    return not controller_position(situation, key) == before


## A step of the master controller up (+1) or down (-1); true when it moved
static func step_main(situation:Situation, direction:int) -> bool:
    return step(situation, MaszynaLegacyDriverHints.master_controller(situation.vehicle, situation.cab),
            &"increase" if direction > 0 else &"decrease", "controller_main_position")


static func step_second(situation:Situation, direction:int) -> bool:
    return step(situation, SECOND_CONTROLLER, &"increase" if direction > 0 else &"decrease",
            "controller_second_position")


## The master controller stepped to `position`, as far as it goes; true when it moved
static func set_main_controller(situation:Situation, position:int) -> bool:
    var start:int = controller_position(situation, "controller_main_position")
    var current:int = start
    while not current == position and step_main(situation, signi(position - current)):
        current = controller_position(situation, "controller_main_position")
    return not current == start


## The second controller stepped to `position`, as far as it goes (DecScndCtrl(2) to 0); true when
## it moved
static func set_second_controller(situation:Situation, position:int) -> bool:
    var start:int = controller_position(situation, "controller_second_position")
    var current:int = start
    while not current == position and step_second(situation, signi(position - current)):
        current = controller_position(situation, "controller_second_position")
    return not current == start


## EIMCtrlType of the engine the controls drive (Cntrl. EIMCtrlType)
static func eim_control_type(situation:Situation) -> RailVehicleEngine.EimControlType:
    var engine:RailVehicleEngine = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleEngine
    return engine.cntrl_eim_control_type if engine else RailVehicleEngine.EIM_CONTROL_TYPE_0


## IncSpeedEIM() (Driver.cpp:3761-3789): power by the EIM controller's kind - a step, or straight to
## its driving position (Traxx 6, Elf 4); true when it moved
func increase_eim(situation:Situation) -> bool:
    var main:int = controller_position(situation, "controller_main_position")
    match eim_control_type(situation):
        RailVehicleEngine.EIM_CONTROL_TYPE_0:
            return step_main(situation, 1)
        RailVehicleEngine.EIM_CONTROL_TYPE_1:
            if main < TRAXX_DRIVING_POSITION:
                return set_main_controller(situation, TRAXX_DRIVING_POSITION)
        RailVehicleEngine.EIM_CONTROL_TYPE_2:
            if main < ELF_DRIVING_POSITION:
                return set_main_controller(situation, ELF_DRIVING_POSITION)
    return false


## DecSpeedEIM() (Driver.cpp:3791-3826): power off by the EIM controller's kind - a step, to its
## neutral position, or the cruise control's power down while the driver still wants to go
func decrease_eim(situation:Situation) -> bool:
    var main:int = controller_position(situation, "controller_main_position")
    match eim_control_type(situation):
        RailVehicleEngine.EIM_CONTROL_TYPE_0:
            return step_main(situation, -1)
        RailVehicleEngine.EIM_CONTROL_TYPE_1:
            if main > TRAXX_NEUTRAL_POSITION:
                return set_main_controller(situation, TRAXX_NEUTRAL_POSITION)
        RailVehicleEngine.EIM_CONTROL_TYPE_2:
            var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
                    situation.controlling, VehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
            var state:Dictionary = RailVehicleServer.vehicle_dump_state(situation.controlling)
            if situation.speed.acceleration_desired > 0.0 and control and state.get("speed_control/active", false) \
                    and control.power_step > 0.0 and float(state.get("speed_control/desired_power", 0.0)) > control.min_power:
                RailVehicleServer.vehicle_send_command(situation.vehicle, "speed_control_power_decrease")
            elif main > ELF_NEUTRAL_POSITION:
                return set_main_controller(situation, ELF_NEUTRAL_POSITION)
    return false


## The voltage under which a series motor's controls keep to series mode (Driver.cpp:3456-3461)
func series_voltage(situation:Situation) -> float:
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(situation.controlling)
    return lerpf(float(state.get("current_collector/min_main_switch_voltage", 0.0)),
            float(state.get("current_collector/max_voltage", 0.0)),
            HEAVY_SERIES_VOLTAGE_SHARE if situation.braking.heavy_cargo else SERIES_VOLTAGE_SHARE)


## control_handles() of a series motor (Driver.cpp:6441-6464) - of the driver's own, or of the one
## an EMU's control car drives
func control_series_motor_handles(situation:Situation) -> void:
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(situation.controlling)
    # the line contactors dropped out: back to zero
    if not state.get("line_contactor_closed", false) and not state.get("controller_main_delayed", false) \
            and main_powercontroller_position(situation) > 1:
        zero(situation)
    # a heavily burdened substation: series mode, to lessen the load
    if voltage <= series_voltage(situation):
        set_series_mode(situation)
    if not situation.trainset.ready and main_powercontroller_position(situation) > 1:
        zero(situation)


## mastercontrollersetseriesmode (driverhints.cpp:503-521): off the parallel positions
func set_series_mode(situation:Situation) -> void:
    var engine:RailVehicleElectricSeriesEngine = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleElectricSeriesEngine
    if engine == null:
        return
    var relays:Array = engine.relay_list
    if controller_position(situation, "controller_main_position") >= relays.size() \
            or (relays[controller_position(situation, "controller_main_position")] as RailVehicleRelayListItem).branch_count <= 1:
        return
    set_second_controller(situation, 0)
    while (relays[controller_position(situation, "controller_main_position")] as RailVehicleRelayListItem).branch_count > 1 \
            and step_main(situation, -1):
        pass


## MainCtrlPowerPos(): the master controller's position past the last without power
static func main_powercontroller_position(situation:Situation) -> int:
    return controller_position(situation, "controller_main_position") \
            - controller_position(situation, "controller_main_no_power_position")
