@tool
extends DriverDelegate
class_name MaszynaLegacyAIDriver

## The original's AI driver (TController, Driver.cpp): the orders a scenario gives a train, in the
## original's vocabulary (TController::PutCommand(), Driver.cpp:4468-4906), and the list of orders
## it works through (OrderList, OrderNext(), OrderPush(), JumpToNextOrder(), Driver.cpp:2040,
## 5125-5236). It takes the orders and carries out those that are a sequence of controls -
## preparing the vehicle, putting it away, turning (PrepareEngine(), ReleaseEngine(), Activation(),
## Driver.cpp:2051, 2759, 2918) - through the cab, as a player does (MaszynaLegacyDriverHints);
## driving is not ported yet (TODO.md, "Drivers"). It acts in moments, one reaction time apart
## (DriverSystem.driver_schedule_update()). One delegate serves every driver, their state is kept
## per driver RID.

## TOrders (Driver.h:29-45): the operations are bits, so a change of direction can sit on top of
## another order
enum Order {
    WAIT_FOR_ORDERS = 0,
    PREPARE_ENGINE = 1,
    RELEASE_ENGINE = 2,
    CHANGE_DIRECTION = 4,
    CONNECT = 8,
    DISCONNECT = 16,
    SHUNT = 32,
    LOOSE_SHUNT = 64,
    OBEY_TRAIN = 128,
    BANK = 256,
    JUMP_TO_FIRST_ORDER = 512,
}

## Orders a driver's list holds (maxorders, Driver.h:191)
const MAX_ORDERS:int = 64
## `Timetable:<name>` - the name is a file in the scenery's directory, `none` for no timetable
const TIMETABLE_PREFIX:String = "Timetable:"
const NO_TIMETABLE:String = "none"
const SCENERY_DIRECTORY:String = "scenery"
## The shunting speed until an order gives another [km/h] (fShuntVelocity, Driver.h:431)
const DEFAULT_SHUNT_VELOCITY:float = 40.0
## A shunting speed is kept only when it is a speed (fabs(NewValue1) > 2.0, Driver.cpp:4667)
const MIN_SHUNT_VELOCITY:float = 2.0
## `Shunt <vehicles> <coupler>`: -1 all the vehicles; below -1.5 wait for the signal; below -2.5
## train or bank after (Driver.cpp:4762-4863)
const ALL_VEHICLES:float = -1.0
const WAIT_FOR_SIGNAL:float = -1.5
const TRAIN_AFTER_SHUNT:float = -2.5
## A timetable speed between these starts in shunting (OrdersInit(), Driver.cpp:5250-5251)
const SHUNT_START_VELOCITY_MAX:float = 0.02
## The vehicle's couplers (end::front, end::rear)
const FRONT_END:int = 0
const REAR_END:int = 1
## The orders a driver goes on with while its engine is not ready (handle_engine(), Driver.cpp:7239)
const DRIVING_ORDERS:int = (Order.CHANGE_DIRECTION | Order.CONNECT | Order.DISCONNECT | Order.SHUNT
        | Order.LOOSE_SHUNT | Order.OBEY_TRAIN | Order.BANK)
## Reaction times [s]: preparing a standing vehicle, and otherwise (PrepareTime, EasyReactionTime,
## Driver.cpp:150, 158)
const PREPARE_TIME:float = 2.0
const EASY_REACTION_TIME:float = 0.5
## Preparing a vehicle rolling faster than this [km/h] takes the easy reaction time (Driver.cpp:2765)
const ROLLING_START_SPEED:float = 5.0
## A vehicle slower than this [km/h] stands (EU07_AI_NOMOVEMENT, Driver.h:25)
const NO_MOVEMENT_SPEED:float = 0.05
## The main reservoir pressure the vehicle is ready to drive at (ScndPipePress, Driver.cpp:2894)
const MIN_MAIN_RESERVOIR_PRESSURE:float = 4.5
## Crew moves one cab at a time, 1 -> 0 -> -1 (TMoverParameters::ChangeCab(), Mover.cpp:784)
const CAB_CHANGE_STEPS:int = 2


