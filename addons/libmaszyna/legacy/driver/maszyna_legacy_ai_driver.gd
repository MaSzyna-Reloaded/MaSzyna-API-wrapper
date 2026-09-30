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
## What PrepareEngine() waits for before the vehicle is ready (isready, Driver.cpp:2843-2851), as
## bits: a converter overload relay open, the line breaker open, the reverser at neutral (DirActive),
## the converter off, the air below MIN_MAIN_RESERVOIR_PRESSURE
enum EngineCheck {
    CONVERTER_OVERLOAD = 1,
    LINE_BREAKER = 2,
    DIRECTION = 4,
    CONVERTER = 8,
    AIR = 16,
}

## Orders a driver's list holds (maxorders, Driver.h:191)
const MAX_ORDERS:int = 64
## `Timetable:<name>` - the name is a file in the scenery's directory, `none` for no timetable
const TIMETABLE_PREFIX:String = "Timetable:"
const NO_TIMETABLE:String = "none"
const SCENERY_DIRECTORY:String = "scenery"
## The guard's departure message (tsGuardSignal, Driver.cpp:4466-4483): <timetable>.<ext>, heard
## beside the train, else <timetable>radio.<ext> on the train radio, looked up in the scenery, then
## in the sounds, with its transcript beside it. Only the radio one is played, and not from a .flac
## file (TODO.md)
const SOUNDS_DIRECTORY:String = "sounds"
const GUARD_RADIO_SUFFIX:String = "radio"
const GUARD_SOUND_EXTENSIONS:PackedStringArray = ["ogg", "flac", "wav"]
const OGG_EXTENSION:String = "ogg"
const WAV_EXTENSION:String = "wav"
## EU07_SOUND_HANDHELDRADIORANGE (sound.h:21): as far as the guard's radio reaches [m]
const GUARD_RADIO_RANGE:float = 3500.0
## fActionTime = -5.0 once the guard has spoken (Driver.cpp:6872, 6882)
const GUARD_HOLD_TIME:float = 5.0
## iRadioChannel before it is told one (Driver.h)
const RADIO_CHANNEL_DEFAULT:int = 1
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
## Uncoupling: it presses the buffers at this speed [km/h] (Driver.cpp:7334)
const PRESSING_VELOCITY:float = 2.0
## Coupling up: it starts within this of the vehicle ahead, and couples within ATTACH_DISTANCE [m]
## (UpdateConnect(), Driver.cpp:7005-7040)
const CONNECT_DISTANCE:float = 20.0
const ATTACH_DISTANCE:float = 2.0
## The couplings a `Shunt` may ask for (coupling::, MOVER.h:162); the high voltage and the power
## lines are nothing a shunter joins
const SHUNTER_COUPLINGS:int = (RailVehicleController.COUPLING_FLAG_COUPLER
        | RailVehicleController.COUPLING_FLAG_BRAKEHOSE | RailVehicleController.COUPLING_FLAG_CONTROL
        | RailVehicleController.COUPLING_FLAG_GANGWAY | RailVehicleController.COUPLING_FLAG_MAINHOSE
        | RailVehicleController.COUPLING_FLAG_HEATING)


## What one driver keeps
class DriverState:
    var orders:PackedInt32Array = []
    var order_position:int = 0
    var order_top:int = 1
    ## iEngineActive - the vehicle is ready to drive (_prepare_engine()), and the EngineCheck
    ## flags that kept it from being ready on the last check
    var engine_active:bool = false
    var engine_missing:int = 0
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
    ## iCouplingVehicle - the vehicle of the trainset that couples up and its end, while it does
    ## (moveConnect)
    var coupling_vehicle:RID = RID()
    var coupling_end:RailVehicleController.CouplerEnd = RailVehicleController.COUPLER_END_FRONT
    ## movePress - it presses the buffers to uncouple; iDirectionBackup - the way it drove before,
    ## 0 for none
    var pressing:bool = false
    var direction_backup:int = 0
    ## vCommandLocation - where the last order about speed came from
    var command_position:Vector3 = Vector3.ZERO
    var radio_channel:int = -1
    ## tsGuardSignal and iGuardRadio - the guard's departure message, null for none, its
    ## transcript and the radio channel it is sent on; guard_signal_due - it is due once the train
    ## may go (moveGuardSignal)
    var guard_signal:SfxEvent = null
    var guard_transcript:Transcript = null
    var guard_radio:int = 0
    var guard_signal_due:bool = false
    ## m_lighthints - the light pattern asked for at the front and the rear, -1 for none
    var light_hints:Vector2i = Vector2i(-1, -1)
    ## fWarningDuration and the horn to sound
    var warning_duration:float = 0.0
    var warning_horn:int = 0
    ## Its timetable and how far it got through it
    var timetable:MaszynaLegacyDriverTimetable = MaszynaLegacyDriverTimetable.new()
    ## ReactionTime - until its next update
    var reaction_time:float = PREPARE_TIME
    ## What it read of its trainset on its last update
    var trainset:MaszynaLegacyDriverTrainset = MaszynaLegacyDriverTrainset.new()
    ## The speed and acceleration it wants
    var speed:MaszynaLegacyDriverSpeed = MaszynaLegacyDriverSpeed.new()
    ## Its own train brake handle position and brake timing
    var braking:MaszynaLegacyDriverBraking = MaszynaLegacyDriverBraking.new()
    ## How it drives the engine of its vehicle - the kind of engine's own (create())
    var traction:MaszynaLegacyDriverTraction = MaszynaLegacyDriverTraction.create(RailVehicleEngine.NONE)
    ## What it reads of the tracks ahead
    var route:MaszynaLegacyDriverRoute = MaszynaLegacyDriverRoute.new()

    func _init() -> void:
        orders.resize(MAX_ORDERS)


