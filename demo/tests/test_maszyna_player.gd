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
## A track long enough for a vehicle past MaszynaPlayer's follow jump distance (10 km), and where the
## vehicles stand on it: two within it, one 20 km off [m]
const LONG_TRACK_LENGTH:float = 25000.0
const NEAR_OFFSET:float = 200.0
const SECOND_OFFSET:float = 5200.0
const FAR_OFFSET:float = 20200.0
## How high above the near vehicle the view stands before following - too far for a view [m]
const VIEW_HEIGHT:float = 2000.0
## Long enough for the following camera to have flown to its view (ExternalCamera3D.response 2/s) [s]
const FLIGHT_TIME:float = 3.0
## A view stands within this of its vehicle [m]
const VIEW_REACH:float = 100.0

var _vehicle:RailVehicle3D
var _second_vehicle:RailVehicle3D
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
    if is_instance_valid(_second_vehicle):
        free_rail_vehicle(_second_vehicle)
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
    _player = PLAYER_SCENE.instantiate()
    _player.auto_start = false
    add_child(_player)
    await wait_idle_frames(SETTLE_FRAMES)
    var vehicle:RID = _vehicle.get_rid()
    assert_true(vehicle.is_valid(), "the vehicle should have its controller")
    CabinSystem.vehicle_set_cabin_scene(vehicle, _cabin_scene())

    PlayerServer.player_enter_vehicle(vehicle)
    var cabin_camera:Camera3D = get_viewport().get_camera_3d()
    assert_eq(PlayerCameraServer.camera_get_mode(), PlayerCameraServer.CAMERA_MODE_CABIN)
    assert_not_null(CabinSystem.vehicle_get_cabin(vehicle), "taken over, the cab interior is shown")
    assert_eq(cabin_camera.get_parent(), CabinSystem.vehicle_get_cabin(vehicle), "the cab camera sits in it")

    var cabin:Cabin3D = CabinSystem.vehicle_get_cabin(vehicle)
    PlayerCameraServer.camera_toggle_cabin()
    assert_eq(get_viewport().get_camera_3d(), _player.free_camera, "F4 steps out to the free camera")
    assert_eq(CabinSystem.vehicle_get_cabin(vehicle), cabin, "outside, the cab stands with its controls")
    assert_eq(PlayerServer.player_get_vehicle(), vehicle, "outside, the player still drives the vehicle")

    PlayerCameraServer.camera_toggle_cabin()
    assert_eq(get_viewport().get_camera_3d(), cabin_camera, "F4 again, back in the cab")
    assert_eq(cabin_camera.get_parent(), cabin)

    PlayerServer.player_leave_vehicle()
    assert_null(CabinSystem.vehicle_get_cabin(vehicle), "let go, the player steps out of the cab")
    assert_eq(get_viewport().get_camera_3d(), _player.free_camera)
    # the cabs hidden are freed at the end of the frame
    await wait_idle_frames(1)


## Following takes the view of the vehicle followed, applied in full - from the view's own place,
## however far it was (a follow from where the camera stood kept that distance); another vehicle takes
## that one's view; one farther than the jump distance is not flown to but jumped beside
func test_the_vehicle_followed_is_looked_at_from_its_view() -> void:
    _track = build_track(TRACK_NAME, LONG_TRACK_LENGTH)
    _vehicle = build_rail_vehicle("PlayerFollowNear", TRACK_NAME, NEAR_OFFSET)
    _second_vehicle = build_rail_vehicle("PlayerFollowSecond", TRACK_NAME, SECOND_OFFSET)
    _far_vehicle = build_rail_vehicle("PlayerFollowFar", TRACK_NAME, FAR_OFFSET)
    _player = PLAYER_SCENE.instantiate()
    _player.auto_start = false
    add_child(_player)
    await wait_idle_frames(SETTLE_FRAMES)
    _player.free_camera.global_position = _vehicle.global_position + Vector3.UP * VIEW_HEIGHT

    PlayerCameraServer.camera_set_target(_vehicle.get_rid())
    PlayerCameraServer.camera_set_mode(PlayerCameraServer.CAMERA_MODE_FOLLOW)
    await wait_seconds(FLIGHT_TIME)
    assert_true(_player.external_camera.global_position.distance_to(_vehicle.global_position) < VIEW_REACH,
            "followed from its view, not from %.0f m up" % VIEW_HEIGHT)

    PlayerCameraServer.camera_set_target(_second_vehicle.get_rid())
    await wait_seconds(FLIGHT_TIME)
    assert_true(_player.external_camera.global_position.distance_to(_second_vehicle.global_position) < VIEW_REACH,
            "another vehicle: its view, not the first one's distance kept")

    PlayerCameraServer.camera_set_target(_far_vehicle.get_rid())
    assert_eq(_player.external_camera.global_transform,
            PlayerCameraServer.camera_get_show_transform(_far_vehicle.get_rid()),
            "a far one: the view jumps beside it, then flies to its view")
