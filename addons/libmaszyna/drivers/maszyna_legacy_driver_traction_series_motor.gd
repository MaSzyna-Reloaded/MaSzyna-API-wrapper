@tool
extends MaszynaLegacyDriverTraction
class_name MaszynaLegacyDriverSeriesMotorTraction

## The traction of an electric series motor (EU07, EP07, ET22...; IncSpeed(), DecSpeed(),
## control_handles(), Driver.cpp:3415-3561, 3702-3707, 6441-6464): the master controller up the
## resistors while the current allows, series to parallel, the field shunt on the last resistorless
## position; the high threshold of the motor overload relay for a heavy start; series mode when the
## line's voltage sags.

## Series mode is used up to these [km/h] of a passenger and goods train, 5 more on the field shunt
## and 10 less uphill; parallel mode's field shunt up to its own (Driver.cpp:3470-3485)
const SERIES_VELOCITY:float = 30.0
const GOODS_SERIES_VELOCITY:float = 40.0
const PARALLEL_VELOCITY:float = 50.0
const GOODS_PARALLEL_VELOCITY:float = 60.0
const SHUNTING_VELOCITY_BONUS:float = 5.0
const UPHILL_VELOCITY_PENALTY:float = 10.0
const SERIES_UPHILL_GRAVITY:float = -0.025
## A start is heavy enough for the high threshold under these [km/h], the threshold on or off, when
## the power over the trainset's weight along the slope makes less than this [m/s]
## (Driver.cpp:3432-3445)
const HIGH_THRESHOLD_VELOCITY:float = 30.0
const LOW_THRESHOLD_VELOCITY:float = 20.0
const HIGH_THRESHOLD_GRAVITY:float = 0.025
const HIGH_THRESHOLD_GRAVITY_FLAT:float = -0.01
const HIGH_THRESHOLD_SPEED:float = -2.8
## Enough force per engine [N] and acceleration [m/s2] to stay in series mode, of a passenger, a
## goods and a heavy goods train (Driver.cpp:3450-3451)
const SUFFICIENT_FORCE:float = 50000.0
const HEAVY_SUFFICIENT_FORCE:float = 75000.0
const SUFFICIENT_ACCELERATION:float = 0.09
const GOODS_SUFFICIENT_ACCELERATION:float = 0.06
const HEAVY_SUFFICIENT_ACCELERATION:float = 0.03
## Parallel mode and the field shunt want this much [V] per engine over series mode's voltage
## (Driver.cpp:3520, 3536-3541)
const POWER_MARGIN:float = 75.0
const HEAVY_POWER_MARGIN:float = 100.0
## A position with less resistance than this [ohm] is resistorless (Driver.cpp:3481)
const RESISTORLESS:float = 0.01
## Under this highest brake cylinder pressure [bar] the driver steps on at Imin, else at IminLo
## (Driver.cpp:3512)
const RELEASED_BRAKE_PRESSURE:float = 0.4
## Not a notch is safe below any speed (Vs = 99999, Driver.cpp:3504)
const ANY_VELOCITY:float = 99999.0


