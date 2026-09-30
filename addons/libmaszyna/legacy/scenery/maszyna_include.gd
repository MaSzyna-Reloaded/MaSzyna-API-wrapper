@tool
extends Node3D
class_name MaszynaIncludeNode

signal loaded
## Emitted by SceneryInstancer before each loading stage (progress 0..1, stage description)
signal load_progress(progress:float, message:String)

## Milliseconds spent freeing content per frame while reloading
const CLEAR_BUDGET_MSEC:int = 8

var _dirty:bool = false
var _editor_dirty:bool = false

const SceneryEditor = preload("res://addons/libmaszyna/editor/scenery_toolbar/scenery_editor.gd")

## Changing filename does not reload the content - call load() explicitly.
@export var filename:String = ""
@export var parameters:Dictionary = {}
@export var context_rotate: Vector3 = Vector3.ZERO
@export var context_origin: Vector3 = Vector3.ZERO
@export var autoload:bool = true
@export var use_cache:bool = true
## Skin of a vehicle by its train_id, applied when the vehicles are built - the scenery cache
## holds what the .scn declares, these only override it for this load
@export var skin_overrides:Dictionary[String, String] = {}

## Off by default: loaded content gets no owner and stays unselectable in the editor (matches
## E3DModelInstance/MaszynaRailVehiclePhysicsNode's own default). Toggle via the "Edit SCN" editor toolbar
## button (addons/libmaszyna/editor/scenery_toolbar/) to make it inspectable/selectable while
## authoring - see scenery_instancer.gd's attach loop for what this actually changes.
@export var editable_in_editor:bool = false:
    set(x):
        if not x == editable_in_editor:
            editable_in_editor = x
            _editor_dirty = true

var _loading:bool = false
## The content of the file is here - the game's data read again loads it again
var _loaded:bool = false

## Tracks, traction and models are built directly against TrackServer/TrackRenderingServer/
## TractionRenderingServer/TractionServer/E3DRenderingServer/SignallingServer RIDs, not as scene nodes (see
## scenery_instancer.gd's instantiate()
## doc comment for why) - so unlike triangle children, they aren't cleaned up just by
## removing children from the tree. scenery_instancer.gd appends the RIDs it creates here;
## _free_owned_rids() releases them all, called before every reload and on exiting the tree.
var _track_rids:Array[RID] = []
var _track_render_rids:Array[RID] = []
var _traction_rids:Array[RID] = []
var _wire_power_rids:Array[RID] = []
var _power_source_rids:Array[RID] = []
var _e3d_rids:Array[RID] = []
var _signalling_system_rids:Array[RID] = []
var _triangle_chunk_rids:Array[RID] = []
var _launcher_rids:Array[RID] = []
var _pickable_rids:Array[RID] = []
var _event_rids:Array[RID] = []
var _memory_rids:Array[RID] = []
var _sound_rids:Array[RID] = []
var _event_track_rids:Array[RID] = []
var _isolated_rids:Array[RID] = []
var _event_isolated_rids:Array[RID] = []
var _driver_rids:Array[RID] = []
var _script_context_rids:Array[RID] = []

## Initial loading (autoload) is deferred to the first _process.
func _ready() -> void:
    _dirty = autoload


func _enter_tree() -> void:
    GameDataServer.data_reload_requested.connect(_on_data_reload_requested)


## A loaded scenery is the game's data - its tracks, events, drivers and vehicles are loaded again,
## from the data read again, once the load under way (if any) is done
func _on_data_reload_requested() -> void:
    if _loaded or _loading:
        _dirty = true


func _exit_tree() -> void:
    GameDataServer.data_reload_requested.disconnect(_on_data_reload_requested)
    # The planning thread calls back into GDScript (the owner's preload) and can be creating
    # rendering resources for the very RIDs freed below. It is stopped and joined here, while the
    # scripts still exist - the server's own destructor runs long after they are gone.
    SceneryInstancer.cancel_loading()
    SceneryStreamingServer.streaming_drain()
    _free_owned_rids()


## The ScenarioScriptServer context the scenery's scripts run in, invalid while nothing is loaded
func get_script_context() -> RID:
    return _script_context_rids[0] if _script_context_rids else RID()


