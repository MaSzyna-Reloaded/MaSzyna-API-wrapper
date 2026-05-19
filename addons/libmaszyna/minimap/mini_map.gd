@tool
extends Control

@export var track_color: Color = Color.WHITE
@export var label_color: Color = Color.YELLOW
@export var switch_active_color: Color = Color.YELLOW
@export var background_color: Color = Color(0, 0, 0, 0.5)
@export var zoom: float = 1.0:
    set(v):
        zoom = v
        queue_redraw()
@export var view_offset: Vector2 = Vector2.ZERO:
    set(v):
        view_offset = v
        queue_redraw()
@export var follow_camera_rotation: bool = true:
    set(v):
        follow_camera_rotation = v
        queue_redraw()

var _font: Font = ThemeDB.fallback_font
const TRACK_SWITCH_HANDLE_SCRIPT = preload("./switch_handle.gd")
var _switch_handles: Dictionary = {} # RID -> Control
@onready var _ui_container = $UIContainer

func _ready() -> void:
    mouse_filter = Control.MOUSE_FILTER_STOP
    clip_contents = true

    resized.connect(_sync_switch_handles)

    $UIContainer/BtnZoomIn.pressed.connect(func(): zoom *= 1.2)

    $UIContainer/BtnZoomOut.pressed.connect(func(): zoom /= 1.2)
    $UIContainer/BtnReset.pressed.connect(func(): zoom = 1.0; view_offset = Vector2.ZERO)

    var timer = Timer.new()
    timer.wait_time = 0.1
    timer.timeout.connect(_on_map_update)
    add_child(timer)
    timer.start()

    if TrackManager:
        if TrackManager.has_signal("switch_state_changed") and not TrackManager.switch_state_changed.is_connected(_on_switch_changed):
            TrackManager.switch_state_changed.connect(_on_switch_changed)
        if TrackManager.has_signal("topology_changed") and not TrackManager.topology_changed.is_connected(_on_topology_changed):
            TrackManager.topology_changed.connect(_on_topology_changed)

func _exit_tree() -> void:
    if TrackManager:
        if TrackManager.has_signal("switch_state_changed") and TrackManager.switch_state_changed.is_connected(_on_switch_changed):
            TrackManager.switch_state_changed.disconnect(_on_switch_changed)
        if TrackManager.has_signal("topology_changed") and TrackManager.topology_changed.is_connected(_on_topology_changed):
            TrackManager.topology_changed.disconnect(_on_topology_changed)

func _on_map_update() -> void:
    if is_visible_in_tree():
        _sync_switch_handles()
        queue_redraw()

func _on_topology_changed() -> void:
    _sync_switch_handles()
    queue_redraw()

func _on_switch_changed(_rid: RID, _state: int) -> void:
    _sync_switch_handles()
    queue_redraw()

func _sync_switch_handles() -> void:
    if not TrackManager or not is_visible_in_tree(): return
    var cam = _get_cam_data()
    var current_rids = TrackManager.get_track_rids()
    var current_rids_map = {}
    for rid in current_rids: current_rids_map[rid] = true

    for rid in _switch_handles.keys():
        if not rid in current_rids_map:
            _switch_handles[rid].queue_free()
            _switch_handles.erase(rid)

    for rid in current_rids:
        var track := TrackManager.get_track(rid)
        
        if track and track.curve2: # Is a switch
            if not _switch_handles.has(rid):
                var sh = _create_switch_handle(rid)
                add_child(sh)
                _switch_handles[rid] = sh
            var sh = _switch_handles[rid]
            var common_idx = track.switch_common_endpoint_index
            if common_idx >= 0 and common_idx < track.get_endpoints().size():
                sh.position = _world_to_view_centered(track.get_endpoints()[common_idx], cam.pos, cam.rot) - sh.size * 0.5
                sh.visible = true
                sh.modulate = Color.WHITE if track.switch_state == TrackManager.SwitchState.COMMON else Color.GRAY
            else:
                sh.visible = false
        elif _switch_handles.has(rid):
            _switch_handles[rid].visible = false

func _create_switch_handle(rid: RID) -> Control:
    var h = TRACK_SWITCH_HANDLE_SCRIPT.new()
    h.track_rid = rid
    h.viewer = self
    h.size = Vector2(15, 15)
    h.mouse_filter = Control.MOUSE_FILTER_STOP
    return h
