@tool
extends RefCounted
class_name MaszynaLegacyDriverRoute

## The original driver's speed table (TController::TableTraceRoute(), TableUpdate(),
## TableUpdateEvent(), TSpeedPos, Driver.cpp:190-1727): what lies on the tracks ahead of the
## trainset - the track limits, the switches, the end of the line, and the passive events the
## scenery speaks to drivers with (a `putvalues`/`getvalues` of SetVelocity, ShuntVelocity,
## OutsideStation, ...; ScenarioEventServer.event_is_passive()) - and what it makes of them: the next
## speed and how far it is (VelNext, ActualProximityDist), the speed at the next signal
## (VelSignalNext), a limit of the speed wanted, and the orders the driver gives itself when it sees
## a signal (SetVelocity, ShuntVelocity) or reaches a memory's command.
##
## The original keeps the table between updates and moves it by the distance driven; here it is
## read again on every update (RailVehicleServer.vehicle_trace_route()) and the events passed
## since the last one take effect once. The passenger stops (`PassengerStopPoint:`) are driven by
## the timetable (TableUpdateStopPoint()). Not ported yet: the section and road speeds, stopping at an
## automatic block signal (spStopOnSBL), the crossings, the turn back at the end of shunting
## (BackwardTraceRoute), the load exchange and the doors at a stop - see TODO.md, "Drivers".

## How far ahead it reads [m]: at least MIN_RANGE; moving, MOVING_RANGE past the braking distance;
## standing, STANDING_DRIVER_DISTANCES of its distance to keep (Driver.cpp:4984-4990, fDriverDist 50)
const MIN_RANGE:float = 750.0
## The vehicles ahead are looked for at least this far [m] (scan_obstacles(), Driver.cpp:6663)
const OBSTACLE_RANGE:float = 1000.0
const MOVING_RANGE:float = 400.0
const STANDING_RANGE:float = 1500.0
## Moving faster than this [km/h] (EU07_AI_MOVEMENT)
const MOVEMENT_SPEED:float = 1.0
## determine_braking_distance() (Driver.cpp:6597-6618): fDriverBraking of a train, the speed floor,
## a heavy train, the G setting's reaction
const DRIVER_BRAKING:float = 0.06
const BRAKING_SPEED_FLOOR:float = 2.0
const BRAKING_SPEED_OFFSET:float = 40.0
const HEAVY_MASS:float = 1000000.0
const HEAVY_FACTOR:float = 2.0
const DECELERATION_FACTOR:float = 25.92
const G_REACTION_FACTOR:float = 2.0
## A braking threshold past this [m/s2] takes the braking distance from the deceleration instead
const THRESHOLD_BRAKING:float = 0.05
## determine_proximity_ranges() (Driver.cpp:6684-6812) [m]: shunting 5/10 plus a vehicle each, at
## most 25/50; train 5/10 plus a vehicle each within 10-15/15-40, a goods train 10 more, standing
## 50; coupling up 2/5 and, once coupling, right up to it; uncoupling 1/10; anything else 5/10
const SHUNT_MIN_BASE:float = 5.0
const SHUNT_MIN_MAX:float = 25.0
const SHUNT_MAX_BASE:float = 10.0
const SHUNT_MAX_MAX:float = 50.0
const TRAIN_MIN_BASE:float = 5.0
const TRAIN_MIN_LOW:float = 10.0
const TRAIN_MIN_HIGH:float = 15.0
const TRAIN_MAX_BASE:float = 10.0
const TRAIN_MAX_LOW:float = 15.0
const TRAIN_MAX_HIGH:float = 40.0
const CARGO_PROXIMITY:float = 10.0
const STANDING_MAX_PROXIMITY:float = 50.0
const STANDING_SPEED:float = 0.1
const CONNECT_MIN_PROXIMITY:float = 2.0
const CONNECT_MAX_PROXIMITY:float = 5.0
const COUPLING_MIN_PROXIMITY:float = -1.0
const COUPLING_MAX_PROXIMITY:float = 0.0
const DISCONNECT_MIN_PROXIMITY:float = 1.0
const DISCONNECT_MAX_PROXIMITY:float = 10.0
const OTHER_MIN_PROXIMITY:float = 5.0
const OTHER_MAX_PROXIMITY:float = 10.0
## ... and the speed [km/h] run over a limit before braking (fVelPlus) and under it before adding
## power (fVelMinus): shunting 2 and a tenth of the shunting speed, at most 3; a train 5% of the
## speed wanted, 2-5 over and 1-5 under; coupling up 2/1, once coupling and uncoupling 1/0.5;
## anything else 2/5
const SHUNT_VELOCITY_PLUS:float = 2.0
const SHUNT_VELOCITY_MINUS_SHARE:float = 0.1
const SHUNT_VELOCITY_MINUS_MAX:float = 3.0
const TRAIN_VELOCITY_SHARE:float = 0.05
const TRAIN_VELOCITY_PLUS_LOW:float = 2.0
const TRAIN_VELOCITY_MINUS_LOW:float = 1.0
const TRAIN_VELOCITY_HIGH:float = 5.0
const CONNECT_VELOCITY_PLUS:float = 2.0
const CONNECT_VELOCITY_MINUS:float = 1.0
const CLOSE_VELOCITY_PLUS:float = 1.0
const CLOSE_VELOCITY_MINUS:float = 0.5
const OTHER_VELOCITY_PLUS:float = 2.0
const OTHER_VELOCITY_MINUS:float = 5.0
## TableUpdate() (Driver.cpp:940-960): the target's acceleration is eased towards the preferred one
## while further than this share of the braking distance
const EASING_BRAKING_SHARE:float = 1.2
## Standing, an event behind asks for this [m/s2] (Driver.cpp:988)
const BEHIND_ACCELERATION:float = -2.0
## -1 is no limit (min_speed())
const NO_LIMIT:float = -1.0
## A speed the driver takes as an order to go (TableUpdateEvent(), Driver.cpp:1680)
const GO_SPEED:float = 1.0
## Sends a memory's command only nearly standing (check_route_ahead(), Driver.cpp:8340)
const COMMAND_SPEED:float = 0.1
## TableUpdateStopPoint() (Driver.cpp:866, 1093-1380): a passenger stop is reached within this [m] of
## the front of the trainset, passed without stopping within PASSING_SHARE of it; another station's
## stop nearer than REWIND_BRAKE_SHARE of the braking distance plus REWIND_DISTANCE [m] is where the
## timetable goes on from
const PASSENGER_STOP_MAX_DISTANCE:float = 400.0
const PASSING_SHARE:float = 0.5
const REWIND_BRAKE_SHARE:float = 1.15
const REWIND_DISTANCE:float = 300.0
## No signal read yet (d_to_next_sem, Driver.cpp:877)
const NO_SIGNAL_DISTANCE:float = 10000.0
## Standing still [km/h] (Driver.cpp:921, 1080)
const STOPPED_SPEED:float = 0.01
## An odd first number of a stop keeps the train there until the way is clear (Driver.cpp:1317)
const HOLD_PARITY:int = 2
## The event lists of a track, by the way it is driven (CheckTrackEvent(), Driver.cpp:459-470)
const EVENTS_TOWARD_END:int = ScenarioEventServer.TRACK_EVENT2
const EVENTS_TOWARD_START:int = ScenarioEventServer.TRACK_EVENT1

