extends MaszynaGutTest

## RailVehicleRenderingServer draws a rail vehicle at the node it is attached to: it moves the node
## wherever RailVehicleServer places the vehicle, poses and hides the submodels of its models, and
## lets go of it with the vehicle.

const TRACK_NAME:String = "rendering_track"
const TRACK_LENGTH:float = 100.0
const OFFSET:float = 30.0
const OTHER_OFFSET:float = 10.0
const TOLERANCE:float = 0.001
const SETTLE_FRAMES:int = 3
const VEHICLE_FIXTURE:String = "res://tests/fixtures/test_vehicle.fiz"
const COUPLING_WITH_BRAKE_HOSE:int = (RailVehicleController.COUPLING_FLAG_COUPLER
        | RailVehicleController.COUPLING_FLAG_BRAKEHOSE)
## The low-poly interior's cabs (LowPolyIntCabs, DynObj.cpp:2383-2391)
const LOW_POLY_CABS:Array[String] = ["cab0", "cab1", "cab2"]

enum HoseGeometry { HANGING_ONLY, CONNECTED }

var _track:RID
var _vehicle:RailVehicle3D
var _other_vehicle:RailVehicle3D
var _model:E3DModelInstance
var _other_model:E3DModelInstance


func before_each() -> void:
    _track = build_track(TRACK_NAME, TRACK_LENGTH)


func after_each() -> void:
    if is_instance_valid(_vehicle):
        free_rail_vehicle(_vehicle)
    if is_instance_valid(_other_vehicle):
        free_rail_vehicle(_other_vehicle)
    TrackServer.track_free(_track)
    TrackServer.topology_rebuild()


func _coupler_model(hose:String, geometry:HoseGeometry) -> E3DModelInstance:
    var submodels:Dictionary = {
        "coupler1_off": Transform3D(),
        "coupler1_on": Transform3D(),
        "coupler2_off": Transform3D(),
        "coupler2_on": Transform3D(),
        hose + "_off": Transform3D(),
    }
    if geometry == HoseGeometry.CONNECTED:
        submodels[hose + "_on"] = Transform3D()
    var no_parents:Dictionary = {}
    var model:E3DModelInstance = build_model_instance(submodels, no_parents)
    for submodel:E3DSubModel in model.model.submodels:
        if submodel.resource_name.ends_with("_on"):
            submodel.dynamic_hidden = true
    return model


func _build_coupler_vehicle(
        vehicle_name:String, offset:float, description:VehicleController, model:E3DModelInstance) -> RailVehicle3D:
    var physics_node:VehiclePhysicsNode = build_vehicle_node(vehicle_name, description)
    var vehicle:RailVehicle3D = RailVehicle3D.new()
    vehicle.start_track_name = TRACK_NAME
    vehicle.start_track_offset = offset
    vehicle.controller_path = NodePath("../%s" % physics_node.name)
    vehicle.add_child(model)
    vehicle.model_instance_path = NodePath("Model")
    add_child(vehicle)
    return vehicle


func test_the_node_stands_where_the_vehicle_is_placed() -> void:
    _vehicle = build_rail_vehicle("RenderingPlaced", TRACK_NAME, OFFSET)
    await wait_idle_frames(SETTLE_FRAMES)

    var rid:RID = _vehicle.get_rid()
    assert_eq(RailVehicleRenderingServer.vehicle_get_node(rid), _vehicle.get_instance_id(), "the vehicle is drawn at its node")
    assert_true(_vehicle.global_transform.is_equal_approx(RailVehicleServer.vehicle_get_transform(rid)),
            "the node stands where the vehicle is placed")

    RailVehicleServer.vehicle_move(rid, 5.0)
    assert_true(_vehicle.global_transform.is_equal_approx(RailVehicleServer.vehicle_get_transform(rid)),
            "and follows it at once when it moves")


func test_a_rebuilt_model_is_announced() -> void:
    _vehicle = build_rail_vehicle("RenderingRebuilt", TRACK_NAME, OFFSET)
    var submodels:Dictionary = {"body": Transform3D()}
    var no_parents:Dictionary = {}
    var model:E3DModelInstance = build_model_instance(submodels, no_parents)
    _vehicle.add_child(model)
    _vehicle.model_instance_path = NodePath("Model")
    await wait_idle_frames(SETTLE_FRAMES)
    watch_signals(RailVehicleRenderingServer)

    model.reload()

    assert_signal_emitted_with_parameters(RailVehicleRenderingServer, "vehicle_model_built", [_vehicle.get_rid()])


