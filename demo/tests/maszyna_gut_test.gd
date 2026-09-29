extends GutTest
class_name MaszynaGutTest

## Standard gauge of build_track() [m]
const BUILT_TRACK_GAUGE:float = 1.435
## build_rail_vehicle()'s mass [kg]
const RAIL_VEHICLE_MASS:float = 74000.0

func wait_idle_frames(frames, message = ""):
    while frames > 0:
        await Engine.get_main_loop().process_frame
        frames -= 1


## A vehicle is an object owned by RailVehicleServer and stepped by the server's own tick - it is
## not a node, so a test cannot put one in the tree. VehiclePhysicsNode is what brings a vehicle
## into being; autofree owns the node, so the vehicle goes away with the test.
## `model` is an authored vehicle (demo/tests/fixtures/*.tres); without one the vehicle comes up
## empty and the test adds the components it cares about.
func build_vehicle(train_id:String = "TestTrain", model:VehicleModel = null,
        initial_velocity:float = 0.0) -> VehicleController:
    return build_vehicle_node(train_id, model, initial_velocity).get_controller()


## The node itself, for a test that needs a NodePath to the vehicle (RailVehicle3D.controller_path).
func build_vehicle_node(train_id:String = "TestTrain", model:VehicleModel = null,
        initial_velocity:float = 0.0) -> VehiclePhysicsNode:
    var physics_node:VehiclePhysicsNode = VehiclePhysicsNode.new()
    physics_node.set_model(model)
    physics_node.train_id = train_id
    physics_node.initial_velocity = initial_velocity
    add_child_autofree(physics_node)
    return physics_node


## A straight track along +X to stand vehicles on; the test frees it (TrackServer.track_free(), then
## topology_rebuild())
func build_track(track_name:String, length:float) -> RID:
    var curve:TrackCurve = TrackCurve.new()
    curve.p2 = Vector3(length, 0.0, 0.0)
    var track:RID = TrackServer.track_create()
    TrackServer.track_update_curves(track, curve, null)
    TrackServer.track_update(track, TrackServer.TRACK_NORMAL, track_name, BUILT_TRACK_GAUGE)
    TrackServer.topology_rebuild()
    return track


## A RailVehicle3D standing on the track at the offset, with a mass - a vehicle with none integrates
## to NaN (test_rail_vehicle_at_rest.gd). It takes its controller within a few frames; the test frees
## it, before its physics node goes with autofree.
func build_rail_vehicle(train_id:String, track_name:String, offset:float) -> RailVehicle3D:
    var model:VehicleModel = VehicleModel.new()
    model.properties = {"train_id": train_id, "mass": RAIL_VEHICLE_MASS, "type_name": "test"}
    var physics_node:VehiclePhysicsNode = build_vehicle_node(train_id, model)
    var vehicle:RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = track_name
    vehicle.start_track_offset = offset
    add_child(vehicle)
    vehicle.controller_path = vehicle.get_path_to(physics_node)
    return vehicle


## The vehicle build_rail_vehicle() made, gone: the node first - it lets go of its controller - then
## its physics node, which takes the vehicle out of RailVehicleServer (vehicle_freed)
func free_rail_vehicle(vehicle:RailVehicle3D) -> void:
    var physics_node:Node = vehicle.get_node(vehicle.controller_path)
    remove_child(vehicle)
    vehicle.free()
    physics_node.free()
