extends MaszynaGutTest

## Cab switching on the REAL EP07 (dynamic/pkp/303e_v1): cab_change -1 moves the crew from cab 1
## to the machine room (cab0definition:, Train.cpp:8908 - its own instruments up to the end of the
## MMD) and then to cab 2. The camera looks along VectorFront * CabOccupied (drivermode.cpp:1071),
## i.e. backward from cab 2. The low-poly interior stays visible from inside with only the
## occupied "cabN" submodel hidden (DynObj.cpp:1211-1219), so the other cabs show through windows.

class PlayerStub extends Node3D:
    var camera:FreeCamera3D

    func get_camera() -> FreeCamera3D:
        return camera


const REAL_GAME_DIR:String = "/home/marcin/Games/MaSzyna"

var _previous_game_dir:String
var vehicle:RailVehicle3D
var player:PlayerStub


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()


func after_each() -> void:
    if is_instance_valid(vehicle):
        vehicle.free()
    if is_instance_valid(player):
        player.free()
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func _assert_low_poly_cabs_visible(expected:Array[bool], label:String) -> void:
    var low_poly:Node3D = vehicle.get_node(vehicle.low_poly_cabin_path) as Node3D
    assert_true(low_poly.visible, "%s: low-poly interior should stay visible" % label)
    for cab_index:int in range(3):
        var cab_node:Node3D = low_poly.find_child("cab%d" % cab_index, true, false) as Node3D
        assert_not_null(cab_node, "EP07 low-poly interior should contain cab%d" % cab_index)
        if cab_node:
            assert_eq(cab_node.visible, expected[cab_index], "%s: low-poly cab%d visibility" % [label, cab_index])


func test_cab_change_moves_camera_to_rear_cab_facing_backward() -> void:
    if not DirAccess.dir_exists_absolute(REAL_GAME_DIR.path_join("dynamic/pkp/303e_v1")):
        pending("real EP07 game data not available on this machine at %s" % REAL_GAME_DIR)
        return
    UserSettings.save_maszyna_game_dir(REAL_GAME_DIR)
    vehicle = DynamicRailVehicle3DManager.load(
            "dynamic/pkp/303e_v1", "303e-ep-tv", "303e-ep-tv-424-hist", "test_ep07_cab_change", 0.0, null)
    add_child(vehicle)
    for i in range(20):
        if vehicle.get_controller():
            break
        await wait_idle_frames(1)
    var controller:VehicleController = vehicle.get_controller()
    assert_not_null(controller, "EP07's FIZ controller should be built")
    if not controller:
        return

    player = PlayerStub.new()
    player.camera = FreeCamera3D.new()
    player.add_child(player.camera)
    add_child(player)
    vehicle.enter_cabin(player)
    await wait_idle_frames(3)

    var camera:FreeCamera3D = player.camera
    var vehicle_forward:Vector3 = -vehicle.global_basis.z
    var cab1_z:float = vehicle.to_local(camera.global_position).z
    assert_true((-camera.global_basis.z).dot(vehicle_forward) > 0.99, "cab 1 camera should look forward")
    _assert_low_poly_cabs_visible([true, false, true], "cab 1")

    controller.send_command("cab_change", -1)
    await wait_idle_frames(3)

    var machine_room:DynamicTrainCabin = camera.get_parent() as DynamicTrainCabin
    assert_eq(controller.state.get("cabin_occupied", 1), 0)
    assert_not_null(machine_room, "camera should stay in the cabin in the machine room")
    if not machine_room:
        return
    assert_eq(machine_room.cab_number, 0, "cabin should be rebuilt as the machine room")
    assert_true(
        absf(vehicle.to_local(camera.global_position).z) < absf(cab1_z),
        "machine room camera should sit between the cabs",
    )
    for diagnostic:Dictionary in machine_room.get_diagnostics():
        assert_false(diagnostic["code"] == "MMD_INVALID_CAB_DEFINITION", "EP07 declares cab0definition:")
    _assert_low_poly_cabs_visible([false, true, true], "machine room")

    controller.send_command("cab_change", -1)
    await wait_idle_frames(3)

    var cabin:Cabin3D = camera.get_parent() as Cabin3D
    assert_eq(controller.state.get("cabin_occupied", 0), -1)
    assert_not_null(cabin, "camera should stay in the cabin after cab change")
    if not cabin:
        return
    assert_eq(cabin.cab_number, -1, "cabin should be rebuilt as cab 2")
    var cab2_z:float = vehicle.to_local(camera.global_position).z
    assert_true(
        signf(cab2_z) == -signf(cab1_z),
        "cab 2 camera (z=%s) should sit at the other end than cab 1 (z=%s)" % [cab2_z, cab1_z],
    )
    assert_true((-camera.global_basis.z).dot(vehicle_forward) < -0.99, "cab 2 camera should look backward")
    _assert_low_poly_cabs_visible([true, true, false], "cab 2")

    vehicle.leave_cabin(player)
    _assert_low_poly_cabs_visible([true, true, true], "outside")
