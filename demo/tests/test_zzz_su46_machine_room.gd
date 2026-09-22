extends MaszynaGutTest

## SU46 (dynamic/pkp/su46_v2, 303d2.mmd) declares cab0definition: without a cab0model: - the
## original then has no hi-fi cab (Train.cpp:8692, mdKabina stays nullptr) and keeps every low-poly
## "cabN" submodel visible (DynObj.cpp:1214), so the machine room is the low-poly interior's cab0.

class PlayerStub extends Node3D:
    var camera:FreeCamera3D

    func get_camera() -> FreeCamera3D:
        return camera


const REAL_GAME_DIR:String = "/home/marcin/Games/Maszyna"

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


func _low_poly_cab_visible(cab_index:int) -> bool:
    var low_poly:Node3D = vehicle.get_node(vehicle.low_poly_cabin_path) as Node3D
    var cab_node:Node3D = low_poly.find_child("cab%d" % cab_index, true, false) as Node3D
    assert_not_null(cab_node, "SU46 low-poly interior should contain cab%d" % cab_index)
    return cab_node.visible if cab_node else false


func test_machine_room_without_cab_model_shows_low_poly_interior() -> void:
    if not DirAccess.dir_exists_absolute(REAL_GAME_DIR.path_join("dynamic/pkp/su46_v2")):
        pending("real SU46 game data not available on this machine at %s" % REAL_GAME_DIR)
        return
    UserSettings.save_maszyna_game_dir(REAL_GAME_DIR)
    vehicle = DynamicRailVehicle3DManager.load(
            "dynamic/pkp/su46_v2", "303d2", "303d-048", "test_su46_machine_room", 0.0, null)
    add_child(vehicle)
    for i in range(20):
        if vehicle.get_controller():
            break
        await wait_idle_frames(1)
    var controller:VehicleController = vehicle.get_controller()
    assert_not_null(controller, "SU46's FIZ controller should be built")
    if not controller:
        return

    player = PlayerStub.new()
    player.camera = FreeCamera3D.new()
    player.add_child(player.camera)
    add_child(player)
    vehicle.enter_cabin(player)
    await wait_idle_frames(3)
    assert_false(_low_poly_cab_visible(1), "hi-fi cab 1 hides its low-poly counterpart")

    controller.send_command("cab_change", -1)
    await wait_idle_frames(3)

    var cabin:Cabin3D = player.camera.get_parent() as Cabin3D
    assert_not_null(cabin, "camera should stay in the cabin in the machine room")
    if not cabin:
        return
    assert_eq(cabin.cab_number, 0)
    assert_false(cabin.has_cab_model, "SU46 cab0definition: has no cab0model:")
    for cab_index:int in range(3):
        assert_true(_low_poly_cab_visible(cab_index), "low-poly cab%d should be visible in the machine room" % cab_index)
