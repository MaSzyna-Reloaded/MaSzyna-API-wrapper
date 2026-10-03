extends MaszynaGutTest

## The demo scene loading a scenery: EP07-424 of td.scn on a cut of its line (demo/tests/fixtures)
const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SCENERY:String = "ep07.scn"
const LOAD_TIMEOUT_SEC:float = 120.0
## How far from the middle of its vehicle the cab camera may be [m] - a vehicle's length at most
const CAB_TO_VEHICLE_MAX_DISTANCE:float = 30.0

var _previous_game_dir:String = ""


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)


func after_each() -> void:
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_demo_scenery_loading_scene_instantiates() -> void:
    var packed:PackedScene = load("res://demo_scenery_loading.tscn")
    assert_not_null(packed, "scene should load")
    var instance:Node3D = packed.instantiate()
    assert_not_null(instance, "scene should instantiate")
    # a scenery given to the scene starts at once, without the selector - its world is made for it
    instance.scenery = SCENERY
    add_child(instance)
    # TrackServer is an engine singleton, not a node under /root - it has been since it moved
    # to C++, and looking it up by path is what AGENTS.md forbids
    # the player gets its vehicle once the scenery has loaded
    await wait_until(func() -> bool: return PlayerServer.player_get_vehicle().is_valid(), LOAD_TIMEOUT_SEC)
    assert_true(_has_tracks(), "at least one track should have registered with TrackServer")
    # the loading screen goes only once the player sits in its vehicle and the scenery around it
    # is streamed in - not while the view is still where the menu left it
    var loading_screen:CanvasItem = instance.get_node("LoadingScreen")
    await wait_until(func() -> bool: return not loading_screen.visible, LOAD_TIMEOUT_SEC)
    var vehicle_position:Vector3 = RailVehicleRenderingServer.vehicle_get_transform(
            PlayerServer.player_get_vehicle()).origin
    assert_lt(SceneryStreamingServer.streaming_get_camera_position().distance_to(vehicle_position),
            CAB_TO_VEHICLE_MAX_DISTANCE, "the scenery is streamed around the cab the player sits in")
    assert_true(SceneryStreamingServer.area_is_ready(0), "the chunk the player is in is built")

    remove_child(instance)
    instance.free()


func _has_tracks() -> bool:
    var summary:Dictionary = TrackServer.topology_get_summary()
    return (summary["graphs"] as Array).size() + summary["orphaned_tracks_count"] > 0