## What one driver keeps
class DriverState:
    var orders:PackedInt32Array = []
    var order_position:int = 0
    var order_top:int = 1
    ## iEngineActive - the vehicle is ready to drive (_prepare_engine())
    var engine_active:bool = false
    ## iDirection (the cab it drives from, +1 or -1) and iDirectionOrder (the one it was told to)
    var direction:int = 1
    var direction_order:int = 0
    ## SetVelocity/ShuntVelocity: the speed allowed and the one after it; stop_here - not to move
    ## towards a signal until told to (moveStopHere)
    ## at first it stands (Driver.h:392)
    var velocity:float = 0.0
    var velocity_next:float = -1.0
    var stop_here:bool = true
    var shunt_velocity:float = DEFAULT_SHUNT_VELOCITY
    ## iVehicleCount, iCoupler, fStopTime of `Shunt` and `Wait_for_orders`
    var vehicle_count:int = -2
    var coupler:int = 0
    var stop_time:float = 0.0
    ## vCommandLocation - where the last order about speed came from
    var command_position:Vector3 = Vector3.ZERO
    var radio_channel:int = -1
    ## m_lighthints - the light pattern asked for at the front and the rear, -1 for none
    var light_hints:Vector2i = Vector2i(-1, -1)
    ## fWarningDuration and the horn to sound
    var warning_duration:float = 0.0
    var warning_horn:int = 0
    var timetable:Timetable = null
    ## TTrainParameters::StationIndex - the station it drives to next
    var station_index:int = 0
    ## ReactionTime - until its next update
    var reaction_time:float = PREPARE_TIME
    ## What it read of its trainset on its last update
    var trainset:MaszynaLegacyDriverTrainset = MaszynaLegacyDriverTrainset.new()
    ## The speed and acceleration it wants
    var speed:MaszynaLegacyDriverSpeed = MaszynaLegacyDriverSpeed.new()
    ## Its own train brake handle position and brake timing
    var braking:MaszynaLegacyDriverBraking = MaszynaLegacyDriverBraking.new()
    ## What it reads of the tracks ahead
    var route:MaszynaLegacyDriverRoute = MaszynaLegacyDriverRoute.new()

    func _init() -> void:
        orders.resize(MAX_ORDERS)


var _drivers:Dictionary[RID, DriverState] = {}


func _driver_attached(driver:RID) -> void:
    var state:DriverState = DriverState.new()
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    if vehicle.is_valid() and RailVehicleServer.vehicle_get_driver_type(vehicle) == VehicleController.DRIVER_REAR:
        state.direction = -1
    _drivers[driver] = state
    DriverSystem.driver_schedule_update(driver, 0.0)


func _driver_detached(driver:RID) -> void:
    _drivers.erase(driver)


## What the driver keeps: its orders and what they asked for
func get_state(driver:RID) -> Dictionary:
    var state:DriverState = _drivers.get(driver)
    if not state:
        return {}
    return {
        "order": state.orders[state.order_position],
        "orders": state.orders.slice(0, state.order_top),
        "order_position": state.order_position,
        "direction": state.direction,
        "direction_order": state.direction_order,
        "velocity": state.velocity,
        "velocity_next": state.velocity_next,
        "stop_here": state.stop_here,
        "shunt_velocity": state.shunt_velocity,
        "vehicle_count": state.vehicle_count,
        "coupler": state.coupler,
        "stop_time": state.stop_time,
        "radio_channel": state.radio_channel,
        "light_hints": state.light_hints,
        "warning_duration": state.warning_duration,
        "warning_horn": state.warning_horn,
        "timetable": state.timetable,
        "station_index": state.station_index,
        "trainset_vehicles": state.trainset.vehicles,
        "trainset_mass": state.trainset.mass,
        "trainset_ready": state.trainset.ready,
        "trainset_brake_pressure_max": state.trainset.brake_pressure_max,
        "trainset_braked": state.trainset.braked,
        "trainset_gravity_acceleration": state.trainset.gravity_acceleration,
        "trainset_acceleration": state.trainset.acceleration,
        "velocity_desired": state.speed.velocity_desired,
        "acceleration_desired": state.speed.acceleration_desired,
        "brake_position": state.braking.position,
        "route_velocity_next": state.route.velocity_next,
        "proximity_distance": state.route.proximity_distance,
        "signal_velocity_next": state.route.signal_velocity_next,
    }


