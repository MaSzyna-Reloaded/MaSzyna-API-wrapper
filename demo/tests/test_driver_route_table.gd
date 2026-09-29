extends MaszynaGutTest

## MaszynaLegacyDriverRoute's table kept between updates, as the original's (TableCheck(),
## TableTraceRoute()): what was traced stays until passed, and a switch thrown ahead is traced again
## from - read from tracks built here.

const Order = MaszynaLegacyAIDriver.Order
const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")
const LINE_VELOCITY:float = 100.0
const RESTRICTED_VELOCITY:float = 40.0
const DIVERGING_VELOCITY:float = 20.0
const SHUNT_SPEED:float = 25.0
## Standing the driver reads 1500 m ahead, moving at this speed [km/h] far less (MOVING_RANGE plus the
## braking distance)
const MOVING_SPEED:float = 50.0
## Where the limit starts on either side of the vehicle's middle [m]: inside the standing reading,
## beyond the moving one
const LIMIT_DISTANCE:float = 1200.0
const LINE_LENGTH:float = 4000.0
## The layout with a switch: the track the vehicle stands on, the switch's length and how far its
## diverging branch ends to the side [m]
const APPROACH_LENGTH:float = 200.0
const SWITCH_LENGTH:float = 30.0
const DIVERGING_OFFSET:float = 10.0

var _tracks:Array[RID] = []
## Freed before the nodes they are driven by (autofree), as test_rail_vehicle_track_movement.gd does
var _vehicles:Array[RailVehicle3D] = []


func after_each() -> void:
    for vehicle:RailVehicle3D in _vehicles:
        remove_child(vehicle)
        vehicle.queue_free()
    _vehicles.clear()
    for track:RID in _tracks:
        TrackServer.track_free(track)
    _tracks.clear()
    TrackServer.topology_rebuild()


func test_a_limit_traced_standing_stays_when_the_reach_shrinks_moving_off() -> void:
    _track(Vector3(-LINE_LENGTH, 0.0, 0.0), Vector3(-LIMIT_DISTANCE, 0.0, 0.0), null, RESTRICTED_VELOCITY, "")
    _track(Vector3(-LIMIT_DISTANCE, 0.0, 0.0), Vector3(LIMIT_DISTANCE, 0.0, 0.0), null, LINE_VELOCITY, "line")
    _track(Vector3(LIMIT_DISTANCE, 0.0, 0.0), Vector3(LINE_LENGTH, 0.0, 0.0), null, RESTRICTED_VELOCITY, "")
    TrackServer.topology_rebuild()
    var vehicle:RID = await _place("line", LIMIT_DISTANCE)
    var trainset:MaszynaLegacyDriverTrainset = _trainset(vehicle, 1)
    var route:MaszynaLegacyDriverRoute = MaszynaLegacyDriverRoute.new()

    _update(route, vehicle, trainset, 0.0)
    assert_eq(route.velocity_next, RESTRICTED_VELOCITY, "standing, the limit is read")
    _update(route, vehicle, trainset, MOVING_SPEED)

    assert_lt(route.reach, LIMIT_DISTANCE, "moving, the reach no longer covers it")
    assert_eq(route.velocity_next, RESTRICTED_VELOCITY, "but what was traced stays until passed")


func test_a_switch_thrown_ahead_is_traced_again_from() -> void:
    _track(Vector3(-APPROACH_LENGTH, 0.0, 0.0), Vector3.ZERO, null, LINE_VELOCITY, "approach")
    var switch_track:RID = _track(Vector3.ZERO, Vector3(SWITCH_LENGTH, 0.0, 0.0),
            _curve(Vector3.ZERO, Vector3(SWITCH_LENGTH, 0.0, DIVERGING_OFFSET)), LINE_VELOCITY, "", TrackServer.TRACK_SWITCH)
    _track(Vector3(SWITCH_LENGTH, 0.0, 0.0), Vector3(LINE_LENGTH, 0.0, 0.0), null, LINE_VELOCITY, "")
    _track(Vector3(SWITCH_LENGTH, 0.0, DIVERGING_OFFSET), Vector3(LINE_LENGTH, 0.0, LINE_LENGTH), null,
            DIVERGING_VELOCITY, "")
    TrackServer.switch_set_active_track(switch_track, TrackServer.TRACK_COMMON)
    TrackServer.topology_rebuild()
    var vehicle:RID = await _place("approach", APPROACH_LENGTH / 2.0)
    # the way that leads over the switch
    var toward_switch:int = 1
    for segment:TrackRouteSegment in RailVehicleServer.vehicle_trace_route(vehicle, -1, APPROACH_LENGTH):
        if segment.track_rid == switch_track:
            toward_switch = -1
    var trainset:MaszynaLegacyDriverTrainset = _trainset(vehicle, toward_switch)
    var route:MaszynaLegacyDriverRoute = MaszynaLegacyDriverRoute.new()

    _update(route, vehicle, trainset, 0.0)
    assert_eq(route.velocity_next, MaszynaLegacyDriverRoute.NO_LIMIT, "set straight, nothing limits it")
    TrackServer.switch_set_active_track(switch_track, TrackServer.TRACK_DIVERGING)
    _update(route, vehicle, trainset, 0.0)

    assert_eq(route.velocity_next, DIVERGING_VELOCITY, "thrown, the diverging track's limit is read")


func _curve(from:Vector3, to:Vector3) -> TrackCurve:
    var curve:TrackCurve = TrackCurve.new()
    curve.p1 = from
    curve.p2 = to
    return curve


func _track(from:Vector3, to:Vector3, diverging:TrackCurve, velocity:float, name:String,
        type:int = TrackServer.TRACK_NORMAL) -> RID:
    var track:RID = TrackServer.track_create()
    _tracks.append(track)
    TrackServer.track_update_curves(track, _curve(from, to), diverging)
    TrackServer.track_update(track, type, name, 1.435)
    TrackServer.track_set_velocity(track, velocity)
    return track


## A standing SM42 on the named track
func _place(track_name:String, offset:float) -> RID:
    var physics_node:VehiclePhysicsNode = build_vehicle_node("RouteTableTest", SM42)
    var vehicle:RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = track_name
    vehicle.start_track_offset = offset
    add_child(vehicle)
    _vehicles.append(vehicle)
    vehicle.controller_path = vehicle.get_path_to(physics_node)
    await wait_idle_frames(2)
    return vehicle.get_rid()


func _trainset(vehicle:RID, direction:int) -> MaszynaLegacyDriverTrainset:
    var trainset:MaszynaLegacyDriverTrainset = MaszynaLegacyDriverTrainset.new()
    trainset.update(vehicle, direction, true)
    return trainset


## One update of the route, shunting and wanting LINE_VELOCITY, at `speed` [km/h]
func _update(route:MaszynaLegacyDriverRoute, vehicle:RID, trainset:MaszynaLegacyDriverTrainset, speed:float) -> void:
    route.update(vehicle, Order.SHUNT, false, SHUNT_SPEED, speed, MaszynaLegacyDriverSpeed.EASY_ACCELERATION,
            MaszynaLegacyDriverSpeed.NO_LIMIT, trainset, MaszynaLegacyDriverTimetable.new(), 0.0, SHUNT_SPEED,
            LINE_VELOCITY, false, MaszynaLegacyDriverBraking.new())