func _get_cam_data() -> Dictionary:
    var cam = get_viewport().get_camera_3d()
    var pos = Vector2.ZERO
    var rot = 0.0
    if cam:
        pos = Vector2(cam.global_position.x, cam.global_position.z)
        if follow_camera_rotation:
            rot = cam.global_transform.basis.get_euler().y
    return {"pos": pos, "rot": rot}

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, size), background_color)
    if not TrackManager: return
    var cam = _get_cam_data()

    var extent = (_view_to_world_centered(size, cam.pos, cam.rot) - _view_to_world_centered(Vector2.ZERO, cam.pos, cam.rot)).length() * 0.75
    var visible_rect = Rect2(cam.pos - Vector2(extent, extent), Vector2(extent * 2, extent * 2)).grow(50.0 / zoom)

    for rid in TrackManager.get_tracks_in_aabb(visible_rect):
        var curve1 := TrackManager.get_track_curve1(rid)
        var curve2 := TrackManager.get_track_curve2(rid)
        var switch_state := TrackManager.get_switch_state(rid)
        var track_name := TrackManager.get_track_name(rid)
        
        _draw_track_curves(curve1, curve2, switch_state, cam.pos, cam.rot)
        _draw_track_label(track_name, curve1, cam.pos, cam.rot)
    _draw_trains(visible_rect, cam.pos, cam.rot)

func _draw_trains(visible_rect: Rect2, cam_pos: Vector2, cam_rot: float) -> void:
    if not Engine.is_editor_hint() and TrainSystem:
        for train_id in TrainSystem.get_train_ids_in_rect(visible_rect):
            var controller = TrainSystem.get_train(train_id)
            if not controller: continue

            var train_node = controller.get_parent()
            while train_node and not train_node is Node3D:
                train_node = train_node.get_parent()
            if not train_node: continue

            var view_pos = _world_to_view_centered(train_node.global_position, cam_pos, cam_rot)
            var square_size = 10.0
            draw_rect(Rect2(view_pos - Vector2(square_size / 2, square_size / 2), Vector2(square_size, square_size)), Color.GREEN)

            var font_size = 12
            draw_string(_font, view_pos + Vector2(square_size * 0.7, font_size * 0.3), train_id, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color.GREEN)

func _draw_track_curves(
    curve1: MaszynaTrackCurve,
    curve2: MaszynaTrackCurve,
    switch_state: TrackManager.SwitchState,
    cam_pos: Vector2,
    cam_rot: float
) -> void:
    if curve1:
        _draw_curve(curve1, switch_active_color if switch_state == 0 else track_color, cam_pos, cam_rot)
    if curve2:
        _draw_curve(curve2, switch_active_color if switch_state != 0 else track_color, cam_pos, cam_rot)

func _draw_track_label(track_name: String, curve1: MaszynaTrackCurve, cam_pos: Vector2, cam_rot: float) -> void:
    if not track_name == "" or not curve1: return
    var view_pos = _world_to_view_centered(_sample_bezier(curve1, 0.5), cam_pos, cam_rot)
    var font_size = 12
    draw_string(_font, view_pos - _font.get_string_size(track_name, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size) * 0.5 + Vector2(0, font_size * 0.25), track_name, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, label_color)

func _draw_curve(curve: MaszynaTrackCurve, color: Color, cam_pos: Vector2, cam_rot: float) -> void:
    if not curve: return
    var points: PackedVector2Array = []
    for i in range(17):
        points.append(_world_to_view_centered(_sample_bezier(curve, float(i) / 16.0), cam_pos, cam_rot))
    draw_polyline(points, color, 1.0, true)

func _sample_bezier(curve: MaszynaTrackCurve, t: float) -> Vector3:
    var p0 = curve.p1
    var p1 = curve.p1 + curve.c1
    var p2 = curve.p2 + curve.c2
    var p3 = curve.p2
    return p0.lerp(p1, t).lerp(p1.lerp(p2, t), t).lerp(p1.lerp(p2, t).lerp(p2.lerp(p3, t), t), t)

func _view_to_world_centered(view_pos: Vector2, cam_pos: Vector2, cam_rot: float) -> Vector3:
    var rel_pos = (view_pos - size * 0.5 - view_offset) / zoom
    if cam_rot != 0.0: rel_pos = rel_pos.rotated(-cam_rot)
    var p = rel_pos + cam_pos
    return Vector3(p.x, 0, p.y)

func _world_to_view_centered(pos: Vector3, cam_pos: Vector2, cam_rot: float) -> Vector2:
    var rel_pos = (Vector2(pos.x, pos.z) - cam_pos)
    if cam_rot != 0.0: rel_pos = rel_pos.rotated(cam_rot)
    return rel_pos * zoom + size * 0.5 + view_offset
