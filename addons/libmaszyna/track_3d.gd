@tool
extends Node3D
class_name Track3D

@export var track_id: String = "":
    set(value):
        if value == track_id:
            return
        track_id = value
        _sync_track()

@export var length: float = 100.0:
    set(value):
        if is_equal_approx(value, length):
            return
        length = value
        _sync_track()

var _track_rid := RID()
var _last_synced_position := Vector3.INF
var _last_synced_length := INF
var _last_synced_track_id := ""


func _enter_tree() -> void:
    set_notify_transform(true)
    set_notify_local_transform(true)
    set_process(Engine.is_editor_hint())
    _register_track()


func _exit_tree() -> void:
    set_process(false)
    if _track_rid != RID():
        TrackManager.remove_track(_track_rid)
        _track_rid = RID()
        _notify_train_set_previews()


func _notification(what: int) -> void:
    if what == NOTIFICATION_TRANSFORM_CHANGED or what == NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
        _sync_track()


func _process(_delta: float) -> void:
    if not Engine.is_editor_hint():
        return

    if (
        _last_synced_position != global_position
        or not is_equal_approx(_last_synced_length, length)
        or _last_synced_track_id != track_id
    ):
        _sync_track()


func _register_track() -> void:
    if _track_rid != RID():
        return

    _track_rid = TrackManager.add_track(length, global_position, track_id)
    _remember_synced_state()
    _notify_train_set_previews()


func _sync_track() -> void:
    if not is_inside_tree():
        return

    if _track_rid == RID():
        _register_track()
        return

    TrackManager.modify_track(_track_rid, length, global_position, track_id)
    _remember_synced_state()
    _notify_train_set_previews()


func _notify_train_set_previews() -> void:
    if not is_inside_tree():
        return

    get_tree().call_group_flags(SceneTree.GROUP_CALL_DEFERRED, &"train_set_3d_editor_preview", "_queue_editor_layout")


func _remember_synced_state() -> void:
    _last_synced_position = global_position
    _last_synced_length = length
    _last_synced_track_id = track_id