var _drivers:Dictionary[RID, DriverState] = {}


func _driver_attached(driver:RID) -> void:
    var state:DriverState = DriverState.new()
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    if vehicle.is_valid() and VehicleServer.vehicle_get_driver_type(vehicle) == VehicleController.DRIVER_REAR:
        state.direction = -1
    # told to drive the way it faces until told otherwise (iDirectionOrder = CabActive,
    # Driver.cpp:1872) - never none: turning towards none puts the reverser at neutral and takes
    # that for the way it drives
    state.direction_order = state.direction
    state.timetable.changed.connect(DriverSystem.driver_report_timetable_changed.bind(driver))
    _drivers[driver] = state
    DriverSystem.driver_schedule_update(driver, 0.0)


func _driver_detached(driver:RID) -> void:
    _drivers.erase(driver)


## A player left the cab: the driver drives the vehicle as it was left (TakeControl(),
## Driver.cpp:5700-5712) - its cab switched on, the way it drives guessed again
## (PrepareDirection(), Driver.cpp:5088-5116): standing, towards the cab driven from; moving, the way
## it moves - the reverser put that way, and the tracks read afresh
func _control_taken(driver:RID) -> void:
    var state:DriverState = _drivers.get(driver)
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    if not state or not vehicle.is_valid():
        return
    # the cab the crew sits in switched on (CabActivisation(true), Driver.cpp:5705)
    MaszynaLegacyDriverHints.cue(vehicle, CabinSystem.occupied_cab(vehicle), MaszynaLegacyDriverHints.Hint.CAB_ACTIVATION)
    if float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0)) < MaszynaLegacyDriverTrainset.NO_MOVEMENT_SPEED:
        # the active cab, else the one the crew sits in; a vehicle with neither keeps its way
        var cab:int = int(CabinSystem.vehicle_state_value(vehicle, "cabin", 0))
        if cab == 0:
            cab = CabinSystem.occupied_cab(vehicle)
        if not cab == 0:
            state.direction = cab
    else:
        state.direction = 1 if float(CabinSystem.vehicle_state_value(vehicle, "velocity", 0.0)) >= 0.0 else -1
    state.direction_order = state.direction
    state.route.forget()
    # and the reverser put that way - a player may have left it the other (PrepareDirection())
    _prepare_direction(state, vehicle, CabinSystem.occupied_cab(vehicle))
    # the lights of its order (control_lights(), CheckVehicles(), Driver.cpp:5625, 5657)
    _check_lights(state, vehicle)


## PrepareDirection() (Driver.cpp:5116-5121): the master controller at zero, then the reverser the
## way the driver drives, relative to the cab
func _prepare_direction(state:DriverState, vehicle:RID, cab:int) -> void:
    MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
    var cab_active:int = int(CabinSystem.vehicle_state_value(vehicle, "cabin", cab))
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, state.direction * cab_active)


## The timetable and how far the driver got through it (DriverDelegate.get_timetable_state())
func _get_timetable_state(driver:RID) -> Dictionary:
    var state:DriverState = _drivers.get(driver)
    if not state:
        return {}
    return {
        "timetable": state.timetable.timetable,
        "station_index": state.timetable.station_index,
        "station_start": state.timetable.station_start,
        "latency": state.timetable.latency,
        "delay": state.timetable.delay,
        "arrived": state.timetable.arrived,
    }


## The seconds from `hours` to the departure of the driver's train, NAN without a timetable
## (DriverDelegate.get_seconds_until_departure())
func _get_seconds_until_departure(driver:RID, hours:float) -> float:
    var state:DriverState = _drivers.get(driver)
    if not state or not state.timetable.timetable:
        return NAN
    return state.timetable.seconds_until_departure(hours)