## What an entry means (TSpeedPosFlag, Driver.h:127-149)
enum Kind { TRACK, SWITCH, LINE_END, SEMAPHORE, SHUNT_SEMAPHORE, OUTSIDE_STATION, COMMAND, STOP_POINT, OTHER }
## What a passenger stop asks of the driver's orders, in the order asked (TableUpdateStopPoint()):
## HOLD and GO set whether it waits for the way to be clear (moveStopHere); OBEY_TRAIN drives on as
## a train; TURN_THEN_TRAIN and TURN_THEN_SHUNT turn a push-pull train by its cab, then drive on;
## NEXT_ORDER takes the next order
enum StopOrder { HOLD, GO, OBEY_TRAIN, TURN_THEN_TRAIN, TURN_THEN_SHUNT, NEXT_ORDER }
## What a passenger stop is on this reading: an entry to take as it is, one to skip, or one that let
## the train go (cm_Ready)
enum StopResult { USE, SKIP, READY }

## One entry of the table (TSpeedPos)
class Entry:
    var kind:Kind
    ## from the front of the trainset [m], negative once passed
    var distance:float
    ## the speed from here on, NO_LIMIT for none (fVelNext)
    var velocity:float
    var length:float
    var event:RID
    ## the command the event carries, read live from its memory (input_command())
    var command:String
    ## the station of a passenger stop
    var station:String
    var value1:float
    var value2:float
    ## where the event stands, sent with its command
    var position:Vector3

