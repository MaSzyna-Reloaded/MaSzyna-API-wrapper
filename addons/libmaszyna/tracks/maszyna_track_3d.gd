@tool
extends VisualInstance3D
class_name MaszynaTrack3D

static var _rail_material = preload("res://addons/libmaszyna/materials/rail.material")

enum TrackEnvironment {FLAT, BRIDGE, TUNNEL, MOUNTAINS, CANYON, BANK}
enum TrackDamageFlagBit {NONE=0, DESTROYED=128}
enum SwitchState {STRAIGHT, DIVERGING}

signal switching_started(from_state: int, to_state: int)
signal switching_finished(state: int)

@export_group("Domain")
@export var track_name: String = "":
    set(x):
        if not track_name == x:
            track_name = x
            _dirty = true
@export var type: TrackManager.TrackType = TrackManager.TrackType.NORMAL:
    set(value):
        if type == value:
            return
        type = value
        _dirty = true
@export var environment:TrackEnvironment = TrackEnvironment.FLAT
@export_flags("Destroyed:128") var damage_flag:int = 0
@export var quality_flag:int
@export var friction:float = 0.0
@export var parameters:Dictionary = {}
@export var switch_state: SwitchState = SwitchState.STRAIGHT:
    set(value):
        if switch_state == value:
            return
        switch_state = value
        if _track_rid and TrackManager:
            TrackManager.set_switch_state(_track_rid, switch_state)

@export_group("Geometry")
@export var curve1:MaszynaTrackCurve:
    set(x):
        if curve1 and curve1.changed.is_connected(_mark_dirty):
            curve1.changed.disconnect(_mark_dirty)
        curve1 = x
        if curve1 and not curve1.changed.is_connected(_mark_dirty):
            curve1.changed.connect(_mark_dirty)
        _dirty = true
@export var curve2:MaszynaTrackCurve:
    set(x):
        if curve2 and curve2.changed.is_connected(_mark_dirty):
            curve2.changed.disconnect(_mark_dirty)
        curve2 = x
        if curve2 and not curve2.changed.is_connected(_mark_dirty):
            curve2.changed.connect(_mark_dirty)
        _dirty = true
@export var width: float = 1.6:
    set(value):
        if is_equal_approx(width, value):
            return
        width = value
        _dirty = true

@export_group("Visualisation")
@export var material1: String = "":
    set(value):
        if material1 == value:
            return
        material1 = value
        _dirty = true

@export var railprofile: String = "default":
    set(value):
        if railprofile == value:
            return
        railprofile = value
        _dirty = true
@export var material2: String = "":
    set(value):
        if material2 == value:
            return
        material2 = value
        _dirty = true

@export var trackbed_material: String = "":
    set(value):
        if trackbed_material == value:
            return
        trackbed_material = value
        _dirty = true

@export var rail_visible: bool = true:
    set(value):
        if rail_visible == value:
            return
        rail_visible = value
        _dirty = true

@export var ballast_visible: bool = true:
    set(value):
        if ballast_visible == value:
            return
        ballast_visible = value
        _dirty = true

@export var tex_length: float = 4.0:
    set(value):
        if is_equal_approx(tex_length, value):
            return
        tex_length = value
        _dirty = true

@export var tex_height: float = 0.0:
    set(value):
        if is_equal_approx(tex_height, value):
            return
        tex_height = value
        _dirty = true

@export var tex_width: float = 0.0:
    set(value):
        if is_equal_approx(tex_width, value):
            return
        tex_width = value
        _dirty = true

@export var tex_slope: float = 0.0:
    set(value):
        if is_equal_approx(tex_slope, value):
            return
        tex_slope = value
        _dirty = true

@export_group("Legacy or unused")
@export var length:float = 0.0  # unused, Rainsted fills with mileage of the first point


## Minimum distance from camera required to render the track.
## When set to [code]0.0[/code], there is no near distance limit.

var _track_rid: RID
var _track_render_rid: RID
var _dirty = false

const DEBOUNCE_TIME = 0.1
var _last_debouce:float

func _enter_tree():
    if curve1 and not curve1.changed.is_connected(_mark_dirty):
        curve1.changed.connect(_mark_dirty)
    if curve2 and not curve2.changed.is_connected(_mark_dirty):
        curve2.changed.connect(_mark_dirty)
    if TrackManager and not TrackManager.switch_state_changed.is_connected(_on_track_manager_switch_state_changed):
        TrackManager.switch_state_changed.connect(_on_track_manager_switch_state_changed)
    if TrackManager and not TrackManager.switch_offset_updated.is_connected(_on_track_manager_switch_offset_updated):
        TrackManager.switch_offset_updated.connect(_on_track_manager_switch_offset_updated)
    if TrackManager and not TrackManager.switching_started.is_connected(_on_track_manager_switching_started):
        TrackManager.switching_started.connect(_on_track_manager_switching_started)
    if TrackManager and not TrackManager.switching_finished.is_connected(_on_track_manager_switching_finished):
        TrackManager.switching_finished.connect(_on_track_manager_switching_finished)
    _create_track()
    _update_track_data()
    _dirty = true


