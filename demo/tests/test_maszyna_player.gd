extends MaszynaGutTest

## The player follows PlayerServer into a vehicle and PlayerCameraServer's view: the vehicle's cab
## interior stands, with the player's cab camera in it, while the player drives the vehicle - also
## while looking from outside, as the cab's widgets take the keys - and goes when it is let go.

const PLAYER_SCENE:PackedScene = preload("res://addons/libmaszyna/player/player.tscn")
## Enough frames for the vehicle to take its controller and stand on its track
const SETTLE_FRAMES:int = 4
const TRACK_NAME:String = "player_cabin_test"
const TRACK_LENGTH:float = 200.0
const TRACK_OFFSET:float = 100.0

var _vehicle:RailVehicle3D
var _player:MaszynaPlayer
var _track:RID


## Out of the cab before the vehicle goes - the player's cab camera would go with it
func after_each() -> void:
    PlayerServer.player_leave_vehicle()
    if is_instance_valid(_vehicle):
        free_rail_vehicle(_vehicle)
    if is_instance_valid(_player):
        _player.free()
    TrackServer.track_free(_track)
    TrackServer.topology_rebuild()


## A bare cab: its interior is not what is tested
static func _cabin_scene() -> PackedScene:
    var cabin:Cabin3D = Cabin3D.new()
    var packed:PackedScene = PackedScene.new()
    packed.pack(cabin)
    cabin.free()
    return packed


func test_the_cab_interior_stands_while_the_vehicle_is_driven() -> void:
    _track = build_track(TRACK_NAME, TRACK_LENGTH)
    _vehicle = build_rail_vehicle("PlayerCabinTest", TRACK_NAME, TRACK_OFFSET)
    _vehicle.cabin_scene = _cabin_scene()
    _player = PLAYER_SCENE.instantiate()
    _player.auto_start = false
    add_child(_player)
    await wait_idle_frames(SETTLE_FRAMES)
    var vehicle:RID = _vehicle.get_rid()
    assert_true(vehicle.is_valid(), "the vehicle should have its controller")

    PlayerServer.player_enter_vehicle(vehicle)
    var cabin_camera:Camera3D = get_viewport().get_camera_3d()
    assert_eq(PlayerCameraServer.camera_get_mode(), PlayerCameraServer.CAMERA_MODE_CABIN)
    assert_not_null(_vehicle.get_cabin(), "taken over, the cab interior is shown")
    assert_eq(cabin_camera.get_parent(), _vehicle.get_cabin(), "the cab camera sits in it")

    var cabin:Cabin3D = _vehicle.get_cabin()
    PlayerCameraServer.camera_toggle_cabin()
    assert_eq(get_viewport().get_camera_3d(), _player.free_camera, "F4 steps out to the free camera")
    assert_eq(_vehicle.get_cabin(), cabin, "outside, the cab stands with its controls")
    assert_eq(PlayerServer.player_get_vehicle(), vehicle, "outside, the player still drives the vehicle")

    PlayerCameraServer.camera_toggle_cabin()
    assert_eq(get_viewport().get_camera_3d(), cabin_camera, "F4 again, back in the cab")
    assert_eq(cabin_camera.get_parent(), cabin)

    PlayerServer.player_leave_vehicle()
    assert_null(_vehicle.get_cabin(), "let go, the player steps out of the cab")
    assert_eq(get_viewport().get_camera_3d(), _player.free_camera)
    # the cabs hidden are freed at the end of the frame
    await wait_idle_frames(1)