func _handle_command(driver:RID, command:String, value1:float, value2:float, position:Vector3) -> void:
    var state:DriverState = _drivers.get(driver)
    if not state:
        return
    if command.begins_with(TIMETABLE_PREFIX):
        _take_timetable(driver, state, command.trim_prefix(TIMETABLE_PREFIX), value1, value2, position)
        return
    match command:
        "SetVelocity":
            state.command_position = position
            if not value1 == 0.0 and not state.orders[state.order_position] == Order.OBEY_TRAIN:
                if not state.engine_active:
                    _order_next(state, Order.PREPARE_ENGINE)
                _order_next(state, Order.OBEY_TRAIN)
                _order_check(state)
            state.stop_here = value1 == 0.0
            state.velocity = value1
            state.velocity_next = value2
        "ShuntVelocity":
            state.command_position = position
            if not state.engine_active:
                _order_next(state, Order.PREPARE_ENGINE)
            _order_next(state, Order.SHUNT)
            if not value1 == 0.0:
                state.vehicle_count = -2
            state.velocity = value1
            state.velocity_next = value2
            state.stop_here = value1 == 0.0
            if absf(value1) > MIN_SHUNT_VELOCITY:
                state.shunt_velocity = absf(value1)
        "Wait_for_orders":
            if value1 > 0.0 and value1 > state.stop_time:
                state.stop_time = value1
            else:
                state.orders[state.order_position] = Order.WAIT_FOR_ORDERS
        "Prepare_engine":
            _orders_clear(state)
            if value1 == 0.0:
                _order_next(state, Order.RELEASE_ENGINE)
            elif value1 > 0.0:
                _order_next(state, Order.PREPARE_ENGINE)
        "Change_direction":
            var previous:int = state.orders[state.order_position]
            if not state.engine_active:
                _order_next(state, Order.PREPARE_ENGINE)
            if value1 == 0.0:
                state.direction_order = -state.direction
            else:
                state.direction_order = _direction_towards(driver, position, value1)
            if not state.direction_order == state.direction:
                _order_next(state, Order.CHANGE_DIRECTION)
            if previous >= Order.SHUNT:
                _order_next(state, previous)
            elif previous == Order.WAIT_FOR_ORDERS:
                _order_next(state, Order.SHUNT)
        "Obey_train", "Bank":
            if not state.engine_active:
                _order_next(state, Order.PREPARE_ENGINE)
            _order_next(state, Order.OBEY_TRAIN if command == "Obey_train" else Order.BANK)
            _order_check(state)
        "Shunt", "Loose_shunt":
            _take_shunt(driver, state, command == "Loose_shunt", value1, value2)
        "Jump_to_first_order":
            _jump_to_first_order(state)
        "Jump_to_order":
            if value1 == -1.0:
                _jump_to_next_order(state)
            elif value1 >= 0.0 and value1 < MAX_ORDERS:
                # the first position only starts it, for the old sceneries (Driver.cpp:4881-4884)
                state.order_position = maxi(floori(value1), 1)
        "Warning_signal":
            if value1 > 0.0 and value2 > 0.0:
                state.warning_duration = value1
                state.warning_horn = int(value2)
        "Radio_channel":
            if value1 >= 0.0:
                state.radio_channel = int(value1)
        "SetLights":
            state.light_hints = Vector2i(int(value1), int(value2))