## What the driver keeps: its orders and what they asked for (DriverDelegate.get_state())
func _get_state(driver:RID) -> Dictionary:
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
        "timetable": state.timetable.timetable,
        "station_index": state.timetable.station_index,
        "station_start": state.timetable.station_start,
        "next_stop": state.timetable.next_stop,
        "at_passenger_stop": state.route.at_passenger_stop,
        "trainset_vehicles": state.trainset.vehicles,
        "trainset_mass": state.trainset.mass,
        "trainset_ready": state.trainset.ready,
        "trainset_brake_pressure_max": state.trainset.brake_pressure_max,
        "trainset_braked": state.trainset.braked,
        "trainset_gravity_acceleration": state.trainset.gravity_acceleration,
        "trainset_acceleration": state.trainset.acceleration,
        "velocity_desired": state.speed.velocity_desired,
        "stop_reason": state.speed.stop_reason,
        "engine_active": state.engine_active,
        "engine_missing": state.engine_missing,
        "speed_velocity_next": state.speed.velocity_next,
        "velocity_limit_last": state.route.velocity_limit_last,
        "velocity_limit_last_distance": state.route.velocity_limit_last_distance,
        "timetable_velocity": state.timetable.velocity,
        "acceleration_desired": state.speed.acceleration_desired,
        "brake_position": state.braking.position,
        "route_velocity_next": state.route.velocity_next,
        "proximity_distance": state.speed.proximity_distance,
        "obstacle_distance": state.route.obstacle.distance if state.route.obstacle else -1.0,
        "signal_velocity_next": state.route.signal_velocity_next,
    }


func _handle_command(driver:RID, command:String, value1:float, value2:float, position:Vector3) -> void:
    var state:DriverState = _drivers.get(driver)
    if not state:
        return
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    # the original writes every order to its log (TController::PutCommand(), Driver.cpp:4470)
    GameLog.debug("%s: %s %s %s (order %s)" % [
            VehicleServer.vehicle_get_name(vehicle), command, value1, value2,
            state.orders[state.order_position]])
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
                _order_check(state, vehicle)
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
            _order_check(state, vehicle)
        "Shunt", "Loose_shunt":
            _take_shunt(driver, state, command == "Loose_shunt", value1, value2)
        "Jump_to_first_order":
            _jump_to_first_order(state, vehicle)
        "Jump_to_order":
            if value1 == -1.0:
                _jump_to_next_order(state, vehicle)
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
                # the guard's too (Driver.cpp:4797-4800)
                if state.guard_radio:
                    state.guard_radio = int(value1)
        "SetLights":
            # the scenery's pattern, lit at once on a train (Driver.cpp:4807-4816)
            state.light_hints = Vector2i(int(value1), int(value2))
            if state.orders[state.order_position] & Order.OBEY_TRAIN:
                _check_lights(state, vehicle)


