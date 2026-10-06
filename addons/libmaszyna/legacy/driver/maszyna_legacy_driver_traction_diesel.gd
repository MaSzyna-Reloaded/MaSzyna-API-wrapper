@tool
extends MaszynaLegacyDriverTraction
class_name MaszynaLegacyDriverDieselTraction

## The traction of a diesel engine with a mechanical or hydraulic transmission (SN61, SA134, SA1xx;
## IncSpeed(), DecSpeed(), SpeedSet(), SetTimeControllers() 5.1-5.2, Driver.cpp:3629-3680,
## 3724-3757, 3964-4001, 4140-4214):
## * a gearbox shifted by hand: the master controller to idle past the clutch's full engagement,
##   up a gear past three quarters of the gear's top speed, down under its lowest;
## * an analog controller (a vehicle faster than ANALOG_CONTROLLER_VELOCITY): the position from
##   the difference to the speed wanted;
## * a DMU's universal controller (EIMCtrlType 3): the share of power wanted (DizelPercentage),
##   reached by holding the controller at the position that adds power or the one that takes it.

## A diesel faster than this [km/h] has an analog controller (Driver.cpp:4198)
const ANALOG_CONTROLLER_VELOCITY:float = 30.0
## The analog controller's position: the speed difference [km/h] over a factor of this times the
## top speed over the top and current speeds (Driver.cpp:4203)
const ANALOG_FACTOR:float = 5.0
## A gear up past this share of its top speed (Driver.cpp:3975)
const GEAR_UP_SHARE:float = 0.75
## DizelPercentage: all the power, a start's 1 % (Driver.cpp:3636-3639)
const FULL_POWER:int = 100
const STARTING_POWER:int = 1
## The last position kept at a start under the cruise control (Driver.cpp:4146)
const HOLD_POWER:int = 101
## The DMU's power: the lowest speed [km/h] it is shared out above is the converter's lock-up or a
## sixth of the top speed; the factor of the difference to the speed wanted; under half the top
## speed and within LOW_VELOCITY_MARGIN of it at most LOW_VELOCITY_SHARE (Driver.cpp:4140-4163)
const MIN_VELOCITY_SHARE:float = 1.0 / 6.0
const DIFFERENCE_FACTOR:float = 10.0
const DIFFERENCE_SPEED_WEIGHT:float = 3.0
const LOW_VELOCITY_SHARE:float = 0.75
const LOW_VELOCITY_LIMIT:float = 0.5
const LOW_VELOCITY_MARGIN:float = 10.0
const CREEPING_SHARE:float = 0.99
## eimic_real to percent, and the gaps that take the fast position down (Driver.cpp:4169, 4180-4185)
const PERCENT_SCALE:float = 100.4
const FAST_DECREASE_GAP:int = 50
const FAST_DECREASE_FROM:int = 10

## DizelPercentage: the share of power the driver wants [%], and DizelPercentage_Speed, the share
## it holds the controller for at the speed it runs
var power_percentage:int = 0
var power_percentage_speed:int = 0


func increase(situation:MaszynaLegacyDriverTraction.Situation) -> bool:
    cruise(situation)
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    if engine == null:
        return false
    var velocity:float = VehicleServer.vehicle_get_speed(situation.controlling)
    var moved:bool = false
    if not eim_control_type(situation) == RailVehicleEngine.EIM_CONTROL_TYPE_0:
        if situation.trainset.ready:
            var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
                    situation.controlling, RailVehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
            var cruising:bool = control != null and control.speed_control_enabled \
                    and second_controller_position(situation) > 0
            power_percentage = FULL_POWER if velocity > engine.clutch_min_velocity_full_engage or cruising else STARTING_POWER
        return false
    if situation.trainset.ready:
        # past the clutch's full engagement the controller goes up, and off the idle positions
        # always (RList[].Mn, the throttle table's clutch)
        if velocity > engine.clutch_min_velocity_full_engage and not _clutch(situation, engine) == RailVehicleThrottlePositionItem.CLUTCH_BEHAVIOR_NONE:
            moved = step_main(situation, 1)
        if _clutch(situation, engine) == RailVehicleThrottlePositionItem.CLUTCH_BEHAVIOR_NONE:
            moved = step_main(situation, 1)
    # TODO in the original: to move to a better place (Driver.cpp:3669-3677). The idle position
    # goes first, as PrepareEngine() cues it (Driver.cpp:2840-2843): at a position without fuel
    # (R = 0) the engine does not start, and a hint list with the line breaker first leads the
    # player to close it there
    if not engine.get_main_switch_enabled():
        set_cruise_control(situation, 0.0)
        set_second_controller(situation, 0)
        MaszynaLegacyDriverHints.cue(situation, MaszynaLegacyDriverHints.Hint.MASTER_CONTROLLER_SET_IDLE)
        MaszynaLegacyDriverHints.cue(situation, MaszynaLegacyDriverHints.Hint.LINE_BREAKER_CLOSE)
        MaszynaLegacyDriverHints.cue(situation, MaszynaLegacyDriverHints.Hint.CONVERTER_ON)
        MaszynaLegacyDriverHints.cue(situation, MaszynaLegacyDriverHints.Hint.COMPRESSOR_ON)
    return moved