## One moment of the driver (TController::Update(), handle_engine(), handle_orders(),
## UpdateChangeDirection(), Driver.cpp:6895-7260): what its current order asks of the cab
func _update(driver:RID) -> void:
    var state:DriverState = _drivers.get(driver)
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    if not state or not vehicle.is_valid():
        return
    # the time since the last update is the reaction time it was scheduled with
    var elapsed:float = state.reaction_time
    state.reaction_time = EASY_REACTION_TIME
    state.trainset.update(vehicle, state.direction, _has_diesel_engine(vehicle))
    # DirectionalVel(), Driver.h:312: the speed, negative when it runs against the way it drives
    var directional_speed:float = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0)) \
            * signf(state.direction * float(CabinSystem.vehicle_state_value(vehicle, "velocity", 0.0)))
    # shunting, the speed allowed is the shunting speed (pick_optimal_speed(), Driver.cpp:7320-7327)
    if not state.orders[state.order_position] & (Order.OBEY_TRAIN | Order.BANK):
        state.velocity = state.shunt_velocity
    # the tracks ahead, and the orders the signals and memories there give (check_route_ahead())
    state.route.update(
            vehicle, state.orders[state.order_position], state.stop_here, state.velocity, directional_speed,
            MaszynaLegacyDriverSpeed.EASY_ACCELERATION, state.trainset.velocity_max, state.trainset)
    state.velocity = state.route.signal_velocity
    for command:Array in state.route.commands:
        _handle_command(driver, command[0], command[1], command[2], command[3])
    state.speed.pick(
            state.orders[state.order_position], state.engine_active, state.stop_here, state.velocity,
            state.shunt_velocity, state.timetable.velocity if state.timetable else 0.0,
            directional_speed, state.trainset, state.route)
    # a player drives it: the driver takes orders and reads the trainset, and touches nothing
    if not DriverSystem.vehicle_is_control_active(vehicle):
        DriverSystem.driver_schedule_update(driver, state.reaction_time)
        return
    _control_security_system(vehicle, CabinSystem.occupied_cab(vehicle))
    # the power and the brakes, as the original's AI decides them on every update (UpdateSituation())
    MaszynaLegacyDriverTraction.control(
            vehicle, CabinSystem.occupied_cab(vehicle), state.speed, state.trainset, directional_speed)
    state.braking.control(
            vehicle, CabinSystem.occupied_cab(vehicle), state.orders[state.order_position], state.speed,
            state.trainset, directional_speed, elapsed)
    var cab:int = CabinSystem.occupied_cab(vehicle)
    var standing:bool = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0)) < NO_MOVEMENT_SPEED
    # a vehicle somebody powered up gets ready to drive (the original's HACK, Driver.cpp:7226-7231)
    if state.orders[state.order_position] == Order.WAIT_FOR_ORDERS and not state.engine_active \
            and CabinSystem.vehicle_state_value(vehicle, "power24_available", false):
        _order_next(state, Order.PREPARE_ENGINE)
    if state.orders[state.order_position] == Order.PREPARE_ENGINE:
        if _prepare_engine(state, vehicle, cab):
            _jump_to_next_order(state)
    if state.orders[state.order_position] & DRIVING_ORDERS and not state.engine_active:
        _prepare_engine(state, vehicle, cab)
    if state.orders[state.order_position] == Order.RELEASE_ENGINE and standing:
        if _release_engine(state, vehicle, cab):
            _jump_to_next_order(state)
    if state.orders[state.order_position] & Order.CHANGE_DIRECTION and standing:
        _activation(state, vehicle)
        if state.direction == state.direction_order:
            _prepare_engine(state, vehicle, CabinSystem.occupied_cab(vehicle))
            _jump_to_next_order(state)
    DriverSystem.driver_schedule_update(driver, state.reaction_time)


## PrepareEngine() (Driver.cpp:2759-2916): the steps that get the vehicle ready, cued on every
## update until it is. What the cab does not have yet is left out (TODO.md, "Drivers").
func _prepare_engine(state:DriverState, vehicle:RID, cab:int) -> bool:
    var speed:float = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0))
    state.reaction_time = PREPARE_TIME if speed < ROLLING_START_SPEED else EASY_REACTION_TIME
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.BATTERY_ON)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CAB_ACTIVATION)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.RADIO_ON)
    if _has_diesel_engine(vehicle):
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.OIL_PUMP_ON)
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FUEL_PUMP_ON)
    # both pantographs up (Driver.cpp:2822-2826)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_ON)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_ON)
    # PrepareDirection() (Driver.cpp): the reverser the way the driver drives, relative to the cab
    var cab_active:int = int(CabinSystem.vehicle_state_value(vehicle, "cabin", cab))
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, state.direction * cab_active)
    var converter_overload:bool = CabinSystem.vehicle_state_value(vehicle, "converter_overload", false)
    if converter_overload:
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.COMPRESSOR_OFF)
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CONVERTER_OFF)
        CabinSystem.act(vehicle, cab, &"converterfuse_bt", &"hold")
        CabinSystem.act(vehicle, cab, &"converterfuse_bt", &"release")
    var mains:bool = CabinSystem.vehicle_state_value(vehicle, "main_switch_enabled", false)
    if not mains:
        MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
        MaszynaLegacyDriverHints.close_line_breaker(vehicle, cab)
    elif not converter_overload:
        var converter_enabled:bool = MaszynaLegacyDriverHints.cue(
                vehicle, cab, MaszynaLegacyDriverHints.Hint.CONVERTER_ON)
        if converter_enabled:
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.COMPRESSOR_ON)
        MaszynaLegacyDriverHints.release_train_brake(vehicle, cab)
    var converter:Variant = CabinSystem.vehicle_state_value(vehicle, "converter_enabled")
    state.engine_active = not converter_overload and mains \
            and not int(CabinSystem.vehicle_state_value(vehicle, "direction", 0)) == 0 \
            and (converter == null or bool(converter)) \
            and float(CabinSystem.vehicle_state_value(vehicle, "compressor_pressure", 0.0)) > MIN_MAIN_RESERVOIR_PRESSURE
    return state.engine_active