## VelNext, ActualProximityDist
var velocity_next:float = NO_LIMIT
var proximity_distance:float = MIN_RANGE
## How far ahead it read on the last update [m]
var reach:float = MIN_RANGE
## VelSignalNext, VelSignalLast
var signal_velocity_next:float = 0.0
var signal_velocity_last:float = NO_LIMIT
## The limit TableUpdate() puts on the speed wanted (fVelDes)
var velocity_limit:float = NO_LIMIT
## fMinProximityDist, fMaxProximityDist, fBrakeDist
var min_proximity:float = OTHER_MIN_PROXIMITY
var max_proximity:float = OTHER_MAX_PROXIMITY
var brake_distance:float = 0.0
## fVelPlus, fVelMinus [km/h]
var velocity_plus:float = OTHER_VELOCITY_PLUS
var velocity_minus:float = OTHER_VELOCITY_MINUS
## The speed allowed after this update (VelSignal) and the orders it gives itself: [command,
## value1, value2, position]
var signal_velocity:float = 0.0
var commands:Array[Array] = []
## The nearest vehicle ahead (Obstacle), null for none, and its speed [km/h]
var obstacle:VehicleNeighbour = null
var obstacle_speed:float = 0.0
## Standing at its passenger stop (IsAtPassengerStop)
var at_passenger_stop:bool = false
## What its passenger stop asked of the orders on this update
var stop_orders:Array[StopOrder] = []
## The passenger stops done with, by event, until it has left them (TSpeedPos::iFlags = 0)
var _stops_done:Dictionary[RID, bool] = {}
## How far a passenger stop was brought forward for the train's length and the platform, by event
## (TSpeedPos::fMoved)
var _stops_moved:Dictionary[RID, float] = {}
## The next station's stop was read on this update (IsScheduledPassengerStopVisible)
var _scheduled_stop_visible:bool = false
## The memories' commands it sent, by event, not to send them again until they change
## (StopCommandSent(), MemCell.cpp:196-205)
var _sent:Dictionary[RID, String] = {}
## The events ahead on the last update, to take the passed ones once
var _ahead:Dictionary[RID, Entry] = {}


