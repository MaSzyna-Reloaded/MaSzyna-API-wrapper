extends MaszynaGutTest

## StationServer: a train's dispatch at a stop, one step after another, each over when the car
## reports it - its passengers exchanged, its doors closed - on a car built in the test.

const TRACK_NAME:String = "station_server_test"
const TRACK_LENGTH:float = 400.0
const TRACK_OFFSET:float = 100.0
const CAPACITY:float = 100.0
## Passengers a second through one open side
const EXCHANGE_SPEED:float = 5.0
const BOARDING:float = 10.0
const PASSENGERS:String = MaszynaLegacyStation.PASSENGERS
## Long enough for the doors to open and the exchange to be done, and for the doors to close [s]
const EXCHANGE_TIMEOUT:float = 20.0
const SETTLE_FRAMES:int = 2

var _track:RID
var _car:RailVehicle3D
var _vehicle:RID
var _cars:Array[RID] = []


func before_each() -> void:
    _track = build_track(TRACK_NAME, TRACK_LENGTH)
    _car = build_passenger_car("StationServerTest", TRACK_NAME, TRACK_OFFSET, CAPACITY, EXCHANGE_SPEED)
    await wait_idle_frames(SETTLE_FRAMES)
    _vehicle = _car.get_rid()
    _cars = [_vehicle]


func after_each() -> void:
    StationServer.dispatch_cancel(_vehicle)
    free_rail_vehicle(_car)
    TrackServer.track_free(_track)
    TrackServer.topology_rebuild()


func test_without_an_exchange_the_dispatch_waits_for_the_departure() -> void:
    StationServer.dispatch_start(_vehicle, _cars)
    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_WAIT_DEPARTURE)

    watch_signals(StationServer)
    StationServer.dispatch_depart(_vehicle)

    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_NONE, "the doors are closed")
    assert_signal_emitted_with_parameters(StationServer, "dispatch_finished", [_vehicle])


func test_the_train_waits_for_its_passengers_then_for_its_doors() -> void:
    RailVehicleServer.load_add(_vehicle, BOARDING, RailVehicleLoad.PLATFORM_SIDE_LEFT, PASSENGERS)
    StationServer.dispatch_start(_vehicle, _cars)
    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_EXCHANGE)
    assert_gt(StationServer.dispatch_get_exchange_time(_vehicle), 0.0)

    await wait_until(func() -> bool:
        return not StationServer.dispatch_get_step(_vehicle) == StationServer.DISPATCH_STEP_EXCHANGE, EXCHANGE_TIMEOUT)

    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_WAIT_DEPARTURE,
            "exchanged, not let go yet")
    # the car's doors held open, whatever its passengers did
    VehicleServer.vehicle_send_command(_vehicle, "doors_left_local", true)
    await wait_until(func() -> bool:
        return VehicleServer.vehicle_dump_state(_vehicle).get("doors_left_open", false), EXCHANGE_TIMEOUT)
    StationServer.dispatch_depart(_vehicle)
    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_CLOSE_DOORS)

    VehicleServer.vehicle_send_command(_vehicle, "doors_left_local", false)
    await wait_for_signal(StationServer.dispatch_finished, EXCHANGE_TIMEOUT)

    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_NONE)


func test_let_go_during_the_exchange_it_closes_the_doors_once_done() -> void:
    RailVehicleServer.load_add(_vehicle, BOARDING, RailVehicleLoad.PLATFORM_SIDE_LEFT, PASSENGERS)
    StationServer.dispatch_start(_vehicle, _cars)
    StationServer.dispatch_depart(_vehicle)
    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_EXCHANGE)

    await wait_until(func() -> bool:
        return not StationServer.dispatch_get_step(_vehicle) == StationServer.DISPATCH_STEP_EXCHANGE, EXCHANGE_TIMEOUT)

    assert_ne(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_WAIT_DEPARTURE,
            "no wait for a departure given already")


func test_a_cancelled_dispatch_is_gone() -> void:
    RailVehicleServer.load_add(_vehicle, BOARDING, RailVehicleLoad.PLATFORM_SIDE_LEFT, PASSENGERS)
    StationServer.dispatch_start(_vehicle, _cars)

    StationServer.dispatch_cancel(_vehicle)

    assert_eq(StationServer.dispatch_get_step(_vehicle), StationServer.DISPATCH_STEP_NONE)
    assert_eq(StationServer.dispatch_get_exchange_time(_vehicle), 0.0)