func increase(situation:MaszynaLegacyDriverTraction.Situation) -> bool:
    var state:Dictionary = RailVehicleServer.vehicle_dump_state(situation.controlling)
    var engine:VehicleElectricSeriesEngine = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as VehicleElectricSeriesEngine
    if engine == null:
        return false
    # an engine running off its battery reckons with the battery's voltage (Driver.cpp:3424-3426)
    if int(state.get("power_source", TrainController.POWER_SOURCE_NOT_DEFINED)) == TrainController.POWER_SOURCE_ACCUMULATOR:
        voltage = float(state.get("battery_voltage", 0.0))
    if situation.trainset.motor_overload_relay_open or state.get("pressure_switch_tripped", false):
        return false
    # past the first position only once the line contactors closed
    if not (state.get("main_no_power_pos", false) or state.get("line_contactor_closed", false)):
        return false
    if not (situation.trainset.ready or situation.pressing):
        return false
    var relays:Array = engine.relay_list
    var main:int = controller_position(situation, "controller_main_position")
    var second:int = controller_position(situation, "controller_second_position")
    var config:Dictionary = RailVehicleServer.vehicle_dump_config(situation.controlling)
    var main_max:int = int(config.get("main_controller_position_max", 0))
    var second_max:int = int(config.get("second_controller_position_max", 0))
    if relays.size() <= main_max:
        return false
    var imax:float = float(state.get("circuit_imax", 0.0))
    var current:float = absf(float(state.get("Im", 0.0)))
    var high_on:bool = engine.circuit_imax_high > engine.circuit_imax_low and imax > engine.circuit_imax_low
    var velocity:float = float(CabinSystem.vehicle_state_value(situation.vehicle, "speed", 0.0))
    var trainset:MaszynaLegacyDriverTrainset = situation.trainset
    var cargo:bool = situation.braking.cargo
    var heavy:bool = situation.braking.heavy_cargo
    var engines:int = trainset.controlled_engines
    var gravity:float = trainset.gravity_acceleration
    # ET42 uses these variables for another purpose (Driver.cpp:3430)
    var et42:bool = int(config.get("train_type", 0)) == TrainController.TRAIN_TYPE_ET42
    var engine_voltage:float = absf(float(state.get("engine_voltage", 0.0)))
    var slope:float = HIGH_THRESHOLD_GRAVITY_FLAT if gravity == HIGH_THRESHOLD_GRAVITY else gravity - HIGH_THRESHOLD_GRAVITY
    var use_high_threshold:bool = not et42 and engine.circuit_imax_high > engine.circuit_imax_low \
            and velocity < (HIGH_THRESHOLD_VELOCITY if high_on else LOW_THRESHOLD_VELOCITY) \
            and trainset.vehicles.size() - engines > 0 and trainset.mass > 0.0 \
            and imax * engine_voltage * engines / (trainset.mass * slope) < HIGH_THRESHOLD_SPEED
    var sufficient_force:bool = absf(float(state.get("Ft", 0.0))) * engines > (HEAVY_SUFFICIENT_FORCE if heavy else SUFFICIENT_FORCE)
    var sufficient_acceleration:bool = trainset.acceleration >= (HEAVY_SUFFICIENT_ACCELERATION if heavy
            else GOODS_SUFFICIENT_ACCELERATION if cargo else SUFFICIENT_ACCELERATION)
    var branches:int = (relays[main] as RelayListItem).branch_count
    var series_shunting:bool = second > 0 and branches == 1
    var parallel_shunting:bool = second > 0 and branches > 1
    var collector:bool = int(state.get("power_source", TrainController.POWER_SOURCE_NOT_DEFINED)) == TrainController.POWER_SOURCE_CURRENTCOLLECTOR
    var min_voltage:float = float(state.get("current_collector/min_main_switch_voltage", 0.0)) if collector else 0.0
    var max_voltage:float = float(state.get("current_collector/max_voltage", 0.0)) if collector else 0.0
    var series_mode_voltage:float = lerpf(min_voltage, max_voltage, HEAVY_SERIES_VOLTAGE_SHARE if heavy else SERIES_VOLTAGE_SHARE)
    var use_series:bool = imax > engine.circuit_imax_low or use_high_threshold or voltage < series_mode_voltage \
            or (sufficient_acceleration and sufficient_force
                and velocity <= (GOODS_SERIES_VELOCITY if cargo else SERIES_VELOCITY)
                    + (SHUNTING_VELOCITY_BONUS if series_shunting else 0.0)
                    - (UPHILL_VELOCITY_PENALTY if gravity < SERIES_UPHILL_GRAVITY else 0.0))
    # when not in series mode, the first parallel configuration until 50/60 km/h
    var parallel_early:bool = sufficient_acceleration and sufficient_force \
            and velocity <= (GOODS_PARALLEL_VELOCITY if cargo else PARALLEL_VELOCITY) \
                + (SHUNTING_VELOCITY_BONUS if parallel_shunting else 0.0)
    var use_field_shunt:bool = state.get("line_contactor_closed", false) \
            and (relays[main] as RelayListItem).resistance < RESISTORLESS \
            and (branches == 1 if use_series else (branches > 1 if parallel_early else main == main_max))
    if not et42:
        if use_high_threshold:
            if imax < engine.circuit_imax_high:
                # the high threshold needs the series mode (Driver.cpp:3490-3500)
                if branches > 1:
                    set_second_controller(situation, 0)
                    while not CabinSystem.vehicle_state_value(situation.vehicle, "main_no_power_pos", true) \
                            and (relays[controller_position(situation, "controller_main_position")] as RelayListItem).branch_count > 1 \
                            and step_main(situation, -1):
                        pass
                RailVehicleServer.vehicle_send_command(situation.controlling, "motor_overload_relay_threshold", true)
        elif high_on and current < engine.circuit_imax_low:
            RailVehicleServer.vehicle_send_command(situation.controlling, "motor_overload_relay_threshold", false)
    main = controller_position(situation, "controller_main_position")
    second = controller_position(situation, "controller_second_position")
    var safe_velocity:float = ANY_VELOCITY
    if (second < second_max) if use_field_shunt else (main < main_max):
        safe_velocity = engine.get_next_position_velocity(not use_field_shunt)
    var step_current:float = float(state.get("circuit_imin", 0.0)) \
            if trainset.brake_pressure_max < RELEASED_BRAKE_PRESSURE else float(engine.circuit_imin_low)
    if not (current < step_current or float(state.get("speed", 0.0)) > safe_velocity):
        return false
    var margin:float = (HEAVY_POWER_MARGIN if heavy else POWER_MARGIN) * engines
    if use_field_shunt:
        # the field shunt only with power to spare
        return step_second(situation, 1) if voltage - series_mode_voltage > margin else true
    set_second_controller(situation, 0)
    # don't draw too much power: keep from dropping into series mode entering the parallel one, and
    # from shutting down in the series one
    var next_branches:int = (relays[mini(main + 1, main_max)] as RelayListItem).branch_count
    var moved:bool = true
    if voltage - (min_voltage if next_branches == 1 else series_mode_voltage) > margin \
            and not CabinSystem.vehicle_state_value(situation.vehicle, "controller_main_delayed", false):
        moved = step_main(situation, 1)
    # no current on the further positions: the relay tripped or the engine is not on
    if float(state.get("Im", 0.0)) == 0.0 and main_powercontroller_position(situation) > 1:
        retry = true
    return moved


