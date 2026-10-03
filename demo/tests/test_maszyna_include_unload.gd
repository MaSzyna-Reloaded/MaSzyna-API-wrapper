extends MaszynaGutTest

## A scenery that unloads - reloaded in place (another file set on the same node) or leaving the
## tree - stops what runs on its content before freeing it: the streaming's preloads in flight are
## joined, and its scenario is told to stop (MaszynaIncludeNode.unloading).

const FIXTURES_GAME_DIR:String = "res://tests/fixtures"
const SCENERY_FILE:String = "no_time.scn"
const LOAD_TIMEOUT:float = 30.0
## How long a preload in flight takes [ms] - longer than a reload of an empty scenery
const PRELOAD_MSEC:int = 500
## Frames given to the streaming to start the preload
const STREAMING_FRAMES:int = 60

var _previous_game_dir:String = ""
var _camera:Camera3D
var _stream_rids:Array[RID] = []
var _preload_mutex:Mutex = Mutex.new()
var _preloads_started:int = 0
var _preloads_finished:int = 0


func before_each() -> void:
    _previous_game_dir = UserSettings.get_maszyna_game_dir()
    UserSettings.save_maszyna_game_dir(FIXTURES_GAME_DIR)
    _camera = Camera3D.new()
    add_child_autoqfree(_camera)


func after_each() -> void:
    for stream_rid:RID in _stream_rids:
        SceneryStreamingServer.stream_free(stream_rid)
    _stream_rids.clear()
    SceneryStreamingServer.streaming_set_camera(null)
    UserSettings.save_maszyna_game_dir(_previous_game_dir)


## A reload frees the content its preloads in flight are loading: they are joined first, as on
## leaving the tree
func test_a_reload_in_place_joins_the_preloads_in_flight() -> void:
    var owner:int = SceneryStreamingServer.owner_create("unload_test", _slow_preload, _ignore_build, _ignore_clear)
    _stream_rids.append(SceneryStreamingServer.stream_register(owner, rid_from_int64(20000), Vector3.ZERO, 0.0))
    SceneryStreamingServer.streaming_set_camera(_camera)
    for frame:int in STREAMING_FRAMES:
        if _get_preloads_started() > 0:
            break
        await wait_idle_frames(1)
    assert_gt(_get_preloads_started(), 0, "the streaming started no preload")

    var include:MaszynaIncludeNode = MaszynaIncludeNode.new()
    include.autoload = false
    add_child_autofree(include)
    await include.load()

    _preload_mutex.lock()
    var in_flight:int = _preloads_started - _preloads_finished
    _preload_mutex.unlock()
    assert_eq(in_flight, 0, "a preload still ran after the content was cleared")


func test_unloading_is_announced_on_a_reload_and_on_leaving_the_tree() -> void:
    var include:MaszynaIncludeNode = MaszynaIncludeNode.new()
    include.autoload = false
    add_child(include)
    watch_signals(include)

    await include.load()
    assert_signal_emit_count(include, "unloading", 1, "a reload did not announce the unload")

    remove_child(include)
    assert_signal_emit_count(include, "unloading", 2, "leaving the tree did not announce the unload")
    include.free()


## The scenario runs only once started, and stops when its scenery unloads - as the game's world
## wires it (world.gd)
func test_a_scenario_runs_from_its_start_until_its_scenery_unloads() -> void:
    var scenery:MaszynaSceneryNode = MaszynaSceneryNode.new()
    scenery.filename = SCENERY_FILE
    add_child_autofree(scenery)
    await wait_for_signal(scenery.scenery_loaded, LOAD_TIMEOUT)

    var scenario:MaszynaLegacyScenario = MaszynaLegacyScenario.new()
    assert_false(scenario.get_script_context().is_valid(), "a scenario ran before it was started")
    await scenario.start(scenery)
    assert_true(scenario.get_script_context().is_valid(), "a started scenario has no script context")

    scenery.unloading.connect(scenario.stop)
    scenery.filename = ""
    await scenery.load()
    assert_false(scenario.get_script_context().is_valid(), "the scenario ran on after its scenery unloaded")


func _get_preloads_started() -> int:
    _preload_mutex.lock()
    var started:int = _preloads_started
    _preload_mutex.unlock()
    return started


## Streaming worker thread
func _slow_preload(_user_rid:RID) -> Variant:
    _preload_mutex.lock()
    _preloads_started += 1
    _preload_mutex.unlock()
    OS.delay_msec(PRELOAD_MSEC)
    _preload_mutex.lock()
    _preloads_finished += 1
    _preload_mutex.unlock()
    return null


func _ignore_build(_user_rid:RID, _preloaded:Variant) -> void:
    pass


func _ignore_clear(_user_rid:RID) -> void:
    pass
