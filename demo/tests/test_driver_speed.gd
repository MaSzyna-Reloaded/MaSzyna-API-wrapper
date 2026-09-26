extends MaszynaGutTest

## MaszynaLegacyDriverSpeed: the speed and acceleration the original's driver wants
## (pick_optimal_speed(), Driver.cpp:7297-7400), without a speed table.

const Order = MaszynaLegacyAIDriver.Order
const SM42:VehicleModel = preload("res://tests/fixtures/sm42_vehicle.tres")
const SHUNT_VELOCITY:float = 25.0

var speed:MaszynaLegacyDriverSpeed
var trainset:MaszynaLegacyDriverTrainset


func before_each():
    speed = MaszynaLegacyDriverSpeed.new()
    trainset = MaszynaLegacyDriverTrainset.new()
    var vehicles:Array[RID] = [build_vehicle("DriverSpeedTest", SM42).get_rid()]
    trainset.vehicles = vehicles


func test_shunting_drives_at_the_shunting_speed():
    speed.pick(Order.SHUNT, true, false, 0.0, SHUNT_VELOCITY, -1.0, -1.0, 0.0, trainset)

    assert_eq(speed.velocity_desired, SHUNT_VELOCITY)
    assert_gt(speed.acceleration_desired, 0.0, "it asks for power")


func test_a_driver_not_ready_wants_nothing():
    speed.pick(Order.SHUNT, false, false, 0.0, SHUNT_VELOCITY, -1.0, -1.0, 0.0, trainset)

    assert_eq(speed.velocity_desired, 0.0)
    assert_lte(speed.acceleration_desired, MaszynaLegacyDriverSpeed.NO_ACCELERATION)


func test_told_to_stand_it_stays():
    speed.pick(Order.SHUNT, true, true, 0.0, SHUNT_VELOCITY, -1.0, -1.0, 0.0, trainset)

    assert_eq(speed.velocity_desired, 0.0, "standing, told to stop here, no signal ahead")


func test_the_track_limit_holds_and_too_fast_brakes():
    speed.pick(Order.SHUNT, true, false, 0.0, SHUNT_VELOCITY, -1.0, 10.0, 20.0, trainset)

    assert_eq(speed.velocity_desired, 10.0, "the track allows 10")
    assert_lt(speed.acceleration_desired, MaszynaLegacyDriverSpeed.NO_ACCELERATION, "20 against 10: brake")