## One reading of the tracks ahead (TableCheck(), TableUpdate(), check_route_ahead()) for the driver
## of `vehicle`: `allowed` is the speed allowed now (VelSignal), `speed` the trainset's along the way
## it drives [km/h], `acceleration` the one preferred (AccPreferred) [m/s2]; `timetable` how far it
## got, at `hours` of the day; `shunt_velocity` and `velocity_desired` the driver's [km/h], `coupling`
## whether it is coupling up now (moveConnect). The speed allowed afterwards is `signal_velocity`, the
## orders it gives itself `commands` and `stop_orders`.
func update(
    vehicle:RID, order:int, stop_here:bool, allowed:float, speed:float, acceleration:float, velocity_max:float,
    trainset:MaszynaLegacyDriverTrainset, timetable:MaszynaLegacyDriverTimetable, hours:float,
    shunt_velocity:float, velocity_desired:float, coupling:bool
) -> void:
    commands.clear()
    stop_orders.clear()
    at_passenger_stop = false
    _scheduled_stop_visible = false
    # IsCargoTrain (Driver.cpp:2303): keeps further from a stop, leaves a passenger stop at once
    var cargo:bool = int(RailVehicleServer.vehicle_dump_state(vehicle).get("brake_delay_setting", 0)) \
            == MaszynaLegacyDriverBraking.DELAY_SETTING_G
    _determine_distances(vehicle, order, speed, trainset, shunt_velocity, velocity_desired, coupling, cargo)
    reach = maxf(MIN_RANGE, MOVING_RANGE + brake_distance if absf(speed) > MOVEMENT_SPEED else STANDING_RANGE)
    var entries:Array[Entry] = _read(reach, trainset)
    # the passenger stops left behind are forgotten
    var read:Dictionary[RID, bool] = {}
    for entry:Entry in entries:
        if entry.kind == Kind.STOP_POINT:
            read[entry.event] = true
    for event:RID in _stops_done.keys():
        if not read.has(event):
            _stops_done.erase(event)
    for event:RID in _stops_moved.keys():
        if not read.has(event):
            _stops_moved.erase(event)
    var signal_distance:float = NO_SIGNAL_DISTANCE
    var obey_train:bool = order & MaszynaLegacyAIDriver.Order.OBEY_TRAIN
    # the events passed since the last update take effect once (TableUpdateEvent(), fDist < 0)
    var seen:Dictionary[RID, Entry] = {}
    for entry:Entry in entries:
        if entry.event.is_valid() and entry.distance > 0.0:
            seen[entry.event] = entry
    for event:RID in _ahead:
        if not seen.has(event):
            allowed = _pass(_ahead[event], obey_train, allowed)
    _ahead = seen

    velocity_next = NO_LIMIT
    proximity_distance = reach
    velocity_limit = NO_LIMIT
    var best_acceleration:float = acceleration
    var signal_found:bool = false
    var go:String = ""
    var command_entry:Entry = null
    for entry:Entry in entries:
        if entry.kind == Kind.STOP_POINT:
            var result:StopResult = _update_stop_point(entry, order, absf(speed), cargo, trainset, timetable, hours, signal_distance)
            if result == StopResult.READY and go.is_empty():
                go = "Ready"
            if not result == StopResult.USE:
                continue
        var velocity:float = entry.velocity
        var distance:float = entry.distance
        if entry.event.is_valid():
            if distance > 0.0:
                if _is_proper_semaphore(entry.kind, obey_train):
                    signal_distance = minf(distance, signal_distance)
                if _is_proper_semaphore(entry.kind, obey_train) and not signal_found:
                    # the nearest signal (TableUpdateEvent(), Driver.cpp:1585-1602)
                    signal_found = true
                    signal_velocity_next = entry.velocity
                    if velocity < 0.0:
                        velocity = velocity_max
                        allowed = velocity_max
                match entry.kind:
                    Kind.OUTSIDE_STATION:
                        # a train goes on past it; shunting ends there
                        velocity = NO_LIMIT if obey_train else 0.0
                    Kind.SHUNT_SEMAPHORE:
                        if obey_train and velocity == 0.0:
                            velocity = NO_LIMIT
                        elif not velocity == 0.0 and go.is_empty():
                            go = "ShuntVelocity"
                            if allowed == 0.0:
                                allowed = velocity
                    Kind.SEMAPHORE:
                        if (velocity < 0.0 or velocity >= GO_SPEED) and go.is_empty():
                            go = "SetVelocity"
                            if allowed == 0.0:
                                allowed = NO_LIMIT
                    Kind.COMMAND:
                        # a memory's command for a standing driver, sent once (cm_Command, Driver.cpp:1708-1723)
                        if go.is_empty() and not _sent.get(entry.event, "") == _signature(entry) \
                                and (stop_here or distance <= max_proximity):
                            go = "Command"
                            command_entry = entry
                    Kind.OTHER:
                        continue
            elif entry.kind == Kind.OTHER or entry.kind == Kind.COMMAND:
                continue
        var line_end:bool = entry.kind == Kind.LINE_END
        if velocity < 0.0 and not line_end:
            continue
        var wanted:float = acceleration
        if distance > 0.0:
            if velocity >= 0.0:
                wanted = (velocity * velocity - speed * speed) / (DECELERATION_FACTOR * distance)
                if speed < velocity or velocity == 0.0:
                    # plenty of room to brake: keep the preferred acceleration, easing into braking
                    var braking:float = EASING_BRAKING_SHARE * brake_distance
                    if braking > 0.0:
                        wanted = lerpf(wanted, acceleration, clampf((distance - braking) / braking, 0.0, 1.0))
                if distance < min_proximity:
                    velocity_limit = MaszynaLegacyDriverSpeed.min_speed(velocity_limit, velocity)
        elif entry.event.is_valid():
            # an event behind holds only a stop (Driver.cpp:984-990)
            wanted = acceleration if velocity > 0.0 else (0.0 if absf(speed) < COMMAND_SPEED else BEHIND_ACCELERATION)
        else:
            # a track the trainset is still on: its limit holds until it has left it
            if velocity >= GO_SPEED and distance + entry.length < -trainset.length and not line_end:
                continue
            velocity_limit = MaszynaLegacyDriverSpeed.min_speed(velocity_limit, velocity)
            if not line_end:
                continue
        if line_end:
            # the end of the line is a stop of its own (Driver.cpp:991-1001)
            var stopping:float = -speed * speed / (DECELERATION_FACTOR * maxf(distance + entry.length, COMMAND_SPEED))
            if stopping < wanted:
                wanted = stopping
                velocity = 0.0
                distance += entry.length
                if distance < min_proximity:
                    velocity_limit = MaszynaLegacyDriverSpeed.min_speed(velocity_limit, 0.0)
        if wanted <= best_acceleration and (velocity < velocity_next or velocity_next < 0.0):
            best_acceleration = wanted
            velocity_next = velocity
            proximity_distance = distance
        elif wanted > 0.0 and wanted <= best_acceleration and velocity >= 0.0 and velocity_next < 0.0:
            best_acceleration = wanted
            velocity_next = velocity
            proximity_distance = distance
        if velocity_next == 0.0:
            break
    # no signal ahead any more: the last one's speed is forgotten on the line (Driver.cpp:1030-1034)
    if obey_train and not signal_found:
        signal_velocity_last = NO_LIMIT
    # standing at its passenger stop, it holds there (Driver.cpp:1080-1082)
    if at_passenger_stop and absf(speed) < STOPPED_SPEED:
        velocity_limit = 0.0
    else:
        velocity_limit = MaszynaLegacyDriverSpeed.min_speed(velocity_limit, signal_velocity_last)
    # a stop let it go: on at once, unless something ahead holds it (check_route_ahead(), cm_Ready)
    if go == "Ready" and not velocity_next == 0.0 and timetable.stop_closer:
        allowed = NO_LIMIT
    signal_velocity = allowed
    # the orders it gives itself (check_route_ahead(), Driver.cpp:8302-8356)
    match go:
        "SetVelocity":
            var takes_speed:int = (MaszynaLegacyAIDriver.Order.SHUNT | MaszynaLegacyAIDriver.Order.LOOSE_SHUNT
                    | MaszynaLegacyAIDriver.Order.OBEY_TRAIN | MaszynaLegacyAIDriver.Order.BANK)
            if absf(allowed) >= GO_SPEED and (order == MaszynaLegacyAIDriver.Order.WAIT_FOR_ORDERS or order & takes_speed):
                commands.append(["SetVelocity", allowed, velocity_next, Vector3.ZERO])
        "ShuntVelocity":
            commands.append(["ShuntVelocity", allowed, velocity_next, Vector3.ZERO])
        "Command":
            if absf(speed) < COMMAND_SPEED:
                commands.append([command_entry.command, command_entry.value1, command_entry.value2, command_entry.position])
                _sent[command_entry.event] = _signature(command_entry)
    # the vehicles ahead, from the front of the trainset the way it drives (scan_obstacles(),
    # Driver.cpp:6638-6680)
    obstacle = null
    obstacle_speed = 0.0
    if trainset.vehicles:
        obstacle = RailVehicleServer.vehicle_find_vehicle(
                trainset.vehicles[0],
                MaszynaLegacyDriverTrainset.FRONT_END if trainset.front_direction > 0 else MaszynaLegacyDriverTrainset.REAR_END,
                maxf(OBSTACLE_RANGE, reach))
    if obstacle:
        obstacle_speed = RailVehicleServer.vehicle_get_speed(obstacle.vehicle_rid)