## ReleaseEngine() (Driver.cpp:2918-3012): the vehicle put away, standing, step by step; done
## when it is dead
func _release_engine(state:DriverState, vehicle:RID, cab:int) -> bool:
    state.reaction_time = PREPARE_TIME
    MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, 0)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.COMPRESSOR_OFF)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CONVERTER_OFF)
    MaszynaLegacyDriverHints.open_line_breaker(vehicle, cab)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FRONT_PANTOGRAPH_VALVE_OFF)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.REAR_PANTOGRAPH_VALVE_OFF)
    if not CabinSystem.vehicle_state_value(vehicle, "main_switch_enabled", false):
        if _has_diesel_engine(vehicle):
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FUEL_PUMP_OFF)
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.OIL_PUMP_OFF)
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.RADIO_OFF)
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.BATTERY_OFF)
    var released:bool = int(CabinSystem.vehicle_state_value(vehicle, "direction", 0)) == 0 \
            and not CabinSystem.vehicle_state_value(vehicle, "main_switch_enabled", false) \
            and not CabinSystem.vehicle_state_value(vehicle, "power24_available", false)
    if released:
        state.engine_active = false
        _order_next(state, Order.WAIT_FOR_ORDERS)
    return released


## Activation() (Driver.cpp:2051-2145): turning a standing vehicle - the controls zeroed, the crew
## to the cab facing the new way (CabOccupied = iDirection), that cab switched on and its reverser
## forward. A move to another vehicle of the trainset is not ported (TODO.md).
func _activation(state:DriverState, vehicle:RID) -> void:
    state.direction = state.direction_order
    var cab:int = CabinSystem.occupied_cab(vehicle)
    MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, 0)
    # the crew walks as a player does - a command of the vehicle, not a control of the cab
    for _step:int in CAB_CHANGE_STEPS:
        if CabinSystem.occupied_cab(vehicle) == state.direction:
            break
        RailVehicleServer.vehicle_send_command(vehicle, "cab_change", signi(state.direction - CabinSystem.occupied_cab(vehicle)))
    cab = CabinSystem.occupied_cab(vehicle)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CAB_ACTIVATION)
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, 1)


## control_security_system() (Driver.cpp:6382-6408): the vigilance and the cab signal acknowledged
## while they flash, the reverser forward first if it stands at neutral. The train brake the
## security system applied is released by the driving (MaszynaLegacyDriverBraking). Radio-Stop's
## radio switched off at a stop is not ported yet (TODO.md).
func _control_security_system(vehicle:RID, cab:int) -> void:
    var cabsignal:bool = CabinSystem.vehicle_state_value(vehicle, "cabsignal_blinking", false) \
            and CabinSystem.vehicle_state_value(vehicle, "separate_acknowledge", false)
    var blinking:bool = CabinSystem.vehicle_state_value(vehicle, "blinking", false)
    if (cabsignal or blinking) and int(CabinSystem.vehicle_state_value(vehicle, "direction", 0)) == 0:
        MaszynaLegacyDriverHints.set_direction(vehicle, cab, int(CabinSystem.vehicle_state_value(vehicle, "cabin", cab)))
    if cabsignal:
        MaszynaLegacyDriverHints.reset_security_system(vehicle, cab, MaszynaLegacyDriverHints.CABSIGNAL_RESET)
    if blinking:
        MaszynaLegacyDriverHints.reset_security_system(vehicle, cab, MaszynaLegacyDriverHints.SECURITY_RESET)