## One moment of the driver (TController::Update(), handle_engine(), handle_orders(),
## UpdateChangeDirection(), Driver.cpp:6895-7260): what its current order asks of the cab
func _update(driver:RID) -> void:
    var state:DriverState = _drivers.get(driver)
    var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
    if not state or not vehicle.is_valid():
        return
    # the time since the last update is the reaction time it was scheduled with
    var elapsed:float = state.reaction_time
    state.trainset.update(vehicle, state.direction, _has_diesel_engine(vehicle))
    # what happened to the trainset from outside - a line breaker tripped by a loss of voltage, a
    # relay a player opened - takes the engine's readiness away, and handle_engine() gets it ready
    # again (determine_consist_state(), Driver.cpp:6100-6104)
    var converter:Variant = CabinSystem.vehicle_state_value(state.trainset.controlling, "converter_enabled")
    if state.trainset.line_breaker_open or state.trainset.converter_overload_relay_open \
            or not (converter == null or bool(converter)):
        state.engine_active = false
    # what its brakes can do - the table again when the trainset or the kind of order changed
    state.braking.read_trainset(
            vehicle, state.orders[state.order_position], state.trainset, DriverSystem.vehicle_is_control_active(vehicle))
    # DirectionalVel(), Driver.h:312: the speed, negative when it runs against the way it drives
    var directional_speed:float = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0)) \
            * signf(state.direction * float(CabinSystem.vehicle_state_value(vehicle, "velocity", 0.0)))
    # shunting, the speed allowed is the shunting speed (pick_optimal_speed(), Driver.cpp:7320-7327)
    if not state.orders[state.order_position] & (Order.OBEY_TRAIN | Order.BANK):
        state.velocity = state.shunt_velocity
    # the tracks ahead, and the orders the signals and memories there give (check_route_ahead())
    state.route.update(
            vehicle, state.orders[state.order_position], state.stop_here, state.velocity, directional_speed,
            MaszynaLegacyDriverSpeed.EASY_ACCELERATION, state.trainset.velocity_max, state.trainset,
            state.timetable, SimulationServer.time_of_day, state.shunt_velocity, state.speed.velocity_desired,
            state.coupling_vehicle.is_valid(), state.braking)
    state.velocity = state.route.signal_velocity
    # uncoupling: stand, then press the buffers at walking pace (pick_optimal_speed(), Driver.cpp:7330-7343)
    if state.orders[state.order_position] & Order.DISCONNECT and state.vehicle_count >= 0:
        var pressing:bool = state.pressing and state.direction == state.direction_order
        state.velocity = PRESSING_VELOCITY if pressing else 0.0
        state.velocity_next = 0.0
    # what its passenger stop asked of the orders (TableUpdateStopPoint(), Driver.cpp:1258-1370)
    for stop_order:MaszynaLegacyDriverRoute.StopOrder in state.route.stop_orders:
        match stop_order:
            MaszynaLegacyDriverRoute.StopOrder.HOLD:
                state.stop_here = true
            MaszynaLegacyDriverRoute.StopOrder.GO:
                state.stop_here = false
            MaszynaLegacyDriverRoute.StopOrder.OBEY_TRAIN:
                _order_next(state, Order.OBEY_TRAIN)
            MaszynaLegacyDriverRoute.StopOrder.TURN_THEN_TRAIN, MaszynaLegacyDriverRoute.StopOrder.TURN_THEN_SHUNT:
                if not state.orders[(state.order_position + 1) % MAX_ORDERS] == Order.CHANGE_DIRECTION:
                    _order_push(state, Order.CHANGE_DIRECTION)
                    _order_push(state,
                            Order.OBEY_TRAIN if stop_order == MaszynaLegacyDriverRoute.StopOrder.TURN_THEN_TRAIN
                            else Order.SHUNT)
            MaszynaLegacyDriverRoute.StopOrder.NEXT_ORDER:
                _jump_to_next_order(state, vehicle)
            MaszynaLegacyDriverRoute.StopOrder.GUARD_SIGNAL:
                # on the radio channel of the station left (Driver.cpp:1103-1112), once the train
                # may go (Driver.cpp:1313-1316)
                var left:TimetableEntry = state.timetable.get_entries()[state.timetable.station_index - 1]
                if state.guard_radio and left.radio_channel > 0:
                    state.guard_radio = left.radio_channel
                state.guard_signal_due = state.guard_signal != null
    for command:Array in state.route.commands:
        _handle_command(driver, command[0], command[1], command[2], command[3])
    # the guard's message, once the way is clear and the stop waited out - to a player's train as
    # much as to its own (UpdateObeyTrain(), Driver.cpp:6850-6884)
    if state.guard_signal_due and state.orders[state.order_position] == Order.OBEY_TRAIN \
            and state.speed.velocity_desired > 0.0:
        state.guard_signal_due = false
        CabinSystem.send_radio_message(state.guard_signal, state.guard_transcript, state.guard_radio,
                RailVehicleServer.vehicle_get_transform(vehicle).origin, GUARD_RADIO_RANGE)
        state.traction.hold(GUARD_HOLD_TIME)
    state.speed.pick(
            state.orders[state.order_position], state.engine_active, state.stop_here, state.velocity,
            state.shunt_velocity, state.timetable.velocity,
            directional_speed, state.trainset, state.route, EASY_REACTION_TIME, state.braking)
    state.reaction_time = state.speed.reaction_time
    var cab:int = CabinSystem.occupied_cab(vehicle)
    # the engine it drives decides how (IncSpeed()'s switch on the engine type, Driver.cpp:3409)
    var engine_type:RailVehicleEngine.EngineType = int(CabinSystem.vehicle_state_value(vehicle, "engine_type", RailVehicleEngine.NONE)) \
            as RailVehicleEngine.EngineType
    if not state.traction.engine_type == engine_type:
        state.traction = MaszynaLegacyDriverTraction.create(engine_type)
    var situation:MaszynaLegacyDriverTraction.Situation = MaszynaLegacyDriverTraction.Situation.new()
    situation.vehicle = vehicle
    situation.cab = cab
    situation.controlling = state.trainset.controlling
    situation.order = state.orders[state.order_position]
    situation.speed = state.speed
    situation.trainset = state.trainset
    situation.route = state.route
    situation.braking = state.braking
    situation.directional_speed = directional_speed
    situation.pressing = state.pressing
    situation.coupling = state.coupling_vehicle.is_valid() and bool(situation.order & Order.CONNECT)
    state.traction.read(situation, elapsed)
    # a player drives it: the driver takes orders and reads the trainset, and touches nothing - its
    # orders still follow the vehicle the player gets ready, as the timetable's stops need them
    if not DriverSystem.vehicle_is_control_active(vehicle):
        _handle_engine(state, vehicle, cab)
        DriverSystem.driver_schedule_update(driver, state.reaction_time)
        return
    _control_security_system(vehicle, cab)
    if state.engine_active:
        MaszynaLegacyDriverPantographs.control(vehicle, cab, state.trainset, state.direction,
                MaszynaLegacyDriverBraking.is_emu(vehicle), state.traction.action_time <= 0.0)
        # the delayed actions: the lights of its order kept up (Driver.cpp:4933-4936)
        if state.traction.action_time > 0.0:
            MaszynaLegacyDriverLights.control(
                    vehicle, state.direction, state.orders[state.order_position], state.light_hints)
    # the controllers held by time back to holding, the power and the brakes, the controllers
    # held by time set to work until the next update (UpdateSituation(), Driver.cpp:5016-5027)
    state.traction.check_time_controllers(situation)
    state.braking.check_time_controllers(situation)
    if state.traction.prepare(situation):
        state.traction.control(situation)
        state.braking.control(situation, elapsed)
    state.traction.set_time_controllers(situation)
    state.braking.set_time_controllers(situation)
    var standing:bool = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0)) < NO_MOVEMENT_SPEED
    _handle_engine(state, vehicle, cab)
    if state.orders[state.order_position] == Order.RELEASE_ENGINE and standing:
        if _release_engine(state, vehicle, cab):
            _jump_to_next_order(state, vehicle)
    match state.orders[state.order_position]:
        Order.CONNECT:
            _update_connect(state, vehicle)
        Order.DISCONNECT:
            _update_disconnect(state, situation)
    if state.orders[state.order_position] & Order.CHANGE_DIRECTION and standing:
        _activation(state, vehicle)
        if state.direction == state.direction_order:
            _prepare_engine(state, vehicle, CabinSystem.occupied_cab(vehicle))
            _jump_to_next_order(state, vehicle)
    DriverSystem.driver_schedule_update(driver, state.reaction_time)


