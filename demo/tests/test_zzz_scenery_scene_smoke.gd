extends MaszynaGutTest


func test_demo_scenery_loading_scene_instantiates() -> void:
    var packed:PackedScene = load("res://demo_scenery_loading.tscn")
    assert_not_null(packed, "scene should load")
    var instance:Node3D = packed.instantiate()
    assert_not_null(instance, "scene should instantiate")
    var scenery:MaszynaSceneryNode = instance.get_node("MaszynaSceneryNode")
    # the demo scene has no filename (it opens the scenery selector) - autoload picks td.scn up
    # from _ready(), no need to call load() here
    scenery.filename = "td.scn"
    add_child(instance)
    await wait_for_signal(scenery.scenery_loaded, 120)

    # TrackServer is an engine singleton, not a node under /root - it has been since it moved
    # to C++, and looking it up by path is what AGENTS.md forbids
    var summary:Dictionary = TrackServer.topology_get_summary()
    assert_gt((summary["graphs"] as Array).size() + summary["orphaned_tracks_count"], 0, "at least one track should have registered with TrackServer")

    remove_child(instance)
    instance.free()
