extends MaszynaGutTest

## A consist is the vehicles' Movers joined by their couplers (TMoverParameters::Attach). The
## brake pipe of every vehicle is fed through the brake hose from the one the driver operates, so
## what the handle does has to reach the last vehicle. Only the pipe is asserted: the fixture's
## W_Lu_L valve has no distributor of its own in the Mover (Mover.cpp:8715 builds a plain TBrake),
## so its cylinder says nothing about the consist.

const FIXTURE_PATH:String = "res://tests/fixtures/test_vehicle.fiz"
const VEHICLE_COUNT:int = 3
## Original engine: coupling::coupler | coupling::brakehose (MOVER.h:161)
const COUPLING_WITH_BRAKE_HOSE:int = 3
## Front and rear coupler of a vehicle standing the normal way (TDynamicObject::AttachNext,
## DynObj.cpp:2590)
const FRONT_END:int = 0
const REAR_END:int = 1
## A non-zero scenery velocity makes a vehicle ready to depart - reservoirs full and the brake
## pipe charged (TMoverParameters::CheckLocomotiveParameters, Mover.cpp:8902); 0.1 is what
## scenery authors write for a standing, ready vehicle
const READY_TO_DEPART_VELOCITY:float = 0.1
const BRAKING_SECONDS:float = 10.0
const RELEASING_SECONDS:float = 30.0
## the brake pipe of a released train (CntrlPipePress)
const CHARGED_PIPE_PRESSURE:float = 5.0
const PRESSURE_TOLERANCE:float = 0.1
## a service braking lowers the pipe by at least this much (FV4a full service: about 1.5 bar)
const SERVICE_BRAKING_PIPE_DROP:float = 1.0
## long enough for a few simulation ticks
const RELEASER_TICK_SECONDS:float = 0.5

var nodes:Array[VehiclePhysicsNode] = []
var controllers:Array[VehicleController] = []
var brakes:Array[RailVehicleBrake] = []


func before_each() -> void:
    nodes.clear()
    controllers.clear()
    brakes.clear()
    var model:VehicleModel = FizVehicleBuilder.build_model_at(FIXTURE_PATH)
    for index:int in range(VEHICLE_COUNT):
        var node:VehiclePhysicsNode = VehiclePhysicsNode.new()
        node.train_id = "ConsistVehicle%d" % index
        # the first vehicle is driven - an unmanned one is not simulated (FINDINGS, 09-23)
        node.driver_type = VehicleController.DRIVER_HEAD if index == 0 else VehicleController.DRIVER_NOBODY
        node.initial_velocity = READY_TO_DEPART_VELOCITY
        node.set_model(model)
        add_child_autofree(node)
        nodes.append(node)
        var controller:VehicleController = node.get_controller()
        controllers.append(controller)
        brakes.append(controller.get_component(VehicleComponentType.COMPONENT_BRAKES) as RailVehicleBrake)
    await wait_idle_frames(2)
    for index:int in range(1, VEHICLE_COUNT):
        controllers[index - 1].couple(controllers[index], REAR_END, FRONT_END, COUPLING_WITH_BRAKE_HOSE)


func after_each() -> void:
    nodes.clear()
    controllers.clear()
    brakes.clear()


func test_every_vehicle_is_joined_by_the_brake_hose() -> void:
    for index:int in range(VEHICLE_COUNT - 1):
        assert_true(controllers[index].is_coupled(REAR_END), "vehicle %d rear coupler" % index)
        assert_true(controllers[index].is_coupled_by(REAR_END, RailVehicleController.COUPLING_ELEMENT_BRAKEHOSE),
                "vehicle %d rear brake hose" % index)
        assert_eq(controllers[index].get_coupled_controller(REAR_END), controllers[index + 1])
    var joined:Array = RailVehicleServer.vehicle_get_coupled(
            controllers[0].get_rid(), FRONT_END, RailVehicleController.COUPLING_ELEMENT_BRAKEHOSE)
    assert_eq(joined.size(), VEHICLE_COUNT)


func test_the_pipe_of_the_last_vehicle_follows_the_handle() -> void:
    var last:RailVehicleBrake = brakes[VEHICLE_COUNT - 1]
    assert_almost_eq(last.pipe_pressure, CHARGED_PIPE_PRESSURE, PRESSURE_TOLERANCE, "a ready train starts charged")

    controllers[0].send_command("brake_level_set_position", "full")
    await wait_seconds(BRAKING_SECONDS)
    assert_lt(last.pipe_pressure, CHARGED_PIPE_PRESSURE - SERVICE_BRAKING_PIPE_DROP, "the pipe of the last vehicle empties")

    controllers[0].send_command("brake_level_set_position", "drive")
    await wait_seconds(RELEASING_SECONDS)
    assert_almost_eq(last.pipe_pressure, CHARGED_PIPE_PRESSURE, PRESSURE_TOLERANCE, "the pipe of the last vehicle refills")


func test_uncouple_parts_the_vehicles() -> void:
    controllers[0].uncouple(REAR_END)
    assert_false(controllers[0].is_coupled(REAR_END))
    assert_false(controllers[1].is_coupled(FRONT_END))


func test_the_consist_releaser_is_held_only_while_the_brakes_brake() -> void:
    controllers[0].send_command("consist_releaser", true)
    assert_true(brakes[0].releaser_active, "switched on")
    await wait_seconds(RELEASER_TICK_SECONDS)
    assert_false(brakes[0].releaser_active, "a released brake lets go of it on the next tick")


func test_a_freed_vehicle_leaves_its_neighbours_uncoupled() -> void:
    var first:RID = controllers[0].get_rid()
    watch_signals(controllers[0])
    nodes[1].free()

    assert_false(controllers[0].is_coupled(REAR_END), "the front neighbour lets go of the freed vehicle")
    assert_false(controllers[2].is_coupled(FRONT_END), "the rear neighbour lets go of the freed vehicle")
    assert_eq(RailVehicleServer.vehicle_get_coupled(
            first, FRONT_END, RailVehicleController.COUPLING_ELEMENT_COUPLER).size(), 1)
    assert_signal_emitted(controllers[0], "consist_changed")