static func _has_diesel_engine(vehicle:RID) -> bool:
    var engine_type:int = int(CabinSystem.vehicle_state_value(vehicle, "engine_type", VehicleEngine.NONE))
    return engine_type == VehicleEngine.DIESEL or engine_type == VehicleEngine.DIESEL_ELECTRIC


## `Timetable:<name> <velocity> <minutes>` (Driver.cpp:4494-4576): the timetable, the first station
## to drive to, the direction towards where the order came from and the orders it makes
func _take_timetable(
    driver:RID, state:DriverState, name:String, velocity:float, minutes:float, position:Vector3
) -> void:
    state.timetable = null
    state.station_index = 0
    if not name == NO_TIMETABLE:
        var directory:String = UserSettings.get_maszyna_game_dir().path_join(SCENERY_DIRECTORY)
        state.timetable = MaszynaLegacyTimetableFactory.load_timetable(directory, name, roundf(minutes))
        if state.timetable:
            state.station_index = 1
    if not position == Vector3.ZERO:
        state.direction_order = _direction_towards(driver, position, velocity)
    _orders_init(state, absf(velocity))


## The orders a timetable makes (OrdersInit(), Driver.cpp:5238-5319): start the engine, then shunt
## without a timetable, or drive it - turning where a station says `@` - and shunt after
func _orders_init(state:DriverState, velocity:float) -> void:
    _orders_clear(state)
    _order_push(state, Order.PREPARE_ENGINE)
    var entries:Array = state.timetable.entries if state.timetable else []
    if entries.is_empty():
        _order_push(state, Order.SHUNT)
    else:
        if velocity > 0.0 and velocity < SHUNT_START_VELOCITY_MAX:
            _order_push(state, Order.SHUNT)
        else:
            _order_push(state, Order.OBEY_TRAIN)
        for index:int in entries.size():
            var entry:TimetableEntry = entries[index]
            if entry.facilities.contains("@"):
                # a train of wagons leaves its locomotive; a push-pull set only turns - see TODO.md
                _order_push(state, Order.DISCONNECT)
                _order_push(state, Order.SHUNT)
                if index < entries.size() - 1:
                    _order_push(state, Order.OBEY_TRAIN)
        _order_push(state, Order.SHUNT)
    if velocity == 0.0:
        state.velocity = 0.0
        return
    state.stop_here = not (velocity >= 1.0 or velocity < SHUNT_START_VELOCITY_MAX)
    _jump_to_first_order(state)
    state.velocity = velocity if velocity >= 1.0 else 0.0


## `Shunt`/`Loose_shunt <vehicles> <coupler>` (Driver.cpp:4749-4867): couple up by the coupler, leave
## the vehicles counted, and shunt - or drive as a train - after
func _take_shunt(driver:RID, state:DriverState, loose:bool, vehicles:float, coupler:float) -> void:
    state.stop_here = false
    if not state.engine_active:
        _order_next(state, Order.PREPARE_ENGINE)
    if not coupler == 0.0:
        state.coupler = floori(absf(coupler))
        _order_next(state, Order.CONNECT)
        if vehicles >= 0.0:
            # after coupling, pull away the vehicles counted: turn first, they are behind
            state.direction_order = -state.direction
            _order_push(state, Order.CHANGE_DIRECTION)
            _order_push(state, Order.DISCONNECT)
            if coupler > 0.0 and loose:
                # after leaving them, carry on pushing the way it came
                state.direction_order = state.direction
                _order_push(state, Order.CHANGE_DIRECTION)
        elif coupler < 0.0:
            state.direction_order = -state.direction
            _order_next(state, Order.CHANGE_DIRECTION)
    elif vehicles >= 0.0:
        var behind:bool = _is_coupled(driver, REAR_END if state.direction > 0 else FRONT_END)
        var ahead:bool = _is_coupled(driver, FRONT_END if state.direction > 0 else REAR_END)
        if not behind and ahead:
            # the vehicles are in front: turn first, then leave them
            state.direction_order = -state.direction
            _order_next(state, Order.CHANGE_DIRECTION)
            _order_push(state, Order.DISCONNECT)
        elif behind:
            _order_next(state, Order.DISCONNECT)
        else:
            # nothing on either side: stand where it is (a use the old sceneries make of it)
            state.velocity = 0.0
            state.stop_here = true
    if vehicles < WAIT_FOR_SIGNAL:
        state.stop_here = true
    var next:int
    if vehicles < TRAIN_AFTER_SHUNT:
        next = Order.BANK if loose else Order.OBEY_TRAIN
    else:
        next = Order.LOOSE_SHUNT if loose else Order.SHUNT
    _order_next(state, next)
    state.vehicle_count = floori(vehicles)