## budget_msec > 0 spreads the freeing over frames, so whatever covers the screen (the loading
## spinner) keeps animating; 0 frees everything at once (leaving the tree)
func _free_owned_rids(budget_msec:int = 0) -> void:
    var groups:Array = [
        # first, so no queued event runs against what is freed after them - the scripts' own
        # events with them
        [_script_context_rids, ScenarioScriptServer.context_free],
        [_driver_rids, DriverSystem.driver_free],
        [_pickable_rids, SceneryHUDMouseServer.pickable_free],
        [_launcher_rids, ScenarioEventServer.launcher_free],
        [_event_rids, ScenarioEventServer.event_free],
        [_memory_rids, ScenarioEventServer.memory_free],
        [_sound_rids, ScenerySoundServer.sound_free],
        [_event_track_rids, ScenarioEventServer.track_clear_events],
        [_event_isolated_rids, ScenarioEventServer.isolated_clear_events],
        [_isolated_rids, TrackServer.isolated_free],
        [_track_render_rids, TrackRenderingServer.free_track],
        [_track_rids, TrackServer.track_free],
        [_traction_rids, TractionRenderingServer.free_traction],
        [_wire_power_rids, TractionServer.wire_free],
        [_power_source_rids, TractionServer.power_source_free],
        [_e3d_rids, E3DRenderingServer.instance_free],
        # after the instances, whose signal heads leave the system as they go
        [_signalling_system_rids, SignallingServer.system_free],
        [_triangle_chunk_rids, MaszynaSceneryChunkRenderingServer.free_chunk],
    ]
    var frame_start:int = Time.get_ticks_msec()
    for group:Array in groups:
        var rids:Array[RID] = group[0]
        var free_rid:Callable = group[1]
        # Taken off the list before it is freed, not after the whole loop: the budgeted path
        # awaits a frame in the middle, and leaving the tree during that await runs this again
        # from _exit_tree over the very same RIDs - a double free.
        while rids.size() > 0:
            var rid:RID = rids.pop_back()
            if rid.is_valid():
                free_rid.call(rid)
            if budget_msec > 0 and Time.get_ticks_msec() - frame_start >= budget_msec:
                await get_tree().process_frame
                frame_start = Time.get_ticks_msec()


## Nothing in here is worth simulating while it is being torn down, and a real scenery is
## hundreds of vehicles with their cabins and sounds, all running their own _process for the
## seconds the freeing takes. RailVehicleServer steps those vehicles from its own registry,
## outside this subtree, so disabling the subtree alone leaves the heaviest part running until the
## last vehicle is freed - it is stopped here too and restored once the content is gone.
func _clear_content(budget_msec:int = 0) -> void:
    process_mode = Node.PROCESS_MODE_DISABLED
    VehicleServer.stepping_set_enabled(false)
    # Streaming builds content on process_frame, and the freeing below yields a frame for its
    # budget - without this it streams new content into the very RIDs being freed, which the
    # RenderingServer reports as "Initializing already initialized RID" and then aborts.
    SceneryStreamingServer.streaming_set_enabled(false)
    await _free_owned_rids(budget_msec)
    var frame_start:int = Time.get_ticks_msec()
    for child:Node in get_children(true):
        child.free()
        if budget_msec > 0 and Time.get_ticks_msec() - frame_start >= budget_msec:
            await get_tree().process_frame
            frame_start = Time.get_ticks_msec()
    SceneryStreamingServer.streaming_set_enabled(true)
    VehicleServer.stepping_set_enabled(true)
    process_mode = Node.PROCESS_MODE_INHERIT


func load() -> void:
    if _loading:
        return

    _dirty = false
    _loading = true
    await _clear_content(CLEAR_BUDGET_MSEC)
    _loaded = false
    if filename:
        await _load_content()
        _loaded = true
    _loading = false
    if filename:
        loaded.emit()


func _load_content() -> void:
    await SceneryInstancer.instantiate(self, parameters)


func _process(delta: float) -> void:
    if _dirty and not _loading:
        _dirty = false
        _process_dirty(delta)
    if _editor_dirty:
        _editor_dirty = false
        if Engine.is_editor_hint():
            SceneryEditor.update_owners(self)
        
func _process_dirty(_delta: float) -> void:
    self.load()
