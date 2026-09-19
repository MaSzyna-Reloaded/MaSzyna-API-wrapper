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

## Off by default: loaded content gets no owner and stays unselectable in the editor (matches
## E3DModelInstance/FIZTrainController's own default). Toggle via the "Edit SCN" editor toolbar
## button (addons/libmaszyna/editor/scenery_toolbar/) to make it inspectable/selectable while
## authoring - see scenery_instancer.gd's attach loop for what this actually changes.
@export var editable_in_editor:bool = false:
    set(x):
        if not x == editable_in_editor:
            editable_in_editor = x
            _editor_dirty = true

var _loading:bool = false

## Tracks, traction and models are built directly against TrackManager/TrackRenderingServer/
## TractionRenderingServer/TractionPowerServer/E3DRenderingServer RIDs, not as scene nodes (see
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

## Initial loading (autoload) is deferred to the first _process.
func _ready() -> void:
    _dirty = autoload


func _exit_tree() -> void:
    _free_owned_rids()


## budget_msec > 0 spreads the freeing over frames, so whatever covers the screen (the loading
## spinner) keeps animating; 0 frees everything at once (leaving the tree)
func _free_owned_rids(budget_msec:int = 0) -> void:
    var groups:Array = [
        [_track_render_rids, TrackRenderingServer.free_track],
        [_track_rids, TrackManager.track_free],
        [_traction_rids, TractionRenderingServer.free_traction],
        [_wire_power_rids, TractionPowerServer.wire_free],
        [_power_source_rids, TractionPowerServer.power_source_free],
        [_e3d_rids, E3DRenderingServer.instance_free],
    ]
    var frame_start:int = Time.get_ticks_msec()
    for group:Array in groups:
        var rids:Array[RID] = group[0]
        var free_rid:Callable = group[1]
        for rid:RID in rids:
            if rid.is_valid():
                free_rid.call(rid)
            if budget_msec > 0 and Time.get_ticks_msec() - frame_start >= budget_msec:
                await get_tree().process_frame
                frame_start = Time.get_ticks_msec()
        rids.clear()


func _clear_content(budget_msec:int = 0) -> void:
    await _free_owned_rids(budget_msec)
    var frame_start:int = Time.get_ticks_msec()
    for child:Node in get_children(true):
        child.free()
        if budget_msec > 0 and Time.get_ticks_msec() - frame_start >= budget_msec:
            await get_tree().process_frame
            frame_start = Time.get_ticks_msec()


func load() -> void:
    if _loading:
        return

    _dirty = false
    _loading = true
    await _clear_content(CLEAR_BUDGET_MSEC)
    if filename:
        await _load_content()
    _loading = false
    if filename:
        loaded.emit()


func _load_content() -> void:
    await SceneryInstancer.instantiate(self, parameters)


func _process(delta: float) -> void:
    if _dirty:
        _dirty = false
        _process_dirty(delta)
    if _editor_dirty:
        _editor_dirty = false
        if Engine.is_editor_hint():
            SceneryEditor.update_owners(self)
        
func _process_dirty(_delta: float) -> void:
    self.load()