## The entries of the tracks ahead, nearest first (TableTraceRoute(), Driver.cpp:589-779)
func _read(reach:float, trainset:MaszynaLegacyDriverTrainset) -> Array[Entry]:
    var entries:Array[Entry] = []
    if trainset.vehicles.is_empty():
        return entries
    var front:RID = trainset.vehicles[0]
    # the placement is the vehicle's middle; the table counts from the trainset's front
    var front_offset:float = float(RailVehicleServer.vehicle_dump_config(front).get("length", 0.0)) / 2.0
    var last_velocity:float = NO_LIMIT - 1.0
    for segment:TrackRouteSegment in RailVehicleServer.vehicle_trace_route(front, trainset.front_direction, reach + front_offset):
        var start:float = segment.distance - front_offset
        # the events first, as the track is entered
        var slot:int = EVENTS_TOWARD_END if segment.toward_end else EVENTS_TOWARD_START
        for event:RID in ScenarioEventServer.track_get_events(segment.track_rid, slot):
            if ScenarioEventServer.event_is_passive(event):
                var entry:Entry = _event_entry(event, segment, start)
                if entry:
                    entries.append(entry)
        if segment.track_switch or segment.velocity == 0.0 or not segment.velocity == last_velocity or segment.line_end:
            var entry:Entry = Entry.new()
            entry.kind = Kind.LINE_END if segment.line_end else (Kind.SWITCH if segment.track_switch else Kind.TRACK)
            entry.distance = start
            entry.velocity = segment.velocity
            entry.length = segment.length
            entries.append(entry)
        last_velocity = segment.velocity
    entries.sort_custom(func(a:Entry, b:Entry) -> bool: return a.distance < b.distance)
    return entries