func _exit_tree():
    _free_track()
    if curve1 and curve1.changed.is_connected(_mark_dirty):
        curve1.changed.disconnect(_mark_dirty)
    if curve2 and curve2.changed.is_connected(_mark_dirty):
        curve2.changed.disconnect(_mark_dirty)
    if TrackManager and TrackManager.switch_state_changed.is_connected(_on_track_manager_switch_state_changed):
        TrackManager.switch_state_changed.disconnect(_on_track_manager_switch_state_changed)
    if TrackManager and TrackManager.switch_offset_updated.is_connected(_on_track_manager_switch_offset_updated):
        TrackManager.switch_offset_updated.disconnect(_on_track_manager_switch_offset_updated)
    if TrackManager and TrackManager.switching_started.is_connected(_on_track_manager_switching_started):
        TrackManager.switching_started.disconnect(_on_track_manager_switching_started)
    if TrackManager and TrackManager.switching_finished.is_connected(_on_track_manager_switching_finished):
        TrackManager.switching_finished.disconnect(_on_track_manager_switching_finished)

func _mark_dirty():
    _dirty = true


func toggle_switch() -> void:
    if not _track_rid or not TrackManager:
        return
    var next_state = SwitchState.STRAIGHT if switch_state == SwitchState.DIVERGING else SwitchState.DIVERGING
    TrackManager.set_switch_state(_track_rid, next_state)


func _on_track_manager_switching_started(track_rid: RID, from_state: int, to_state: int) -> void:
    if not track_rid == _track_rid:
        return
    switching_started.emit(from_state, to_state)


func _on_track_manager_switching_finished(track_rid: RID, state: int) -> void:
    if not track_rid == _track_rid:
        return
    switching_finished.emit(state)


func _on_track_manager_switch_offset_updated(track_rid: RID, _offset: float) -> void:
    if not track_rid == _track_rid:
        return
    if TrackRenderingServer and _track_render_rid:
        var track = TrackManager.get_track(_track_rid)
        if track:
            TrackRenderingServer.set_switch_blade_offsets(_track_render_rid, track.switch_f_offset1, track.switch_f_offset2)


func _on_track_manager_switch_state_changed(track_rid: RID, switch_state: int) -> void:
    if not track_rid == _track_rid:
        return
    if not self.switch_state == switch_state:
        self.switch_state = switch_state


func _ready():
    set_process(true)

func _notification(what):
    match what:
        NOTIFICATION_VISIBILITY_CHANGED:
            if _track_render_rid and _track_render_rid.is_valid() and TrackRenderingServer:
                TrackRenderingServer.set_track_visible(_track_render_rid, visible)


func _process(delta: float) -> void:
    if _dirty:
        _process_dirty(delta)
        return


func _process_dirty(_delta: float) -> void:
    _dirty = false
    var _current := (Time.get_ticks_msec()/1000.0)
    if _current - _last_debouce > DEBOUNCE_TIME:
        _update_track_rendering()
        _last_debouce = _current

func _get_global_aabb() -> AABB:
    var points: Array[Vector3] = []

    if curve1:
        points.append(curve1.p1)
        points.append(curve1.p2)

    if curve2:
        points.append(curve2.p1)
        points.append(curve2.p2)

    if points.is_empty():
        return AABB()

    var aabb := AABB(points[0], Vector3.ZERO)

    for p in points:
        aabb = aabb.expand(p)
    return aabb


func _get_aabb() -> AABB:
    var points: Array[Vector3] = []

    if curve1:
        points.append(to_local(curve1.p1))
        points.append(to_local(curve1.p2))

    if curve2:
        points.append(to_local(curve2.p1))
        points.append(to_local(curve2.p2))

    if points.is_empty():
        return AABB()

    var aabb := AABB(points[0], Vector3.ZERO)

    for p in points:
        aabb = aabb.expand(p)

    var half_width := width * 0.5
    half_width += 1.0  # FIXME: hardcoded / guess

    aabb.position.x -= half_width
    aabb.position.z -= half_width
    aabb.size.x += half_width * 2
    aabb.size.z += half_width * 2
    aabb.position.y -= 0.2  # FIXME: hardcoded / guess
    aabb.size.y += 0.4  # FIXME: hardcoded / guess

    return aabb


func _update_track_data() -> void:
    if not TrackManager or not TrackRenderingServer:
        return
    if not _track_rid or not _track_rid.is_valid() or not curve1:
        return

    self.global_position = _get_global_aabb().get_center()
    TrackManager.update_track(
        _track_rid,
        type,
        track_name,
        curve1,
        curve2,
        width,
    )
    TrackManager.set_switch_state(_track_rid, switch_state)


func _update_track_rendering():
    if not TrackManager or not TrackRenderingServer:
        return
    if not _track_rid or not _track_rid.is_valid() or not _track_render_rid or not _track_render_rid.is_valid() or not curve1:
        return

    _update_track_data()
    TrackRenderingServer.set_track_render_options(
        _track_render_rid,
        tex_length,
        tex_height,
        tex_width,
        tex_slope,
        material1,
        material2,
        trackbed_material,
        railprofile,
        rail_visible,
        ballast_visible
    )
    TrackRenderingServer.rebuild_track(_track_render_rid)
    TrackRenderingServer.set_track_scenario(_track_render_rid, get_world_3d().scenario)
    TrackRenderingServer.set_track_visible(_track_render_rid, visible)


func _is_switch() -> bool:
    return curve2 != null


func _create_track() -> void:
    if not TrackManager or not TrackRenderingServer:
        return

    _free_track()
    _track_rid = TrackManager.register_track()
    _track_render_rid = TrackRenderingServer.create_track(_track_rid)
    TrackRenderingServer.set_track_scenario(_track_render_rid, get_world_3d().scenario)


func _free_track() -> void:
    if TrackRenderingServer and _track_render_rid and _track_render_rid.is_valid():
        TrackRenderingServer.free_track(_track_render_rid)
    if TrackManager and _track_rid and _track_rid.is_valid():
        TrackManager.unregister_track(_track_rid)
    _track_rid = RID()
    _track_render_rid = RID()
