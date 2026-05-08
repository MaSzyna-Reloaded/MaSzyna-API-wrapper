@tool
extends TrackNormal3D
class_name TrackSwitch3D

signal switching_started(from_track: int, to_track: int)
signal switching_finished(active_track: int)

func _init() -> void:
    type = TrackManager.TrackType.TRACK_SWITCH

@export var diverging_curve:MaszynaTrackCurve:
    set(x):
        if diverging_curve and diverging_curve.changed.is_connected(_mark_dirty):
            diverging_curve.changed.disconnect(_mark_dirty)
        diverging_curve = x
        if diverging_curve and not diverging_curve.changed.is_connected(_mark_dirty):
            diverging_curve.changed.connect(_mark_dirty)
        _dirty = true

@export var active_track: TrackManager.SwitchTrack = TrackManager.SwitchTrack.TRACK_COMMON:
    set(value):
        if active_track == value:
            return
        active_track = value
        if _track_rid and TrackManager:
            TrackManager.switch_set_active_track(_track_rid, active_track)


func toggle_switch() -> void:
    if not _track_rid or not TrackManager:
        return
    var next_track = (
        TrackManager.SwitchTrack.TRACK_COMMON
        if active_track == TrackManager.SwitchTrack.TRACK_DIVERGING
        else TrackManager.SwitchTrack.TRACK_DIVERGING
    )
    TrackManager.switch_set_active_track(_track_rid, next_track)

func _update_track_curve() -> void:
    TrackManager.track_update_curves(_track_rid, curve, diverging_curve)

func _update_track_data() -> void:
    super._update_track_data()
    TrackManager.switch_set_active_track(_track_rid, active_track)

func _enter_tree():
    super._enter_tree()
    if TrackManager:
        TrackManager.switch_active_track_changed.connect(_on_track_manager_switch_active_track_changed)
        TrackManager.switch_offset_updated.connect(_on_track_manager_switch_offset_updated)
        TrackManager.switching_started.connect(_on_track_manager_switching_started)
        TrackManager.switching_finished.connect(_on_track_manager_switching_finished)

func _exit_tree():
    if diverging_curve and diverging_curve.changed.is_connected(_mark_dirty):
        diverging_curve.changed.disconnect(_mark_dirty)
    if TrackManager:
        TrackManager.switch_active_track_changed.disconnect(_on_track_manager_switch_active_track_changed)
        TrackManager.switch_offset_updated.disconnect(_on_track_manager_switch_offset_updated)
        TrackManager.switching_started.disconnect(_on_track_manager_switching_started)
        TrackManager.switching_finished.disconnect(_on_track_manager_switching_finished)
    super._exit_tree()

func _get_aabb_points() -> Array[Vector3]:
    var points = super._get_aabb_points()
    if diverging_curve:
        points.append(diverging_curve.p1)
        points.append(diverging_curve.p2)
    return points

func _on_track_manager_switching_started(track_rid: RID, from_track: int, to_track: int) -> void:
    if not track_rid == _track_rid:
        return
    switching_started.emit(from_track, to_track)

func _on_track_manager_switching_finished(track_rid: RID, active_track_value: int) -> void:
    if not track_rid == _track_rid:
        return
    switching_finished.emit(active_track_value)

func _on_track_manager_switch_offset_updated(track_rid: RID, _offset: float) -> void:
    if not track_rid == _track_rid:
        return
    if TrackRenderingServer and _track_render_rid:
        if TrackManager.track_exists(_track_rid):
            TrackRenderingServer.set_switch_blade_offsets(
                _track_render_rid,
                TrackManager.switch_get_f_offset1(_track_rid),
                TrackManager.switch_get_f_offset2(_track_rid)
            )

func _on_track_manager_switch_active_track_changed(track_rid: RID, active_track_value: int) -> void:
    if not track_rid == _track_rid:
        return
    if not self.active_track == active_track_value:
        self.active_track = active_track_value
