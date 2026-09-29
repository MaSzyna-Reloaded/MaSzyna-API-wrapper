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
## A track long enough for a vehicle past MaszynaPlayer's follow jump distance, and where the two
## vehicles stand on it: one near the view, one 20 km off [m]
const LONG_TRACK_LENGTH:float = 25000.0
const NEAR_OFFSET:float = 200.0
const FAR_OFFSET:float = 20200.0
## MaszynaPlayer.FOLLOW_JUMP_DISTANCE_DEFAULT - the default of the Project Setting [m]
const FOLLOW_JUMP_DISTANCE:float = 10000.0
## How high above the near vehicle the view stands before following [m]
const VIEW_HEIGHT:float = 100.0

var _vehicle:RailVehicle3D
var _far_vehicle:RailVehicle3D
var _player:MaszynaPlayer
var _track:RID


## Out of the cab before the vehicle goes - the player's cab camera would go with it
func after_each() -> void:
    PlayerServer.player_leave_vehicle()
    PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FREE)
    PlayerCameraServer.camera_set_target(RID())
    if is_instance_valid(_vehicle):
        free_rail_vehicle(_vehicle)
    if is_instance_valid(_far_vehicle):
        free_rail_vehicle(_far_vehicle)
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


## A vehicle to follow far off is not flown to for minutes: the view jumps beside it, where the
## crosshair would put it, and follows from there; a near one is followed from where the view is
func test_a_far_vehicle_is_followed_from_beside_it() -> void:
    _track = build_track(TRACK_NAME, LONG_TRACK_LENGTH)
    _vehicle = build_rail_vehicle("PlayerFollowNear", TRACK_NAME, NEAR_OFFSET)
    _far_vehicle = build_rail_vehicle("PlayerFollowFar", TRACK_NAME, FAR_OFFSET)
    _player = PLAYER_SCENE.instantiate()
    _player.auto_start = false
    add_child(_player)
    await wait_idle_frames(SETTLE_FRAMES)
    _player.free_camera.global_position = _vehicle.global_position + Vector3.UP * VIEW_HEIGHT
    var view:Vector3 = _player.free_camera.global_position

    PlayerCameraServer.camera_set_target(_vehicle.get_rid())
    PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FOLLOW)
    assert_eq(_player.external_camera.global_position, view, "a near vehicle: followed from where the view is")

    PlayerCameraServer.camera_set_target(_far_vehicle.get_rid())
    var beside:Transform3D = PlayerCameraServer.camera_get_show_transform(_far_vehicle.get_rid())
    assert_true(view.distance_to(_far_vehicle.global_position) > FOLLOW_JUMP_DISTANCE)
    assert_eq(_player.external_camera.global_transform, beside, "a far one: the view jumps beside it")
