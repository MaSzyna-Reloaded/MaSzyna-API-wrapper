extends MaszynaGutTest

## RailVehicleRenderingServer draws a rail vehicle at the node it is attached to: it moves the node
## wherever RailVehicleServer places the vehicle, poses and hides the submodels of its models, and
## lets go of it with the vehicle.

const TRACK_NAME:String = "rendering_track"
const TRACK_LENGTH:float = 100.0
const OFFSET:float = 30.0
const TOLERANCE:float = 0.001
const SETTLE_FRAMES:int = 3
## The low-poly interior's cabs (LowPolyIntCabs, DynObj.cpp:2383-2391)
const LOW_POLY_CABS:Array[String] = ["cab0", "cab1", "cab2"]

var _track:RID
var _vehicle:RailVehicle3D


func before_each() -> void:
    _track = build_track(TRACK_NAME, TRACK_LENGTH)


func after_each() -> void:
    if is_instance_valid(_vehicle):
        free_rail_vehicle(_vehicle)
    TrackServer.track_free(_track)
    TrackServer.topology_rebuild()


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


func test_the_vehicle_is_let_go_of_when_freed() -> void:
    _vehicle = build_rail_vehicle("RenderingFreed", TRACK_NAME, OFFSET)
    await wait_idle_frames(SETTLE_FRAMES)
    var rid:RID = _vehicle.get_rid()
    assert_true(RailVehicleRenderingServer.vehicle_is_attached(rid))

    free_rail_vehicle(_vehicle)

    assert_false(RailVehicleRenderingServer.vehicle_is_attached(rid), "freed with the vehicle")
