@tool
extends Node3D
class_name MaszynaIncludeNode

signal loaded

var _dirty:bool = false
var _editor_dirty:bool = false
var _filename_changed_at:int = 0

const RELOAD_DEBOUNCE_MSEC:int = 500
const SceneryEditor = preload("res://addons/libmaszyna/editor/scenery_toolbar/scenery_editor.gd")

@export var filename:String = "":
    set(x):
        if not filename == x:
            _dirty = true
            filename = x
            _filename_changed_at = Time.get_ticks_msec()
@export var parameters:Dictionary = {}
@export var context_rotate: Vector3 = Vector3.ZERO
@export var context_origin: Vector3 = Vector3.ZERO
@export var autoload:bool = true

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

## Tracks and traction are built directly against TrackManager/TrackRenderingServer/
## TractionRenderingServer/TractionPowerServer RIDs, not as scene nodes (see
## scenery_instancer.gd's instantiate()
## doc comment for why) - so unlike triangle/model children, they aren't cleaned up just by
## removing children from the tree. scenery_instancer.gd appends the RIDs it creates here;
## _free_owned_rids() releases them all, called before every reload and on exiting the tree.
var _track_rids:Array[RID] = []
var _track_render_rids:Array[RID] = []
var _traction_rids:Array[RID] = []
var _wire_power_rids:Array[RID] = []
var _power_source_rids:Array[RID] = []

## Initial loading uses the same dirty processing as filename changes, without the debounce.
func _ready() -> void:
    _filename_changed_at = -RELOAD_DEBOUNCE_MSEC


func _exit_tree() -> void:
    _free_owned_rids()


func _free_owned_rids() -> void:
    for rid:RID in _track_render_rids:
        if rid.is_valid():
            TrackRenderingServer.free_track(rid)
    for rid:RID in _track_rids:
        if rid.is_valid():
            TrackManager.track_free(rid)
    for rid:RID in _traction_rids:
        if rid.is_valid():
            TractionRenderingServer.free_traction(rid)
    for rid:RID in _wire_power_rids:
        if rid.is_valid():
            TractionPowerServer.wire_free(rid)
    for rid:RID in _power_source_rids:
        if rid.is_valid():
            TractionPowerServer.power_source_free(rid)
    _track_rids.clear()
    _track_render_rids.clear()
    _traction_rids.clear()
    _wire_power_rids.clear()
    _power_source_rids.clear()

func _clear_content() -> void:
    _free_owned_rids()
    for child:Node in get_children(true):
        child.free()


func load() -> void:
    if _loading:
        return

    _dirty = false
    _loading = true
    _clear_content()
    if filename:
        _load_content()
    _loading = false
    if filename:
        loaded.emit()


func _load_content() -> void:
    SceneryInstancer.instantiate(self, parameters)


func _process(delta: float) -> void:
    if _dirty and Time.get_ticks_msec() - _filename_changed_at >= RELOAD_DEBOUNCE_MSEC:
        _dirty = false
        _process_dirty(delta)
    if _editor_dirty:
        _editor_dirty = false
        if Engine.is_editor_hint():
            SceneryEditor.update_owners(self)
        
func _process_dirty(_delta: float) -> void:
    if autoload or filename.is_empty():
        self.load()
