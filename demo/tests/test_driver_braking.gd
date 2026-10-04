extends MaszynaGutTest

## MaszynaLegacyDriverBraking: the original driver's brakes, through the cab
## (control_braking_force(), Driver.cpp:8065-8190).

const Order = MaszynaLegacyAIDriver.Order
const SM42:VehicleController = preload("res://tests/fixtures/sm42_vehicle.tres")
const REACTION:float = MaszynaLegacyAIDriver.EASY_REACTION_TIME
## No dispatch at a stop (StationServer)
const NO_DISPATCH:StationServer.DispatchStep = StationServer.DISPATCH_STEP_NONE
const STEP:float = 0.5

var train:VehicleController
var vehicle:RID
var braking:MaszynaLegacyDriverBraking
var speed:MaszynaLegacyDriverSpeed
var trainset:MaszynaLegacyDriverTrainset
var route:MaszynaLegacyDriverRoute


func before_each():
    train = build_vehicle("DriverBrakingTest", SM42)
    vehicle = train.get_rid()
    # a cab with no controls of its own: the knobs are there unmodelled
    var controls:LegacyCabinControls = LegacyCabinControls.new()
    CabinSystem.vehicle_attach_cab_logic(
            vehicle, LegacyCabinLogic.new(func(_cab:int) -> LegacyCabinControls: return controls))
    braking = MaszynaLegacyDriverBraking.new()
    speed = MaszynaLegacyDriverSpeed.new()
    trainset = MaszynaLegacyDriverTrainset.new()
    trainset.update(vehicle, 1, false)
    route = MaszynaLegacyDriverRoute.new()


func after_each():
    CabinSystem.vehicle_attach_cab_logic(vehicle, null)


## One update of the driver: its decision, then the handles set (control_braking_force(),
## SetTimeControllers())
func _decide() -> void:
    var situation:MaszynaLegacyDriverTraction.Situation = MaszynaLegacyDriverTraction.Situation.new()
    situation.vehicle = vehicle
    situation.cab = 1
    situation.controlling = vehicle
    situation.order = Order.SHUNT
    situation.speed = speed
    situation.trainset = trainset
    situation.route = route
    situation.braking = braking
    braking.control(situation, STEP)
    braking.set_time_controllers(situation)


func test_standing_it_holds_the_locomotive_with_its_own_brake():
    speed.pick(Order.SHUNT, false, false, NO_DISPATCH, 0.0, 0.0, -1.0, 0.0, trainset, route, REACTION, braking)

    # the fixture's handle starts at lap: the train brake to running first, then the local brake
    _decide()
    _decide()

    assert_eq(float(train.get_state()["brake_local_position_normalized"]), MaszynaLegacyDriverBraking.LOCAL_BRAKE_APPLIED)


func test_wanting_to_go_it_releases():
    speed.pick(Order.SHUNT, false, false, NO_DISPATCH, 0.0, 0.0, -1.0, 0.0, trainset, route, REACTION, braking)
    _decide()
    _decide()
    assert_eq(float(train.get_state()["brake_local_position_normalized"]), MaszynaLegacyDriverBraking.LOCAL_BRAKE_APPLIED)

    speed.pick(Order.SHUNT, true, false, NO_DISPATCH, 20.0, 20.0, -1.0, 0.0, trainset, route, REACTION, braking)
    _decide()

    assert_eq(float(train.get_state()["brake_local_position_normalized"]), MaszynaLegacyDriverBraking.LOCAL_BRAKE_RELEASED)
    assert_eq(braking.position, MaszynaLegacyDriverBraking.POSITION_RUNNING, "the train brake at running")
