extends MaszynaGutTest
## The declared-property path: a component says what it can be asked about, the registry interns
## the name once, and the value is fetched from the component that owns it - nothing is copied into
## a Dictionary per frame.
##
## VehicleState is the first object in this project to implement _get/_set/_get_property_list, so
## the behaviour those give GDScript is asserted here rather than assumed.

var _vehicle_rid: RID = RID()
var _controller: VehicleController = null
var _heating: VehicleHeating = null


func before_each() -> void:
    _controller = VehicleController.new()
    _controller.name = "StatePropertyController"
    _controller.train_id = "state_property_test"
    _controller.type_name = "test"
    _heating = VehicleHeating.new()
    _heating.name = "Heating"
    _controller.add_child(_heating)
    add_child(_controller)
    _vehicle_rid = RailVehicleServer.vehicle_create()
    RailVehicleServer.vehicle_attach_controller(_vehicle_rid, _controller.get_instance_id())
    await wait_idle_frames(2)


func after_each() -> void:
    if _vehicle_rid.is_valid():
        RailVehicleServer.vehicle_free(_vehicle_rid)
        _vehicle_rid = RID()
    if is_instance_valid(_controller):
        remove_child(_controller)
        _controller.free()
    _controller = null
    _heating = null


func test_a_declared_name_resolves_to_a_stable_id() -> void:
    var id: int = RailVehicleServer.state_property_get_id("heating_enabled")
    assert_gt(id, -1, "the component's declaration interned the name")
    assert_eq(id, VehicleState.resolve("heating_enabled"), "both ways in agree on the id")


func test_an_undeclared_name_resolves_to_minus_one() -> void:
    assert_eq(VehicleState.resolve("no_such_property_anywhere"), -1)


## The value is fetched live from the component that owns it, so a change is visible with no step,
## no tick and nothing copied in between - which is the whole point of dropping the Dictionary.
func test_the_value_is_read_live_from_the_component() -> void:
    var id: int = RailVehicleServer.state_property_get_id("heating_power")
    _controller.heating_power = 123.0
    _controller.apply_config()
    assert_almost_eq(
        float(RailVehicleServer.vehicle_get_state_value(_vehicle_rid, id)),
        123.0,
        0.001,
        "the server asks the component rather than a copy made earlier"
    )


func test_state_proxy_reads_by_name_and_by_id() -> void:
    var state: VehicleState = RailVehicleServer.vehicle_get_state(_vehicle_rid)
    assert_not_null(state)
    var id: int = VehicleState.resolve("heating_enabled")
    assert_eq(state.get_by_id(id), state.get("heating_enabled"), "_get by name matches the id path")


func test_state_proxy_is_the_same_object_every_time() -> void:
    assert_same(
        RailVehicleServer.vehicle_get_state(_vehicle_rid),
        RailVehicleServer.vehicle_get_state(_vehicle_rid),
        "taking the state is one crossing, not an allocation"
    )


func test_read_floats_answers_many_ids_in_one_call() -> void:
    var state: VehicleState = RailVehicleServer.vehicle_get_state(_vehicle_rid)
    var ids: PackedInt32Array = PackedInt32Array([
        VehicleState.resolve("heating_enabled"),
        VehicleState.resolve("heating_power"),
    ])
    var values: PackedFloat64Array = state.read_floats(ids)
    assert_eq(values.size(), 2)
    assert_eq(values[1], state.get_float_by_id(ids[1]))


func test_property_list_holds_only_what_this_vehicle_publishes() -> void:
    var state: VehicleState = RailVehicleServer.vehicle_get_state(_vehicle_rid)
    var names: PackedStringArray = PackedStringArray()
    for property: Dictionary in state.get_property_list():
        names.append(String(property["name"]))
    assert_true(names.has("heating_enabled"), "a property this vehicle has is listed")
    assert_false(names.has("brake_pipe_pressure"), "one it has not is not")


func test_snapshot_is_keyed_by_name() -> void:
    var snapshot: Dictionary = RailVehicleServer.vehicle_get_state_snapshot(_vehicle_rid)
    assert_true(snapshot.has("heating_enabled"))
    assert_true(snapshot.has("heating_power"))


## A vehicle can be freed while a script still holds its state; reading it must be safe and say so,
## which is why VehicleState holds the handle and never a pointer.
func test_reading_after_the_vehicle_is_freed_is_safe() -> void:
    var state: VehicleState = RailVehicleServer.vehicle_get_state(_vehicle_rid)
    RailVehicleServer.vehicle_free(_vehicle_rid)
    _vehicle_rid = RID()
    assert_false(state.is_valid(), "the handle no longer names a vehicle")
    assert_eq(state.get_float_by_id(VehicleState.resolve("heating_power")), 0.0)