## handle_engine() (Driver.cpp:7223-7244): the engine's orders - a vehicle somebody powered up gets
## ready to drive, and the driving orders follow once it is
func _handle_engine(state:DriverState, vehicle:RID, cab:int) -> void:
    # the original's HACK (Driver.cpp:7226-7231)
    if state.orders[state.order_position] == Order.WAIT_FOR_ORDERS and not state.engine_active \
            and CabinSystem.vehicle_state_value(vehicle, "power24_available", false):
        _order_next(state, Order.PREPARE_ENGINE)
    if state.orders[state.order_position] == Order.PREPARE_ENGINE:
        if _prepare_engine(state, vehicle, cab):
            _jump_to_next_order(state, vehicle)
    if state.orders[state.order_position] & DRIVING_ORDERS and not state.engine_active:
        _prepare_engine(state, vehicle, cab)


## PrepareEngine() (Driver.cpp:2759-2916): the steps that get the vehicle ready, cued on every
## update until it is - to a player only hinted (cue_action()), so the driver then just reads
## whether the vehicle is ready. What the cab does not have yet is left out (TODO.md, "Drivers").
func _prepare_engine(state:DriverState, vehicle:RID, cab:int) -> bool:
    var speed:float = float(CabinSystem.vehicle_state_value(vehicle, "speed", 0.0))
    state.reaction_time = PREPARE_TIME if speed < ROLLING_START_SPEED else EASY_REACTION_TIME
    var controlling:RID = state.trainset.controlling
    var converter_overload:bool = CabinSystem.vehicle_state_value(controlling, "converter_overload", false)
    var mains:bool = CabinSystem.vehicle_state_value(controlling, "main_switch_enabled", false)
    # to a player the steps are only hinted (cue_action()) - the driver just reads whether it is ready
    if DriverSystem.vehicle_is_control_active(vehicle):
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.BATTERY_ON)
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CAB_ACTIVATION)
        MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.RADIO_ON)
        if _has_diesel_engine(vehicle):
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.OIL_PUMP_ON)
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.FUEL_PUMP_ON)
        # the pantographs' air, and both up (Driver.cpp:2782-2811)
        MaszynaLegacyDriverPantographs.prepare(vehicle, cab, state.trainset, MaszynaLegacyDriverBraking.is_emu(vehicle))
        _prepare_direction(state, vehicle, cab)
        # the main circuit, the converter and the air are the engine's the controls drive - an EMU's
        # motor car (mvControlling, Driver.cpp:2820-2870)
        if converter_overload:
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.COMPRESSOR_OFF)
            MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CONVERTER_OFF)
            CabinSystem.act(vehicle, cab, &"converterfuse_bt", &"hold")
            CabinSystem.act(vehicle, cab, &"converterfuse_bt", &"release")
        if not mains:
            MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
            # a diesel with a gearbox starts at its idle position, or it stalls (Driver.cpp:2840-2843)
            var engine_type:RailVehicleEngine.EngineType = int(CabinSystem.vehicle_state_value(vehicle, "engine_type",
                    RailVehicleEngine.NONE)) as RailVehicleEngine.EngineType
            if engine_type == RailVehicleEngine.DIESEL:
                MaszynaLegacyDriverHints.set_idle(vehicle, cab)
            MaszynaLegacyDriverHints.close_line_breaker(vehicle, cab)
        elif not converter_overload:
            var converter_enabled:bool = MaszynaLegacyDriverHints.cue(
                    vehicle, cab, MaszynaLegacyDriverHints.Hint.CONVERTER_ON, controlling)
            if converter_enabled:
                MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.COMPRESSOR_ON, controlling)
            MaszynaLegacyDriverHints.release_train_brake(vehicle, cab)
    var converter:Variant = CabinSystem.vehicle_state_value(controlling, "converter_enabled")
    var missing:int = 0
    if converter_overload:
        missing |= EngineCheck.CONVERTER_OVERLOAD
    if not mains:
        missing |= EngineCheck.LINE_BREAKER
    if int(CabinSystem.vehicle_state_value(vehicle, "direction", 0)) == 0:
        missing |= EngineCheck.DIRECTION
    if not (converter == null or bool(converter)):
        missing |= EngineCheck.CONVERTER
    if float(CabinSystem.vehicle_state_value(controlling, "compressor_pressure", 0.0)) <= MIN_MAIN_RESERVOIR_PRESSURE:
        missing |= EngineCheck.AIR
    state.engine_missing = missing
    state.engine_active = missing == 0
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
    # lightsoff (Driver.cpp:2932)
    MaszynaLegacyDriverLights.off(vehicle, state.direction)
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
        MaszynaLegacyDriverHints.send(vehicle, "cab_change", signi(state.direction - CabinSystem.occupied_cab(vehicle)))
    cab = CabinSystem.occupied_cab(vehicle)
    MaszynaLegacyDriverHints.cue(vehicle, cab, MaszynaLegacyDriverHints.Hint.CAB_ACTIVATION)
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, 1)


