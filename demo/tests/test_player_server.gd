extends MaszynaGutTest

## PlayerServer - what the player drives: taking a vehicle over and letting it go, the way the
## vehicle's driver steps back and takes over again, announced to whoever follows it.

const TRACK_NAME:String = "player_server_test"
const TRACK_LENGTH:float = 200.0
const FIRST_OFFSET:float = 50.0
const SECOND_OFFSET:float = 150.0
## Enough frames for the vehicles to take their controllers and stand on their track
const SETTLE_FRAMES:int = 4
## The cab the crew of a head-driven vehicle sits in (VehicleController.get_occupied_cab())
const HEAD_CAB:int = 1

var _track:RID
var _first:RailVehicle3D
var _second:RailVehicle3D
## What PlayerServer announced: [vehicle, previous] per change
var _announced:Array = []


func before_each() -> void:
    _announced.clear()
    _track = build_track(TRACK_NAME, TRACK_LENGTH)
    _first = build_rail_vehicle("PlayerServerFirst", TRACK_NAME, FIRST_OFFSET)
    _second = build_rail_vehicle("PlayerServerSecond", TRACK_NAME, SECOND_OFFSET)
    PlayerServer.player_vehicle_changed.connect(_on_player_vehicle_changed)


func after_each() -> void:
    PlayerServer.player_leave_vehicle()
    PlayerServer.player_vehicle_changed.disconnect(_on_player_vehicle_changed)
    if is_instance_valid(_first):
        free_rail_vehicle(_first)
    if is_instance_valid(_second):
        free_rail_vehicle(_second)
    TrackServer.track_free(_track)
    TrackServer.topology_rebuild()


func _on_player_vehicle_changed(vehicle:RID, previous:RID) -> void:
    _announced.append([vehicle, previous])


func test_taking_over_and_letting_go_are_announced_with_the_vehicle_before() -> void:
    await wait_idle_frames(SETTLE_FRAMES)
    var first:RID = _first.get_rid()
    var second:RID = _second.get_rid()

    PlayerServer.player_enter_vehicle(first)
    PlayerServer.player_enter_vehicle(first)
    PlayerServer.player_enter_vehicle(second)
    PlayerServer.player_leave_vehicle()

    assert_eq(_announced, [[first, RID()], [second, first], [RID(), second]], "the same vehicle again is no change")
    assert_eq(PlayerServer.player_get_vehicle(), RID())


## drivermode.cpp:266 - the driver of the vehicle taken over only takes orders; let go, it drives
## again (TakeControl(), Driver.cpp:5700)
func test_the_driver_steps_back_while_the_player_drives() -> void:
    await wait_idle_frames(SETTLE_FRAMES)
    var vehicle:RID = _first.get_rid()
    var driver:RID = DriverSystem.driver_create()
    DriverSystem.driver_attach_vehicle(driver, vehicle)

    PlayerServer.player_enter_vehicle(vehicle)
    assert_false(DriverSystem.vehicle_is_control_active(vehicle), "the player drives")
    PlayerServer.player_leave_vehicle()
    assert_true(DriverSystem.vehicle_is_control_active(vehicle), "the driver drives again")
    DriverSystem.driver_free(driver)


## Train.cpp:9147 - taking a vehicle over activates the cab its crew sits in, with or without a 3D
## cab to look from
func test_taking_over_activates_the_cab() -> void:
    var physics_node:VehiclePhysicsNode = _first.get_node(_first.controller_path) as VehiclePhysicsNode
    physics_node.driver_type = VehicleController.DRIVER_HEAD
    physics_node.get_controller().apply_configuration()
    await wait_idle_frames(SETTLE_FRAMES)
    var vehicle:RID = _first.get_rid()
    assert_eq(RailVehicleServer.vehicle_dump_state(vehicle).get("cabin_occupied", 0), HEAD_CAB)
    assert_eq(RailVehicleServer.vehicle_dump_state(vehicle).get("cabin", -1), 0, "no cab active before")

    PlayerServer.player_enter_vehicle(vehicle)
    await wait_idle_frames(SETTLE_FRAMES)

    assert_eq(RailVehicleServer.vehicle_dump_state(vehicle).get("cabin", 0), HEAD_CAB)


func test_a_vehicle_without_a_node_is_refused() -> void:
    var controller:VehicleController = build_vehicle("PlayerServerBare")
    await wait_idle_frames(SETTLE_FRAMES)

    PlayerServer.player_enter_vehicle(controller.get_rid())

    assert_engine_error("The vehicle has no cab to sit in")
    assert_eq(PlayerServer.player_get_vehicle(), RID())
    assert_eq(_announced, [])


func test_a_freed_vehicle_is_let_go() -> void:
    await wait_idle_frames(SETTLE_FRAMES)
    var vehicle:RID = _first.get_rid()
    PlayerServer.player_enter_vehicle(vehicle)

    free_rail_vehicle(_first)

    assert_eq(PlayerServer.player_get_vehicle(), RID())
    assert_eq(_announced.back(), [RID(), vehicle])