## A passive event as an entry (TSpeedPos::Set(), CommandCheck(), Driver.cpp:203-290, 390-419): its
## command read live, placed where its position meets the track (GetDistanceToEvent())
func _event_entry(event:RID, segment:TrackRouteSegment, start:float) -> Entry:
    var action:MaszynaLegacyVehicleCommandAction = ScenarioEventServer.event_get_action(event) as MaszynaLegacyVehicleCommandAction
    if not action:
        return null
    var entry:Entry = Entry.new()
    entry.event = event
    if action.source.is_valid():
        entry.command = ScenarioEventServer.memory_get_text(action.source)
        entry.value1 = ScenarioEventServer.memory_get_value1(action.source)
        entry.value2 = ScenarioEventServer.memory_get_value2(action.source)
    else:
        entry.command = action.command
        entry.value1 = action.value1
        entry.value2 = action.value2
    var curve:Curve3D = TrackManager.track_get_domain_curve(segment.track_rid)
    var along:float = curve.get_closest_offset(action.position) if curve else 0.0
    entry.distance = start + (along if segment.toward_end else segment.length - along)
    entry.position = action.position
    match entry.command:
        "ShuntVelocity":
            entry.kind = Kind.SHUNT_SEMAPHORE
            entry.velocity = entry.value1
        "SetVelocity":
            entry.kind = Kind.SEMAPHORE
            entry.velocity = entry.value1
        "OutsideStation":
            entry.kind = Kind.OUTSIDE_STATION
            entry.velocity = NO_LIMIT
        "SetProximityVelocity", "RoadVelocity", "SectionVelocity", "CabSignal", "Emergency_brake":
            entry.kind = Kind.OTHER
            entry.velocity = NO_LIMIT
        _:
            if entry.command.begins_with(MaszynaLegacyEventFactory.PASSENGER_STOP_POINT):
                # a stop, until the timetable says otherwise (TSpeedPos::Set(), Driver.cpp:242-246)
                entry.kind = Kind.STOP_POINT
                entry.station = entry.command.trim_prefix(MaszynaLegacyEventFactory.PASSENGER_STOP_POINT)
                entry.velocity = 0.0
            else:
                # any other text is a command for a standing driver: it stops there for it
                entry.kind = Kind.COMMAND
                entry.velocity = 0.0
    return entry


