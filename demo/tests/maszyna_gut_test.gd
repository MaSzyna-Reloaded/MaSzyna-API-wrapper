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
## `description` is an authored vehicle (demo/tests/fixtures/*.tres); without one the vehicle comes
## up empty and the test adds the components it cares about.
func build_vehicle(train_id:String = "TestTrain", description:VehicleController = null,
        initial_velocity:float = 0.0) -> VehicleController:
    return VehicleServer.vehicle_get_controller(build_vehicle_node(train_id, description, initial_velocity).get_vehicle_rid())


## The node itself, for a test that needs a NodePath to the vehicle (RailVehicle3D.controller_path).
func build_vehicle_node(train_id:String = "TestTrain", description:VehicleController = null,
        initial_velocity:float = 0.0) -> VehiclePhysicsNode:
    var physics_node: RailVehiclePhysicsNode = RailVehiclePhysicsNode.new()
    physics_node.controller = description
    physics_node.vehicle_id = train_id
    physics_node.initial_velocity = initial_velocity
    # a rail vehicle, stepped by RailVehicleServer whether or not a RailVehicle3D places it
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
    var model:RailVehicleController = MoverRailVehicleController.new()
    model.vehicle_id = train_id
    model.mass = RAIL_VEHICLE_MASS
    model.type_name = "test"
    var physics_node:VehiclePhysicsNode = build_vehicle_node(train_id, model)
    var vehicle:RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = track_name
    vehicle.start_track_offset = offset
    # a sibling of the physics node, which it takes the vehicle from as it enters the tree
    vehicle.controller_path = NodePath("../%s" % physics_node.name)
    add_child(vehicle)
    return vehicle


## A passenger car standing on the track at the offset, as build_rail_vehicle(), with doors and room
## for `capacity` passengers getting off and on `exchange_speed` a second through one open side
func build_passenger_car(train_id:String, track_name:String, offset:float, capacity:float,
        exchange_speed:float, initial_velocity:float = 0.0) -> RailVehicle3D:
    var model:RailVehicleController = MoverRailVehicleController.new()
    model.vehicle_id = train_id
    model.mass = RAIL_VEHICLE_MASS
    model.type_name = "test"
    model.add_component(MoverRailVehicleDoors.new())
    var load:MoverRailVehicleLoad = MoverRailVehicleLoad.new()
    load.max_load = capacity
    load.load_speed = exchange_speed
    load.unload_speed = exchange_speed
    var accepted:Array[String] = [MaszynaLegacyStation.PASSENGERS]
    load.accepted_loads = accepted
    model.add_component(load)
    var physics_node:VehiclePhysicsNode = build_vehicle_node(train_id, model, initial_velocity)
    var vehicle:RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = track_name
    vehicle.start_track_offset = offset
    vehicle.controller_path = NodePath("../%s" % physics_node.name)
    add_child(vehicle)
    return vehicle


## The vehicle build_rail_vehicle() made, gone: the node first - it lets go of its controller - then
## its physics node, which takes the vehicle out of RailVehicleServer (vehicle_freed)
func free_rail_vehicle(vehicle:RailVehicle3D) -> void:
    var physics_node:Node = vehicle.get_node(vehicle.controller_path)
    remove_child(vehicle)
    vehicle.free()
    physics_node.free()


## An exterior model built in code, drawn as nodes - what a vehicle assembled by hand names its
## parts in (RailVehicle3D.model_instance_path): a transform submodel for every name, placed as
## given, under the submodel `parents` names for it or at the top. Added by the test to its vehicle.
func build_model_instance(submodels:Dictionary, parents:Dictionary) -> E3DModelInstance:
    var built:Dictionary[String, E3DSubModel] = {}
    for submodel_name:String in submodels:
        var submodel:E3DSubModel = E3DSubModel.new()
        submodel.resource_name = submodel_name
        submodel.submodel_type = E3DSubModel.SUBMODEL_TRANSFORM
        submodel.transform = submodels[submodel_name]
        built[submodel_name] = submodel
    var top:Array[E3DSubModel] = []
    for submodel_name:String in built:
        if parents.has(submodel_name):
            var parent:E3DSubModel = built[parents[submodel_name]]
            var children:Array = parent.submodels
            children.append(built[submodel_name])
            parent.submodels = children
        else:
            top.append(built[submodel_name])
    var model:E3DModel = E3DModel.new()
    model.submodels = top
    var instance:E3DModelInstance = E3DModelInstance.new()
    instance.name = "Model"
    instance.instance_kind = E3DRenderingServer.INSTANCE_KIND_DYNAMIC
    instance.model = model
    return instance


## A MaSzyna vehicle built from the game data, added to the test and built - awaited
func spawn_maszyna_vehicle(data_path:String, file_name:String, skin:String, vehicle_id:String) -> MaszynaRailVehicle3D:
    var vehicle:MaszynaRailVehicle3D = MaszynaRailVehicle3D.new()
    vehicle.data_path = data_path
    vehicle.file_name = file_name
    vehicle.skin = skin
    vehicle.vehicle_id = vehicle_id
    add_child(vehicle)
    await vehicle.vehicle_built
    # resumed inside the vehicle's own emission, the test would run on its stack - where the
    # vehicle is locked and cannot be freed; the test goes on from the next frame
    await wait_idle_frames(1)
    return vehicle
