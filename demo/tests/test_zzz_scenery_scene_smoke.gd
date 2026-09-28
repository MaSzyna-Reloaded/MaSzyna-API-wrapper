extends MaszynaGutTest

## The demo scene loading a scenery: EP07-424 of td.scn on a cut of its line (demo/tests/fixtures)
const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SCENERY:String = "ep07.scn"

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
    var scenery:MaszynaSceneryNode = instance.get_node("MaszynaSceneryNode")
    # the demo scene has no filename (it opens the scenery selector) - autoload picks the scenery
    # up from _ready(), no need to call load() here
    scenery.filename = SCENERY
    add_child(instance)
    await wait_for_signal(scenery.scenery_loaded, 120)

    # TrackServer is an engine singleton, not a node under /root - it has been since it moved
    # to C++, and looking it up by path is what AGENTS.md forbids
    var summary:Dictionary = TrackServer.topology_get_summary()
    assert_gt((summary["graphs"] as Array).size() + summary["orphaned_tracks_count"], 0, "at least one track should have registered with TrackServer")

    remove_child(instance)
    instance.free()