## A passenger stop on this reading (TableUpdateStopPoint(), Driver.cpp:1093-1380): another station's
## is skipped or the timetable goes on from it; the next station's is passed at speed where the train
## does not stop, else brought forward for the train and the platform, stopped at, and left at the
## departure time - or turned at, or the end of the timetable. `speed` is the vehicle's [km/h].
func _update_stop_point(
    entry:Entry, order:int, speed:float, cargo:bool, trainset:MaszynaLegacyDriverTrainset,
    timetable:MaszynaLegacyDriverTimetable, hours:float, signal_distance:float
) -> StopResult:
    var driving:int = (MaszynaLegacyAIDriver.Order.SHUNT | MaszynaLegacyAIDriver.Order.LOOSE_SHUNT
            | MaszynaLegacyAIDriver.Order.OBEY_TRAIN | MaszynaLegacyAIDriver.Order.BANK)
    if not order & driving or _stops_done.has(entry.event):
        return StopResult.SKIP
    if not entry.station.to_lower() == timetable.next_stop.to_lower():
        if not _scheduled_stop_visible and entry.distance > 0.0 \
                and entry.distance < REWIND_BRAKE_SHARE * brake_distance + REWIND_DISTANCE:
            timetable.rewind(entry.station)
        return StopResult.SKIP
    if not timetable.stop_point:
        # coupling up or turning: it drives past
        _stops_done[entry.event] = true
        return StopResult.SKIP
    _scheduled_stop_visible = true
    if not timetable.is_stop():
        # passed at speed, taken as reached a little before it
        entry.velocity = NO_LIMIT
        if entry.distance < PASSENGER_STOP_MAX_DISTANCE * PASSING_SHARE:
            timetable.arrive(hours)
            timetable.advance()
            _stops_done[entry.event] = true
            return StopResult.SKIP
        return StopResult.USE
    if not _stops_moved.has(entry.event):
        # the first number: negative, where the front stops before it; positive, where the middle
        # stops; the second: the platform's length, or negative - only for trains shorter than that
        var place:float = entry.value1
        var platform:float = entry.value2
        if platform < 0.0 and trainset.length >= -platform:
            _stops_done[entry.event] = true
            return StopResult.SKIP
        var moved:float = -place if place < 0.0 else place - min_proximity - trainset.length / 2.0
        _stops_moved[entry.event] = maxf(0.0, minf(moved, absf(platform) - min_proximity - trainset.length))
    var shift:float = _stops_moved[entry.event]
    entry.distance -= shift
    at_passenger_stop = entry.distance <= PASSENGER_STOP_MAX_DISTANCE and (
            entry.distance + trainset.length + shift - min_proximity / 2.0
                    <= maxf(absf(entry.value2), 2.0 * max_proximity + trainset.length)
            if timetable.stop_closer else entry.distance < signal_distance)
    if speed > MOVEMENT_SPEED:
        return StopResult.USE
    if not at_passenger_stop:
        # standing short of it: let it draw up closer
        entry.velocity = NO_LIMIT
        return StopResult.USE
    if timetable.arrive(hours) and timetable.turns_here():
        # `@`: a push-pull train turns by its cab and stays, a locomotive goes on to its next order
        if trainset.push_pull:
            stop_orders.append(StopOrder.HOLD)
            stop_orders.append(StopOrder.TURN_THEN_SHUNT if timetable.is_last_station() else StopOrder.TURN_THEN_TRAIN)
        else:
            timetable.pass_stops()
            stop_orders.append(StopOrder.GO)
        stop_orders.append(StopOrder.NEXT_ORDER)
        timetable.stop_short()
        _stops_done[entry.event] = true
        return StopResult.SKIP
    if order & (MaszynaLegacyAIDriver.Order.SHUNT | MaszynaLegacyAIDriver.Order.LOOSE_SHUNT):
        stop_orders.append(StopOrder.OBEY_TRAIN)
    if timetable.is_last_station():
        # the end of the timetable: its next order, and it stays until told to go
        timetable.finish()
        timetable.stop_short()
        if not trainset.push_pull:
            timetable.pass_stops()
        stop_orders.append(StopOrder.NEXT_ORDER)
        stop_orders.append(StopOrder.HOLD)
        _stops_done[entry.event] = true
        return StopResult.SKIP
    if cargo or timetable.is_time_to_go(hours):
        at_passenger_stop = false
        timetable.advance()
        stop_orders.append(StopOrder.HOLD if floori(absf(entry.value1)) % HOLD_PARITY else StopOrder.GO)
        timetable.draw_up_close()
        _stops_done[entry.event] = true
        return StopResult.READY
    # waiting for the departure time
    return StopResult.USE


## An event passed (TableUpdateEvent(), fDist < 0, Driver.cpp:1535-1700); returns the speed allowed
## from here
func _pass(entry:Entry, obey_train:bool, allowed:float) -> float:
    if _is_proper_semaphore(entry.kind, obey_train):
        signal_velocity_last = entry.velocity
    match entry.kind:
        Kind.SHUNT_SEMAPHORE:
            if not entry.velocity == 0.0 and not (obey_train and entry.velocity == 0.0):
                return entry.velocity
        Kind.SEMAPHORE:
            if entry.velocity < 0.0 or entry.velocity >= GO_SPEED:
                return NO_LIMIT
    return allowed


## A memory's command, as far as sending it again goes
static func _signature(entry:Entry) -> String:
    return "%s %s %s" % [entry.command, entry.value1, entry.value2]


## IsProperSemaphor() (Driver.cpp:373-388)
func _is_proper_semaphore(kind:Kind, obey_train:bool) -> bool:
    if obey_train:
        return kind == Kind.SEMAPHORE
    return kind == Kind.SEMAPHORE or kind == Kind.SHUNT_SEMAPHORE or kind == Kind.OUTSIDE_STATION


