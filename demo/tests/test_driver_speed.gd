extends MaszynaGutTest

## MaszynaLegacyDriverSpeed: the speed and acceleration the original's driver wants
## (pick_optimal_speed(), Driver.cpp:7297-7400), without a speed table; a vehicle ahead set by hand.

const Order = MaszynaLegacyAIDriver.Order
const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")
const REACTION:float = MaszynaLegacyAIDriver.EASY_REACTION_TIME
const SHUNT_VELOCITY:float = 25.0

var speed:MaszynaLegacyDriverSpeed
var trainset:MaszynaLegacyDriverTrainset
var route:MaszynaLegacyDriverRoute


func before_each():
    speed = MaszynaLegacyDriverSpeed.new()
    trainset = MaszynaLegacyDriverTrainset.new()
    # nothing read ahead: no next speed, no limit
    route = MaszynaLegacyDriverRoute.new()
    var vehicles:Array[RID] = [build_vehicle("DriverSpeedTest", SM42).get_rid()]
    trainset.vehicles = vehicles


func test_shunting_drives_at_the_shunting_speed():
    speed.pick(Order.SHUNT, true, false, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 0.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, SHUNT_VELOCITY)
    assert_gt(speed.acceleration_desired, 0.0, "it asks for power")


func test_a_driver_not_ready_wants_nothing():
    speed.pick(Order.SHUNT, false, false, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 0.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, 0.0)
    assert_lte(speed.acceleration_desired, MaszynaLegacyDriverSpeed.NO_ACCELERATION)


func test_told_to_stand_it_stays():
    speed.pick(Order.SHUNT, true, true, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 0.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, 0.0, "standing, told to stop here, no signal ahead")


func test_the_track_limit_holds_and_too_fast_brakes():
    # the track under the trainset allows 10, as the speed table read it
    route.velocity_limit = 10.0
    speed.pick(Order.SHUNT, true, false, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 20.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, 10.0, "the track allows 10")
    assert_lt(speed.acceleration_desired, MaszynaLegacyDriverSpeed.NO_ACCELERATION, "20 against 10: brake")


func test_a_vehicle_standing_close_ahead_stops_it():
    route.obstacle = _vehicle_ahead(20.0)
    speed.pick(Order.SHUNT, true, false, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 15.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, 0.0, "within the distance it keeps")
    assert_lt(speed.acceleration_desired, MaszynaLegacyDriverSpeed.NO_ACCELERATION, "brake")
    assert_eq(speed.reaction_time, MaszynaLegacyDriverSpeed.HURRIED_REACTION_TIME, "and look again soon")


func test_a_vehicle_far_ahead_changes_nothing():
    route.obstacle = _vehicle_ahead(900.0)
    speed.pick(Order.SHUNT, true, false, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 0.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, SHUNT_VELOCITY)
    assert_gt(speed.acceleration_desired, 0.0, "it asks for power")
    assert_eq(speed.reaction_time, REACTION)


func test_a_vehicle_running_away_is_not_minded():
    route.obstacle = _vehicle_ahead(20.0)
    route.obstacle_speed = 60.0
    speed.pick(Order.SHUNT, true, false, SHUNT_VELOCITY, SHUNT_VELOCITY, -1.0, 15.0, trainset, route, REACTION)

    assert_eq(speed.velocity_desired, SHUNT_VELOCITY)


func _vehicle_ahead(distance:float) -> VehicleNeighbour:
    var neighbour:VehicleNeighbour = VehicleNeighbour.new()
    neighbour.distance = distance
    return neighbour