## UpdateConnect() (Driver.cpp:6993-7055): within CONNECT_DISTANCE of the vehicle ahead the front
## vehicle of the trainset starts coupling up; within ATTACH_DISTANCE the shunter joins an element
## a time - the vehicle's `coupler_connect`, as the player's crew does - and once every element
## asked for is joined, the next order follows. The coupler adapter is not ported (TODO.md).
func _update_connect(state:DriverState, vehicle:RID) -> void:
    if not state.coupling_vehicle.is_valid():
        if state.route.obstacle and state.route.obstacle.distance <= CONNECT_DISTANCE and state.trainset.vehicles:
            state.coupling_vehicle = state.trainset.vehicles[0]
            state.coupling_end = (RailVehicleController.COUPLER_END_FRONT if state.trainset.front_direction > 0
                    else RailVehicleController.COUPLER_END_REAR)
        return
    if not _is_coupled_as_asked(state.coupling_vehicle, state.coupling_end, state.coupler):
        var neighbour:RailVehicleNeighbour = RailVehicleServer.vehicle_find_vehicle(
                state.coupling_vehicle, state.coupling_end, MaszynaLegacyDriverRoute.OBSTACLE_RANGE)
        if neighbour and neighbour.distance < ATTACH_DISTANCE:
            MaszynaLegacyDriverHints.send(state.coupling_vehicle, "coupler_connect", state.coupling_end)
    # the command joins at once: coupled now, it drives on
    if _is_coupled_as_asked(state.coupling_vehicle, state.coupling_end, state.coupler):
        state.coupler = 0
        state.coupling_vehicle = RID()
        _jump_to_next_order(state, vehicle)


## UpdateDisconnect() (Driver.cpp:7101-7235): leaving all but `vehicle_count` vehicles from the
## driver's. The train braked and the direction turned (2nd stage); the buffers pressed, the brakes
## of the vehicles released and the coupler undone by the shunter - the vehicles' `brake_releaser`
## and `coupler_disconnect`, as the player's crew does - (3rd); the direction restored and the next
## order (4th, 5th). The coupler adapter is not ported (TODO.md).
func _update_disconnect(state:DriverState, situation:MaszynaLegacyDriverTraction.Situation) -> void:
    var vehicle:RID = situation.vehicle
    var cab:int = situation.cab
    if state.vehicle_count >= 0:
        if not state.direction == state.direction_order:
            _reverse(state, vehicle, cab)
        # pressing and uncoupling only once the trainset was read the way it now drives: in the
        # update that turned, it is still the old front, and the walk from the locomotive found its
        # free coupler and took the uncoupling as done (FINDINGS.md, 2026-09-27)
        if state.pressing and state.direction == state.direction_order and state.trainset.direction == state.direction:
            state.braking.release_local_brake(vehicle, cab)
            state.traction.press(situation)
            # from the driver's vehicle into the ones pressed, as many as stay; a unit counts once
            var vehicles:Array[RID] = state.trainset.vehicles
            var index:int = vehicles.find(vehicle)
            var count:int = state.vehicle_count
            var decoupled:RID = RID()
            var end:RailVehicleController.CouplerEnd = RailVehicleController.COUPLER_END_FRONT
            while index >= 0:
                var current:RID = vehicles[index]
                end = _end_towards_front(state.trainset, index)
                if _is_coupled_by(current, end, RailVehicleController.COUPLING_FLAG_PERMANENT):
                    count += 1
                if not current == vehicle:
                    # released, to be pressed together
                    MaszynaLegacyDriverHints.send(current, "brake_releaser", true)
                if count == 0:
                    decoupled = current
                    break
                index -= 1
                count -= 1
            if not decoupled.is_valid():
                # nothing there to uncouple
                state.vehicle_count = -2
            else:
                # refused until the buffers are pressed enough: it presses on
                MaszynaLegacyDriverHints.send(decoupled, "coupler_disconnect", end)
                if not _is_coupled_by(decoupled, end, RailVehicleController.COUPLING_FLAG_COUPLER):
                    state.vehicle_count = -2
        if not state.pressing:
            if state.direction_backup == 0:
                state.direction_backup = state.direction
            if not state.trainset.braked:
                state.braking.apply_train_brake()
            else:
                state.direction_order = -state.direction
                state.pressing = true
    if state.vehicle_count < 0:
        if not state.direction_backup == 0:
            state.direction_order = state.direction_backup
            state.direction_backup = 0
        if not state.direction == state.direction_order:
            _reverse(state, vehicle, cab)
        if state.direction == state.direction_order:
            state.pressing = false
            _jump_to_next_order(state, vehicle)


