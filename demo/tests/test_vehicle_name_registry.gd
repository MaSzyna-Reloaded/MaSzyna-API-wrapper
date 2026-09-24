extends MaszynaGutTest

## A vehicle's name belongs to the server that owns its handle: whoever knows a vehicle only by
## name - a scenery event, the console, a `.scn` command - asks the server for the handle, and
## everything that already holds the vehicle never comes through here at all. TrackManager has the
## same pair for tracks, for the same reason.

var _controller:VehicleController = null


func before_each() -> void:
    _controller = build_vehicle("name_registry_test")
    _controller.type_name = "test"
    await wait_idle_frames(2)


func after_each() -> void:
    _controller = null


func test_a_vehicle_is_found_by_the_name_the_scenery_gave_it() -> void:
    var rid:RID = _controller.get_rid()
    assert_eq(
            RailVehicleServer.vehicle_get_name(rid), "name_registry_test",
            "the server knows what the scenery called this vehicle")
    assert_eq(
            RailVehicleServer.vehicle_get_rid_by_name("name_registry_test"), rid,
            "and hands back its handle when asked by that name")


func test_a_name_nothing_carries_answers_with_no_handle() -> void:
    assert_false(
            RID(RailVehicleServer.vehicle_get_rid_by_name("no_such_train")).is_valid(),
            "a name no vehicle carries is not a handle")


func test_a_freed_vehicle_takes_its_name_with_it() -> void:
    var rid:RID = _controller.get_rid()
    RailVehicleServer.vehicle_free(rid)
    assert_false(
            RID(RailVehicleServer.vehicle_get_rid_by_name("name_registry_test")).is_valid(),
            "the name of a vehicle that is gone leads nowhere")