## Forwards (+1) or back (-1) along the vehicle, towards where the order came from - or away, for a
## negative value (Driver.cpp:4707-4715)
func _direction_towards(driver:RID, position:Vector3, value:float) -> int:
    var transform:Transform3D = RailVehicleServer.vehicle_get_transform(DriverSystem.driver_get_vehicle(driver))
    var towards:Vector3 = position - transform.origin
    var front:Vector3 = -transform.basis.z
    return 1 if (towards.x * front.x + towards.z * front.z) * value > 0.0 else -1


## Whether something is coupled at the vehicle's end - the walk out through it starts beyond it
func _is_coupled(driver:RID, end:int) -> bool:
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    var coupled:Array[RID] = RailVehicleServer.vehicle_get_coupled(vehicle, end, VehicleController.COUPLING_ELEMENT_COUPLER)
    return not coupled.is_empty() and not coupled[0] == vehicle


func _orders_clear(state:DriverState) -> void:
    state.order_position = 0
    state.order_top = 1
    state.orders.fill(Order.WAIT_FOR_ORDERS)


## The order to do next: in place of what it does now, or after the operations under way
## (OrderNext(), Driver.cpp:5187-5207)
func _order_next(state:DriverState, order:int) -> void:
    if state.orders[state.order_position] == order:
        return
    if state.order_position == 0:
        state.order_position = 1
    state.order_top = state.order_position
    if order >= Order.SHUNT:
        while not state.orders[state.order_top] == Order.WAIT_FOR_ORDERS and state.orders[state.order_top] < Order.SHUNT:
            state.order_top += 1
    else:
        while state.orders[state.order_top] and state.orders[state.order_top] < Order.SHUNT and not state.orders[state.order_top] == order:
            state.order_top += 1
    state.orders[state.order_top] = order
    state.order_top += 1


## An order added after the rest (OrderPush(), Driver.cpp:5209-5220)
func _order_push(state:DriverState, order:int) -> void:
    if state.order_position == state.order_top and state.orders[state.order_position] < Order.SHUNT:
        state.order_top += 1
    if not state.orders[state.order_top] == order:
        state.orders[state.order_top] = order
        state.order_top += 1


func _jump_to_next_order(state:DriverState) -> void:
    var current:int = state.orders[state.order_position]
    if not current == Order.WAIT_FOR_ORDERS:
        if current & Order.CHANGE_DIRECTION and not current == Order.CHANGE_DIRECTION:
            # a change of direction on top of another order goes first
            state.orders[state.order_position] = current & ~Order.CHANGE_DIRECTION
            _order_check(state)
            return
        state.order_position = (state.order_position + 1) % MAX_ORDERS
    _order_check(state)


func _jump_to_first_order(state:DriverState) -> void:
    state.order_position = 1
    state.order_top = maxi(state.order_top, 1)
    _order_check(state)


## What a new order changes at once (OrderCheck(), Driver.cpp:5161-5185) - the lights and the doors
## it checks belong to the driving (TODO.md)
func _order_check(state:DriverState) -> void:
    var current:int = state.orders[state.order_position]
    if current & Order.CHANGE_DIRECTION:
        state.direction_order = -state.direction
    elif current == Order.WAIT_FOR_ORDERS:
        _orders_clear(state)