func decrease(situation:MaszynaLegacyDriverTraction.Situation, force:bool = false) -> bool:
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    if engine == null:
        return false
    if not eim_control_type(situation) == RailVehicleEngine.EIM_CONTROL_TYPE_0:
        power_percentage = 0
        var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
                situation.controlling, RailVehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
        var velocity_desired:float = situation.speed.velocity_desired
        if force or (control and velocity_desired > SPEED_CONTROL_TARGET_FROM and control.min_velocity > velocity_desired):
            set_cruise_control(situation, 0.0)
            set_second_controller(situation, 0)
        return false
    var moved:bool = false
    if VehicleServer.vehicle_get_speed(situation.controlling) > engine.clutch_min_velocity_full_engage:
        if not _clutch(situation, engine) == RailVehicleThrottlePositionItem.CLUTCH_BEHAVIOR_NONE:
            moved = step_main(situation, -1)
    else:
        while not _clutch(situation, engine) == RailVehicleThrottlePositionItem.CLUTCH_BEHAVIOR_NONE and main_powercontroller_position(situation) > 1 and step_main(situation, -1):
            moved = true
    if force:
        # DecMainCtrl(2) of a diesel: to zero (Mover.cpp:2638-2643)
        set_main_controller(situation, 0)
        set_cruise_control(situation, 0.0)
        set_second_controller(situation, 0)
        moved = false
    return moved


func set_speed(situation:MaszynaLegacyDriverTraction.Situation) -> void:
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    if engine == null:
        return
    # SpeedSet() (Driver.cpp:3964-4001): the gearbox shifted by hand - up a gear past three quarters
    # of the gear's top speed, down under its lowest, the power off for the shift and neutral gears
    # passed. The shunting mode's extra gear (AnPos) is not read.
    var gears:Array = engine.motor_param_table
    var gear:int = second_controller_position(situation)
    if gear >= gears.size():
        return
    var parameters:RailVehicleMotorParameter = gears[gear]
    if parameters.auto_switch:
        return
    var second_max:int = mini(gears.size() - 1, second_position_count(situation))
    var velocity:float = VehicleServer.vehicle_get_speed(situation.controlling)
    if velocity > GEAR_UP_SHARE * parameters.voltage_constant_multiplier:
        if gear < second_max:
            set_main_controller(situation, 0)
            while step_second(situation, 1) and _neutral(gears, second_controller_position(situation)):
                pass
    elif velocity < parameters.voltage_constant and gear > 1:
        set_main_controller(situation, 0)
        while second_controller_position(situation) > 1 and step_second(situation, -1) \
                and _neutral(gears, second_controller_position(situation)):
            pass


