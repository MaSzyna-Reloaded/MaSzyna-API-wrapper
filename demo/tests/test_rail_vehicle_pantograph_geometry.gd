extends MaszynaGutTest

## Where each pantograph sits on the vehicle is the vehicle's own geometry, and it comes from the
## model: the original reads it off the pantograph submodel's matrix (TAnimPant::vPos,
## DynObj.cpp:5508-5549 - sideways, up, and along the length). RailVehicle3D reads the same thing
## from the arm nodes it resolves and publishes it to the electric engine, because the wire is
## sampled at those points - one per pantograph.
##
## The test discriminates: until the position was published, both pantographs of every vehicle in
## the game sampled the wire at the same place - the vehicle's origin - because the two exported
## offsets on the node were never written by anything.

const FRONT_ALONG:float = -5.0
const REAR_ALONG:float = 5.0
const LOWER_HEIGHT:float = 4.0
const UPPER_HEIGHT:float = 4.6
const SLIDER_HEIGHT:float = 5.2
## the upper arm leans along the vehicle, so the arm has a length to read at all
const UPPER_LEAN:float = 0.1
const ARM_NODE_COUNT:int = 5
const TOLERANCE:float = 0.001
## The vehicle's tick is driven here rather than awaited: the geometry is read once, when the arm
## paths change, so a test that waits for frames is asserting against whatever the frame count
## happened to be. RailVehicle3D binds its tick as a callable method for exactly this.
const TICK:float = 1.0 / 60.0

var vehicle:RailVehicle3D
var physics_node:VehiclePhysicsNode
var engine:RailVehicleElectricEngine


func after_each() -> void:
    if is_instance_valid(vehicle):
        if vehicle.get_parent():
            vehicle.get_parent().remove_child(vehicle)
        vehicle.queue_free()
    vehicle = null
    engine = null
    physics_node = null


## The five nodes a pantograph is animated through, in the order RailVehicle3D resolves them:
## lower arm 1, lower arm 2, upper arm 1, upper arm 2, slider. The second arm of each pair is
## optional in the data, so it stands where the first one does.
func _add_pantograph_arms(along:float) -> Array[NodePath]:
    var positions:Array[Vector3] = [
        Vector3(0.0, LOWER_HEIGHT, along),
        Vector3(0.0, LOWER_HEIGHT, along),
        Vector3(0.0, UPPER_HEIGHT, along + UPPER_LEAN),
        Vector3(0.0, UPPER_HEIGHT, along + UPPER_LEAN),
        Vector3(0.0, SLIDER_HEIGHT, along),
    ]
    var paths:Array[NodePath] = []
    for index:int in range(ARM_NODE_COUNT):
        var arm:Node3D = Node3D.new()
        arm.name = "Arm%s_%s" % [str(along).replace(".", "_").replace("-", "m"), index]
        vehicle.add_child(arm)
        arm.position = positions[index]
        paths.append(vehicle.get_path_to(arm))
    return paths


func _build_electric_vehicle() -> void:
    var model:VehicleModel = VehicleModel.new()
    model.properties = {"type_name": "test"}
    physics_node = build_vehicle_node("test_pantograph_geometry", model)
    engine = MoverRailVehicleElectricSeriesEngine.new()
    engine.power_source = RailVehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    engine.power_current_collector_number_of_collectors = 2
    physics_node.get_controller().add_component(engine)

    vehicle = RailVehicle3D.new()
    add_child(vehicle)
    vehicle.controller_path = vehicle.get_path_to(physics_node)
    vehicle.pantograph_front_arm_paths = _add_pantograph_arms(FRONT_ALONG)
    vehicle.pantograph_rear_arm_paths = _add_pantograph_arms(REAR_ALONG)
    vehicle._process(TICK)


func test_each_pantograph_publishes_its_own_position_to_the_vehicle() -> void:
    _build_electric_vehicle()

    assert_almost_eq(
            engine.power_current_collector_first_position,
            Vector3(0.0, LOWER_HEIGHT, FRONT_ALONG),
            Vector3(TOLERANCE, TOLERANCE, TOLERANCE),
            "the front pantograph's position is where its lower arm stands on the vehicle")
    assert_almost_eq(
            engine.power_current_collector_second_position,
            Vector3(0.0, LOWER_HEIGHT, REAR_ALONG),
            Vector3(TOLERANCE, TOLERANCE, TOLERANCE),
            "the rear pantograph's position is where its lower arm stands on the vehicle")


func test_the_two_pantographs_do_not_share_one_sampling_point() -> void:
    _build_electric_vehicle()

    var front:Vector3 = engine.power_current_collector_first_position
    var rear:Vector3 = engine.power_current_collector_second_position
    assert_almost_eq(
            rear.z - front.z, REAR_ALONG - FRONT_ALONG, TOLERANCE,
            "the two pantographs are as far apart along the vehicle as the model puts them")
    assert_true(
            front.length() > 0.0 and rear.length() > 0.0,
            "neither pantograph samples the wire at the vehicle's origin, got %s and %s" % [front, rear])


func test_a_vehicle_without_pantograph_arms_publishes_no_position() -> void:
    var model:VehicleModel = VehicleModel.new()
    model.properties = {"type_name": "test"}
    physics_node = build_vehicle_node("test_pantograph_geometry_bare", model)
    engine = MoverRailVehicleElectricSeriesEngine.new()
    engine.power_source = RailVehicleController.POWER_SOURCE_CURRENTCOLLECTOR
    physics_node.get_controller().add_component(engine)

    vehicle = RailVehicle3D.new()
    add_child(vehicle)
    vehicle.controller_path = vehicle.get_path_to(physics_node)
    vehicle._process(TICK)

    assert_eq(
            engine.power_current_collector_first_position, Vector3(),
            "a vehicle whose model carries no pantograph has nothing to publish")