## directionother (driverhints.cpp:935-946): the reverser the other way from the same cab, the
## master controller at zero first; the driver's direction follows once the reverser has moved
func _reverse(state:DriverState, vehicle:RID, cab:int) -> void:
    MaszynaLegacyDriverHints.set_zero_speed(vehicle, cab)
    var cab_active:int = int(CabinSystem.vehicle_state_value(vehicle, "cabin", cab))
    MaszynaLegacyDriverHints.set_direction(vehicle, cab, state.direction_order * cab_active)
    if int(CabinSystem.vehicle_state_value(vehicle, "direction", 0)) == state.direction_order * cab_active:
        state.direction = state.direction_order


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
    var engine_type:RailVehicleEngine.EngineType = int(CabinSystem.vehicle_state_value(vehicle, "engine_type", RailVehicleEngine.NONE)) \
            as RailVehicleEngine.EngineType
    return engine_type == RailVehicleEngine.DIESEL or engine_type == RailVehicleEngine.DIESEL_ELECTRIC


## `Timetable:<name> <velocity> <minutes>` (Driver.cpp:4494-4576): the timetable, the first station
## to drive to, the direction towards where the order came from and the orders it makes
func _take_timetable(
    driver:RID, state:DriverState, name:String, velocity:float, minutes:float, position:Vector3
) -> void:
    var timetable:Timetable = null
    if not name == NO_TIMETABLE:
        var directory:String = UserSettings.get_maszyna_game_dir().path_join(SCENERY_DIRECTORY)
        timetable = MaszynaLegacyTimetableFactory.load_timetable(directory, name, roundf(minutes))
    state.timetable.take(timetable)
    # the guard of the timetable, speaking on the radio (Driver.cpp:4424, 4474-4483)
    state.guard_signal = null
    state.guard_transcript = null
    state.guard_radio = 0
    state.guard_signal_due = false
    if timetable:
        var path:String = ""
        var radio:bool = false
        for suffix:String in ["", GUARD_RADIO_SUFFIX]:
            for directory:String in [SCENERY_DIRECTORY, SOUNDS_DIRECTORY]:
                for extension:String in GUARD_SOUND_EXTENSIONS:
                    var candidate:String = UserSettings.get_maszyna_game_dir().path_join(directory).path_join(
                            "%s%s.%s" % [name, suffix, extension])
                    if not path and FileAccess.file_exists(candidate):
                        path = candidate
                        radio = suffix == GUARD_RADIO_SUFFIX
        var stream:AudioStream = null
        if radio and path.get_extension() == OGG_EXTENSION:
            stream = AudioStreamOggVorbis.load_from_file(path)
        elif radio and path.get_extension() == WAV_EXTENSION:
            stream = AudioStreamWAV.load_from_file(path)
        if stream:
            var clip:SfxClip = SfxClip.new()
            clip.stream = stream
            var clips:Array[SfxClip] = [clip]
            state.guard_signal = SfxEvent.new()
            state.guard_signal.clips = clips
            # its caption beside it (openal_buffer::fetch_caption(), audio.cpp:84-94)
            state.guard_transcript = MaszynaLegacySoundCaption.from_sound_file(path.get_basename())
            state.guard_radio = state.radio_channel if state.radio_channel > 0 else RADIO_CHANNEL_DEFAULT
    if not position == Vector3.ZERO:
        state.direction_order = _direction_towards(driver, position, velocity)
    _orders_init(state, DriverSystem.driver_get_vehicle(driver), absf(velocity))