## determine_braking_distance(), determine_proximity_ranges() (Driver.cpp:6597-6812); the original's
## margins for modern vehicles and for the weather are not ported (TODO.md)
func _determine_distances(
    vehicle:RID, order:int, speed:float, trainset:MaszynaLegacyDriverTrainset, shunt_velocity:float,
    velocity_desired:float, coupling:bool, cargo:bool
) -> void:
    var velocity_ceiling:float = maxf(BRAKING_SPEED_FLOOR, ceilf(absf(speed)))
    brake_distance = DRIVER_BRAKING * velocity_ceiling * (BRAKING_SPEED_OFFSET + velocity_ceiling)
    if trainset.mass > HEAVY_MASS:
        brake_distance *= HEAVY_FACTOR
    if -MaszynaLegacyDriverSpeed.ACCELERATION_THRESHOLD > THRESHOLD_BRAKING:
        brake_distance = velocity_ceiling * velocity_ceiling / DECELERATION_FACTOR / -MaszynaLegacyDriverSpeed.ACCELERATION_THRESHOLD
    # the G setting brakes later: its reaction on top
    if int(RailVehicleServer.vehicle_dump_state(vehicle).get("brake_delay_setting", 0)) == MaszynaLegacyDriverBraking.DELAY_SETTING_G:
        brake_distance += G_REACTION_FACTOR * velocity_ceiling
    var vehicles:float = trainset.vehicles.size()
    match order:
        MaszynaLegacyAIDriver.Order.CONNECT:
            if coupling:
                # stood close without a collision: right up to it
                _set_ranges(COUPLING_MIN_PROXIMITY, COUPLING_MAX_PROXIMITY, CLOSE_VELOCITY_PLUS, CLOSE_VELOCITY_MINUS)
            else:
                _set_ranges(CONNECT_MIN_PROXIMITY, CONNECT_MAX_PROXIMITY, CONNECT_VELOCITY_PLUS, CONNECT_VELOCITY_MINUS)
        MaszynaLegacyAIDriver.Order.DISCONNECT:
            _set_ranges(DISCONNECT_MIN_PROXIMITY, DISCONNECT_MAX_PROXIMITY, CLOSE_VELOCITY_PLUS, CLOSE_VELOCITY_MINUS)
        MaszynaLegacyAIDriver.Order.SHUNT:
            _set_ranges(minf(SHUNT_MIN_BASE + vehicles, SHUNT_MIN_MAX), minf(SHUNT_MAX_BASE + vehicles, SHUNT_MAX_MAX),
                    SHUNT_VELOCITY_PLUS, minf(SHUNT_VELOCITY_MINUS_SHARE * shunt_velocity, SHUNT_VELOCITY_MINUS_MAX))
        MaszynaLegacyAIDriver.Order.LOOSE_SHUNT:
            _set_ranges(COUPLING_MIN_PROXIMITY, COUPLING_MAX_PROXIMITY, SHUNT_VELOCITY_PLUS, CLOSE_VELOCITY_MINUS)
        MaszynaLegacyAIDriver.Order.OBEY_TRAIN:
            var extra:float = CARGO_PROXIMITY if cargo else 0.0
            _set_ranges(clampf(TRAIN_MIN_BASE + vehicles, TRAIN_MIN_LOW, TRAIN_MIN_HIGH) + extra,
                    clampf(TRAIN_MAX_BASE + vehicles, TRAIN_MAX_LOW, TRAIN_MAX_HIGH) + extra,
                    clampf(ceilf(TRAIN_VELOCITY_SHARE * velocity_desired), TRAIN_VELOCITY_PLUS_LOW, TRAIN_VELOCITY_HIGH),
                    clampf(roundf(TRAIN_VELOCITY_SHARE * velocity_desired), TRAIN_VELOCITY_MINUS_LOW, TRAIN_VELOCITY_HIGH))
            if absf(speed) < STANDING_SPEED:
                # stood too far: it does not draw up the last metres
                max_proximity = STANDING_MAX_PROXIMITY
        MaszynaLegacyAIDriver.Order.BANK:
            # the original leaves them as they were (Driver.cpp:6803-6806)
            pass
        _:
            _set_ranges(OTHER_MIN_PROXIMITY, OTHER_MAX_PROXIMITY, OTHER_VELOCITY_PLUS, OTHER_VELOCITY_MINUS)


func _set_ranges(minimum:float, maximum:float, plus:float, minus:float) -> void:
    min_proximity = minimum
    max_proximity = maximum
    velocity_plus = plus
    velocity_minus = minus

