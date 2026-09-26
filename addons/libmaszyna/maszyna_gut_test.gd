extends GutTest
class_name MaszynaGutTest

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