func set_time_controllers(situation:MaszynaLegacyDriverTraction.Situation) -> void:
    var engine:RailVehicleDieselEngine = VehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleDieselEngine
    if engine == null:
        return
    if eim_control_type(situation) == RailVehicleEngine.EIM_CONTROL_TYPE_3:
        # 5.1 (Driver.cpp:4140-4193): a DMU's universal controller, held at the position that adds
        # power until the share wanted is reached, or at the one that takes it
        var velocity:float = VehicleServer.vehicle_get_speed(situation.controlling)
        var velocity_max:float = VehicleServer.vehicle_get_controller(situation.controlling).max_velocity
        var velocity_desired:float = situation.speed.velocity_desired
        var control:RailVehicleSpeedControl = RailVehicleServer.vehicle_component_get(
                situation.controlling, RailVehicleComponentType.COMPONENT_SPEED_CONTROL) as RailVehicleSpeedControl
        var wanted:int = power_percentage
        var min_velocity:float = minf(engine.torque_converter_lockup_speed, velocity_max * MIN_VELOCITY_SHARE)
        if control and control.speed_control_enabled and second_controller_position(situation) > 0:
            # keep the last position to start
            if velocity < 1.0 + control.start_velocity and power_percentage > 0:
                wanted = HOLD_POWER
        elif velocity_desired > min_velocity:
            var factor:float = DIFFERENCE_FACTOR * velocity_max / (velocity_max + DIFFERENCE_SPEED_WEIGHT * velocity)
            var share:float = clampf((velocity_desired - velocity) / factor if velocity_desired > velocity else 0.0, 0.0, 1.0)
            if velocity_desired < LOW_VELOCITY_LIMIT * velocity_max and velocity_desired - velocity < LOW_VELOCITY_MARGIN:
                share = minf(share, LOW_VELOCITY_SHARE)
            wanted = roundi(share * power_percentage)
        else:
            # slow acceleration while shunting at the lowest speed
            wanted = mini(power_percentage, 1 if velocity < CREEPING_SHARE * velocity_desired else 0)
        power_percentage_speed = wanted
        var actual:int = int(PERCENT_SCALE * engine.get_eimic_real())
        var controller:RailVehicleUniversalController = RailVehicleServer.vehicle_component_get(
                situation.controlling, RailVehicleComponentType.COMPONENT_UNIVERSAL_CONTROLLER) as RailVehicleUniversalController
        if controller == null or wanted == actual:
            return
        var positions:Array = controller.positions
        var increase_position:int = mini(positions.size() - 1, main_position_count(situation))
        var decrease_position:int = 0
        for index:int in range(increase_position, -1, -1):
            var item:RailVehicleUniversalControllerListItem = positions[index]
            if item.target_value <= 0.0 and item.decrease_speed > UNIVERSAL_DECREASING_SPEED:
                decrease_position = index
                break
        # one position earlier is the fast decrease
        if decrease_position > 0 and (actual - wanted > FAST_DECREASE_GAP or (wanted == 0 and actual > FAST_DECREASE_FROM)):
            decrease_position -= 1
        set_main_controller(situation, increase_position if wanted > actual else decrease_position)
    elif VehicleServer.vehicle_get_controller(situation.controlling).max_velocity > ANALOG_CONTROLLER_VELOCITY:
        # 5.2 (Driver.cpp:4196-4214): an analog controller, its position from the speed still to
        # gain, never lower than the first position with the clutch in
        var positions:Array = engine.throttle_table_positions
        var max_position:int = mini(positions.size() - 1, main_position_count(situation))
        var min_position:int = max_position
        var index:int = max_position
        while index > 1 and not (positions[index] as RailVehicleThrottlePositionItem).clutch_behavior == RailVehicleThrottlePositionItem.CLUTCH_BEHAVIOR_NONE:
            min_position = index
            index -= 1
        var main:int = main_controller_position(situation)
        var velocity_desired:float = situation.speed.velocity_desired
        if not (max_position > min_position and main > 0 and situation.speed.acceleration_desired > 0.0):
            return
        var velocity:float = VehicleServer.vehicle_get_speed(situation.controlling)
        var velocity_max:float = VehicleServer.vehicle_get_controller(situation.controlling).max_velocity
        var factor:float = ANALOG_FACTOR * velocity_max / (velocity_max + velocity)
        var wanted:int = clampi(min_position + int((max_position - min_position)
                * ((velocity_desired - velocity) / factor if velocity_desired > velocity else 0.0)), min_position, max_position)
        var wheels:RailVehicleWheels = VehicleServer.vehicle_component_get(
                situation.controlling, VehicleComponentType.COMPONENT_WHEELS) as RailVehicleWheels
        if wheels and wheels.get_slipping():
            return
        while main_controller_position(situation) > wanted and step_main(situation, -1):
            pass
        if velocity > engine.clutch_min_velocity_full_engage:
            while main_controller_position(situation) < wanted and step_main(situation, 1):
                pass


## RList[MainCtrlPos].Mn of a diesel: the clutch at the master controller's position (0 idle)
func _clutch(situation:MaszynaLegacyDriverTraction.Situation,
        engine:RailVehicleDieselEngine) -> RailVehicleThrottlePositionItem.ClutchBehavior:
    var positions:Array = engine.throttle_table_positions
    var main:int = main_controller_position(situation)
    return (positions[main] as RailVehicleThrottlePositionItem).clutch_behavior if main < positions.size() \
            else RailVehicleThrottlePositionItem.CLUTCH_BEHAVIOR_NONE


## A neutral gear, to be passed (MotorParam[].mIsat == 0)
static func _neutral(gears:Array, gear:int) -> bool:
    return gear < gears.size() and (gears[gear] as RailVehicleMotorParameter).saturation_current_multiplier == 0.0


## CheckTimeControllers() 5.1 (Driver.cpp:4303-4321): a DMU's universal controller not braking goes
## back to its position that holds the power, from either side of it, once the share is reached
func check_time_controllers(situation:MaszynaLegacyDriverTraction.Situation) -> void:
    if not eim_control_type(situation) == RailVehicleEngine.EIM_CONTROL_TYPE_3 or situation.braking.position >= MaszynaLegacyDriverBraking.BRAKING_FROM:
        return
    var controller:RailVehicleUniversalController = RailVehicleServer.vehicle_component_get(
            situation.controlling, RailVehicleComponentType.COMPONENT_UNIVERSAL_CONTROLLER) as RailVehicleUniversalController
    if controller == null:
        return
    var positions:Array = controller.positions
    var main_max:int = mini(positions.size() - 1, main_position_count(situation))
    # the last but one should hold - the original's working hypothesis
    var neutral:int = main_max - 1
    for index:int in range(main_max, -1, -1):
        var item:RailVehicleUniversalControllerListItem = positions[index]
        if item.target_value <= 0.0 and item.decrease_speed < UNIVERSAL_DECREASING_SPEED:
            neutral = index
            break
    var engine:RailVehicleEngine = VehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as RailVehicleEngine
    var actual:int = int(PERCENT_SCALE * (engine.get_eimic_real() if engine else 0.0))
    var main:int = main_controller_position(situation)
    if (actual >= power_percentage_speed and main > neutral) or (actual <= power_percentage_speed and main < neutral):
        set_main_controller(situation, neutral)