func test_the_low_poly_interior_hides_the_cab_the_player_sits_in() -> void:
    _vehicle = build_rail_vehicle("RenderingCabs", TRACK_NAME, OFFSET)
    var exterior_submodels:Dictionary = {"body": Transform3D()}
    var cab_submodels:Dictionary = {}
    for cab:String in LOW_POLY_CABS:
        cab_submodels[cab] = Transform3D()
    var no_parents:Dictionary = {}
    var exterior:E3DModelInstance = build_model_instance(exterior_submodels, no_parents)
    var low_poly:E3DModelInstance = build_model_instance(cab_submodels, no_parents)
    low_poly.name = "LowPoly"
    _vehicle.add_child(exterior)
    _vehicle.add_child(low_poly)
    _vehicle.model_instance_path = NodePath("Model")
    _vehicle.low_poly_cabin_path = NodePath("LowPoly")
    await wait_idle_frames(SETTLE_FRAMES)

    RailVehicleRenderingServer.vehicle_set_cab(_vehicle.get_rid(), 1, true)

    assert_true((low_poly.get_node("cab0") as Node3D).visible, "the machine room stays")
    assert_false((low_poly.get_node("cab1") as Node3D).visible, "the cab sat in is hidden under its modelled cab")
    assert_true((low_poly.get_node("cab2") as Node3D).visible, "the other cab stays")

    RailVehicleRenderingServer.vehicle_set_cab(_vehicle.get_rid(), 0, false)
    assert_true((low_poly.get_node("cab1") as Node3D).visible, "without a modelled cab every low-poly one is shown")


func test_coupling_shows_the_owned_submodel_and_keeps_missing_hose_geometry_off() -> void:
    var description:VehicleController = FizVehicleBuilder.build_description_at(VEHICLE_FIXTURE)
    _model = _coupler_model("cpneumatic2", HoseGeometry.HANGING_ONLY)
    _other_model = _coupler_model("cpneumatic1", HoseGeometry.CONNECTED)
    _vehicle = _build_coupler_vehicle("RenderingCouplerFirst", OFFSET, description, _model)
    _other_vehicle = _build_coupler_vehicle("RenderingCouplerSecond", OTHER_OFFSET, description, _other_model)
    await wait_idle_frames(SETTLE_FRAMES)

    var first_on:Node3D = _model.get_node("coupler2_on")
    var first_off:Node3D = _model.get_node("coupler2_off")
    var second_on:Node3D = _other_model.get_node("coupler1_on")
    var second_off:Node3D = _other_model.get_node("coupler1_off")
    assert_false(first_on.visible)
    assert_true(first_off.visible)
    assert_false(second_on.visible)
    assert_true(second_off.visible)

    RailVehicleServer.vehicle_couple(
            _vehicle.get_rid(), RailVehicleController.COUPLER_END_REAR,
            _other_vehicle.get_rid(), RailVehicleController.COUPLER_END_FRONT,
            COUPLING_WITH_BRAKE_HOSE)

    assert_eq(int(first_on.visible) + int(second_on.visible), 1,
            "exactly the coupling owner should draw the connected mechanical coupler")
    assert_eq(int(first_off.visible) + int(second_off.visible), 1,
            "the other vehicle should retain its hanging coupler when it has no xon variant")
    assert_true((_model.get_node("cpneumatic2_off") as Node3D).visible,
            "a vehicle with no connected hose geometry should retain its hanging hose")
    var second_hose_on:Node3D = _other_model.get_node("cpneumatic1_on")
    var second_hose_off:Node3D = _other_model.get_node("cpneumatic1_off")
    assert_true(second_hose_on.visible, "the other vehicle should draw its connected hose")
    assert_false(second_hose_off.visible)

    var controller:VehicleController = VehicleServer.vehicle_get_controller(_vehicle.get_rid())
    controller.uncouple(RailVehicleController.COUPLER_END_REAR)
    assert_false(second_hose_on.visible, "uncoupling should hide the other vehicle's connected hose")
    assert_true(second_hose_off.visible)

    RailVehicleServer.vehicle_couple(
            _vehicle.get_rid(), RailVehicleController.COUPLER_END_REAR,
            _other_vehicle.get_rid(), RailVehicleController.COUPLER_END_FRONT,
            COUPLING_WITH_BRAKE_HOSE)
    assert_true(second_hose_on.visible, "recoupling should restore the other vehicle's connected hose")
    assert_false(second_hose_off.visible)

    free_rail_vehicle(_vehicle)
    _vehicle = null
    assert_false(second_on.visible, "the remaining vehicle should stop drawing the connected coupler")
    assert_true(second_off.visible, "the remaining vehicle should restore its hanging coupler")


func test_the_vehicle_is_let_go_of_when_freed() -> void:
    _vehicle = build_rail_vehicle("RenderingFreed", TRACK_NAME, OFFSET)
    await wait_idle_frames(SETTLE_FRAMES)
    var rid:RID = _vehicle.get_rid()
    assert_true(RailVehicleRenderingServer.vehicle_is_attached(rid))

    free_rail_vehicle(_vehicle)

    assert_false(RailVehicleRenderingServer.vehicle_is_attached(rid), "freed with the vehicle")