func decrease(situation:MaszynaLegacyDriverTraction.Situation, _force:bool = false) -> bool:
    # the field shunt off first
    if controller_position(situation, "controller_second_position") > 0:
        return set_second_controller(situation, 0)
    # DecMainCtrl(min(MainCtrlPowerPos(), 2)): one position, or back to the previous resistorless
    # one (Mover.cpp:2607-2619)
    var power:int = main_powercontroller_position(situation)
    if power <= 0:
        return false
    if power == 1:
        return step_main(situation, -1)
    var engine:VehicleElectricSeriesEngine = RailVehicleServer.vehicle_component_get(
            situation.controlling, VehicleComponentType.COMPONENT_ENGINE) as VehicleElectricSeriesEngine
    var relays:Array = engine.relay_list if engine else []
    var main:int = controller_position(situation, "controller_main_position")
    if main >= relays.size():
        return step_main(situation, -1)
    if (relays[main] as RelayListItem).resistance == 0.0:
        step_main(situation, -1)
    while (relays[controller_position(situation, "controller_main_position")] as RelayListItem).resistance > 0.0 \
            and step_main(situation, -1):
        pass
    return not controller_position(situation, "controller_main_position") == main


func control_handles(situation:MaszynaLegacyDriverTraction.Situation) -> void:
    control_series_motor_handles(situation)
