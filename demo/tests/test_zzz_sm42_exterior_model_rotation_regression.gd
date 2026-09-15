extends MaszynaGutTest

## Regression test for SM42 (and every MaSzyna-data vehicle) being drawn reversed. The original
## draws the exterior, low-poly interior, passengers and cab all under one vehicle-local frame
## with +Z = direction of travel (TDynamicObject::mMatrix, DynObj.cpp:2506-2508); RailVehicle3D
## uses Godot's -Z forward. MaszynaRailVehicle3DInstancer._build_structure() converts all of them
## with the same 180 degree yaw - these tests spawn the REAL SM42 via
## DynamicRailVehicle3DManager.load() and check every part ends up in the same frame, facing
## the vehicle's own forward.

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


func _spawn_sm42() -> bool:
    if not DirAccess.dir_exists_absolute(REAL_GAME_DIR.path_join("dynamic/pkp/sm42_v1")):
        pending("real SM42 game data not available on this machine at %s" % REAL_GAME_DIR)
        return false
    UserSettings.save_maszyna_game_dir(REAL_GAME_DIR)
    vehicle = DynamicRailVehicle3DManager.load("dynamic/pkp/sm42_v1", "6da", "6d-907", "test_sm42_rotation", 0.0, null)
    add_child(vehicle)
    var model:E3DModelInstance = vehicle.get_node(vehicle.model_instance_path) as E3DModelInstance
    for i in range(20):
        await wait_idle_frames(1)
        if model.is_e3d_loaded():
            break
    assert_true(model.is_e3d_loaded(), "SM42's real exterior model should finish loading")
    return model.is_e3d_loaded()


func _exterior_cab_z() -> float:
    var model:E3DModelInstance = vehicle.get_node(vehicle.model_instance_path) as E3DModelInstance
    var cab:Node3D = model.find_child("budka_maszynisty", true, false) as Node3D
    assert_not_null(cab, "SM42's exterior model should contain its cab shell (budka_maszynisty)")
    return vehicle.to_local(cab.global_position).z if cab else 0.0


func test_exterior_nose_faces_vehicle_forward() -> void:
    if not await _spawn_sm42():
        return
    var model:E3DModelInstance = vehicle.get_node(vehicle.model_instance_path) as E3DModelInstance
    var nose:Node3D = model.find_child("nos01", true, false) as Node3D
    assert_not_null(nose, "SM42's real model should contain a nos01 submodel")
    if not nose:
        return
    var nose_local:Vector3 = vehicle.to_local(nose.global_position)
    assert_true(nose_local.z < 0.0, "nos01 should sit on the forward (-Z) side of the vehicle, got %s" % nose_local)


func test_low_poly_interior_shares_exterior_frame() -> void:
    if not await _spawn_sm42():
        return
    var exterior:E3DModelInstance = vehicle.get_node(vehicle.model_instance_path) as E3DModelInstance
    var low_poly:E3DModelInstance = vehicle.get_node(vehicle.low_poly_cabin_path) as E3DModelInstance
    assert_not_null(low_poly, "SM42 declares a lowpolyinterior: model")
    if not low_poly:
        return
    assert_true(low_poly.basis.is_equal_approx(exterior.basis), "low-poly interior must use the exterior's frame")

    var cab_mesh:MeshInstance3D = low_poly.find_child("cab1", true, false) as MeshInstance3D
    assert_not_null(cab_mesh, "SM42's low-poly interior should contain cab1")
    if not cab_mesh or not cab_mesh.mesh:
        return
    var cab_center:Vector3 = vehicle.to_local(cab_mesh.global_transform * cab_mesh.mesh.get_aabb().get_center())
    var exterior_cab_z:float = _exterior_cab_z()
    assert_true(
        signf(cab_center.z) == signf(exterior_cab_z),
        "low-poly cab (z=%s) should sit inside the exterior's cab shell (z=%s)" % [cab_center.z, exterior_cab_z],
    )


func test_cabin_camera_sits_in_exterior_cab_and_looks_forward() -> void:
    if not await _spawn_sm42():
        return
    for i in range(20):
        if vehicle.get_controller():
            break
        await wait_idle_frames(1)
    assert_not_null(vehicle.get_controller(), "SM42's FIZ controller should be built")
    if not vehicle.get_controller():
        return

    player = PlayerStub.new()
    player.camera = FreeCamera3D.new()
    player.add_child(player.camera)
    add_child(player)
    vehicle.enter_cabin(player)
    await wait_idle_frames(3)

    var camera:FreeCamera3D = player.camera
    assert_false(camera.get_parent() == player, "camera should have moved into the cabin")
    var camera_local:Vector3 = vehicle.to_local(camera.global_position)
    var exterior_cab_z:float = _exterior_cab_z()
    assert_true(
        signf(camera_local.z) == signf(exterior_cab_z) and absf(camera_local.z - exterior_cab_z) < 2.0,
        "driver camera (z=%s) should sit inside the exterior's cab shell (z=%s)" % [camera_local.z, exterior_cab_z],
    )
    var camera_forward:Vector3 = -camera.global_basis.z
    var vehicle_forward:Vector3 = -vehicle.global_basis.z
    assert_true(
        camera_forward.dot(vehicle_forward) > 0.99,
        "driver camera should look along the vehicle's forward, got %s vs %s" % [camera_forward, vehicle_forward],
    )
