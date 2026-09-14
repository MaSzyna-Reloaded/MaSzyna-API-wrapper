extends MaszynaGutTest

var _previous_game_dir:String = ""


func before_each():
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir("/mnt/ArchiwumX/Games/Maszyna")


func after_each():
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


func test_demo_scenery_loading_scene_instantiates() -> void:
    var packed:PackedScene = load("res://demo_scenery_loading.tscn")
    assert_not_null(packed, "scene should load")
    var instance:Node3D = packed.instantiate()
    assert_not_null(instance, "scene should instantiate")
    add_child(instance)
    await wait_idle_frames(3)

    # MaszynaIncludeNode.autoload fires from _ready() (both in-editor and at runtime) - no need
    # to call load() again here, and doing so would just race the automatic load.
    var scenery:MaszynaSceneryNode = instance.get_node("MaszynaSceneryNode")
    assert_gt(scenery._track_rids.size(), 0, "scenery load should have created at least one track RID")

    var track_manager:Node = get_tree().root.get_node("TrackManager")
    var summary:Dictionary = track_manager.topology_get_summary()
    assert_gt((summary["graphs"] as Array).size() + summary["orphaned_tracks_count"], 0, "at least one track should have registered with TrackManager")

    remove_child(instance)
    instance.free()