## The orders a timetable makes (OrdersInit(), Driver.cpp:5238-5319): start the engine, then shunt
## without a timetable, or drive it - turning where a station says `@` - and shunt after
func _orders_init(state:DriverState, vehicle:RID, velocity:float) -> void:
    _orders_clear(state)
    _order_push(state, Order.PREPARE_ENGINE)
    var entries:Array = state.timetable.get_entries()
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
    if not state.stop_here:
        # told to go: it draws up close to the next passenger stop (Driver.cpp:5305-5309)
        state.timetable.draw_up_close()
    _jump_to_first_order(state, vehicle)
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
        var vehicle:RID = DriverSystem.driver_get_vehicle(driver)
        var forward_end:RailVehicleController.CouplerEnd = (RailVehicleController.COUPLER_END_FRONT
                if state.direction > 0 else RailVehicleController.COUPLER_END_REAR)
        var behind:bool = _is_coupled_by(
                vehicle, RailVehicleController.opposite_end(forward_end), RailVehicleController.COUPLING_FLAG_COUPLER)
        var ahead:bool = _is_coupled_by(vehicle, forward_end, RailVehicleController.COUPLING_FLAG_COUPLER)
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


## Whether something is joined at the vehicle's end by every one of `flags` - the walk out through
## that end starts beyond it
static func _is_coupled_by(vehicle:RID, end:RailVehicleController.CouplerEnd, flags:int) -> bool:
    var coupled:Array[RID] = RailVehicleServer.vehicle_get_coupled(vehicle, end, flags)
    return not coupled.is_empty() and not coupled[0] == vehicle


## Whether every coupling `coupler` asks for (the original's bits) is joined at the vehicle's end;
## asked for none, it is
static func _is_coupled_as_asked(vehicle:RID, end:RailVehicleController.CouplerEnd, coupler:int) -> bool:
    var asked:int = coupler & SHUNTER_COUPLINGS
    return not asked or _is_coupled_by(vehicle, end, asked)


## The end of the trainset's vehicle at `index` towards its front
static func _end_towards_front(trainset:MaszynaLegacyDriverTrainset, index:int) -> RailVehicleController.CouplerEnd:
    if index == 0:
        return (RailVehicleController.COUPLER_END_FRONT if trainset.front_direction > 0
                else RailVehicleController.COUPLER_END_REAR)
    var vehicle:RID = trainset.vehicles[index]
    var coupled:Array[RID] = RailVehicleServer.vehicle_get_coupled(
            vehicle, RailVehicleController.COUPLER_END_FRONT, RailVehicleController.COUPLING_FLAG_COUPLER)
    var position:int = coupled.find(vehicle)
    # beyond the front end come first
    return (RailVehicleController.COUPLER_END_FRONT if position > 0 and coupled[position - 1] == trainset.vehicles[index - 1]
            else RailVehicleController.COUPLER_END_REAR)


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


func _jump_to_next_order(state:DriverState, vehicle:RID) -> void:
    var current:int = state.orders[state.order_position]
    if not current == Order.WAIT_FOR_ORDERS:
        if current & Order.CHANGE_DIRECTION and not current == Order.CHANGE_DIRECTION:
            # a change of direction on top of another order goes first
            state.orders[state.order_position] = current & ~Order.CHANGE_DIRECTION
            _order_check(state, vehicle)
            return
        state.order_position = (state.order_position + 1) % MAX_ORDERS
    _order_check(state, vehicle)


## CheckVehicles()'s lights (Driver.cpp:2451, 2510-2512): set by a driver the computer is
## (AIControllFlag) - to a player they are only hinted - once its vehicle is ready to drive
## (iEngineActive)
func _check_lights(state:DriverState, vehicle:RID) -> void:
    if DriverSystem.vehicle_is_control_active(vehicle) and state.engine_active:
        MaszynaLegacyDriverLights.check_vehicles(
                vehicle, state.direction, state.orders[state.order_position], state.light_hints)


func _jump_to_first_order(state:DriverState, vehicle:RID) -> void:
    state.order_position = 1
    state.order_top = maxi(state.order_top, 1)
    _order_check(state, vehicle)


## What a new order changes at once (OrderCheck(), Driver.cpp:5161-5185): the lights of the order
## (CheckVehicles(), Driver.cpp:5084-5092) - the doors it checks belong to the driving (TODO.md)
func _order_check(state:DriverState, vehicle:RID) -> void:
    var current:int = state.orders[state.order_position]
    if not current == Order.OBEY_TRAIN:
        state.light_hints = Vector2i(MaszynaLegacyDriverLights.NO_HINT, MaszynaLegacyDriverLights.NO_HINT)
    if current & (Order.SHUNT | Order.LOOSE_SHUNT | Order.CONNECT | Order.OBEY_TRAIN | Order.BANK):
        _check_lights(state, vehicle)
    if current & Order.CHANGE_DIRECTION:
        state.direction_order = -state.direction
    elif current == Order.OBEY_TRAIN:
        state.timetable.mind_stops()
    elif current == Order.CONNECT:
        state.timetable.pass_stops()
    elif current == Order.DISCONNECT:
        # uncoupling the locomotive: nothing stays with it
        state.vehicle_count = maxi(state.vehicle_count, 0)
    elif current == Order.WAIT_FOR_ORDERS:
        _orders_clear(state)
