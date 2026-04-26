@tool
extends Path3D
class_name MaszynaTrack3D

var _track_id:RID
var _virtual_track_rid:RID

@export var track_id: String = "":
    set(x):
        if x == track_id:
            return
        track_id = x
        _request_track_update()

@export_group("Rails")
@export var rail_spacing: float = 1.6:
    set(x):
        rail_spacing = x
        _request_track_update()

@export_group("Sleepers")
@export var sleeper_model_name: String = "":
    set(x):
        sleeper_model_name = x
        _request_track_update()

@export var sleeper_model_skin: String = "":
    set(x):
        sleeper_model_skin = x
        _request_track_update()

@export var sleeper_spacing: float = 1.0:
    set(x):
        sleeper_spacing = x
        _request_track_update()

@export var sleeper_height: float = 0.4:
    set(x):
        sleeper_height = x
        _request_track_update()

@export var rail_height: float = 0.16:
    set(x):
        rail_height = x
        _request_track_update()

@export_group("Ballast")
@export var ballast_enabled: bool = true:
    set(x):
        ballast_enabled = x
        _request_track_update()

@export var ballast_texture: String = "":
    set(x):
        ballast_texture = x
        _request_track_update()

@export var ballast_uv_scale: float = 1.0:
    set(x):
        ballast_uv_scale = x
        _request_track_update()

@export var ballast_width_tiling: float = 1.0:
    set(x):
        ballast_width_tiling = x
        _request_track_update()

@export var ballast_length_tiling: float = 1.0:
    set(x):
        ballast_length_tiling = x
        _request_track_update()

@export var ballast_height: float = 0.4:
    set(x):
        ballast_height = x
        _request_track_update()

@export var ballast_offset: float = 0.0:
    set(x):
        ballast_offset = x
        _request_track_update()


@export_group("Geometry")
@export var curve_precision: float = 0.5:
    set(x):
        curve_precision = x
        _request_track_update()

@export var auto_smooth: bool = true:
    set(x):
        auto_smooth = x
        _request_track_update()

@export var min_turn_radius: float = 50.0:
    set(x):
        min_turn_radius = x
        _request_track_update()

@export var banking_intensity: float = 0.0:
    set(x):
        banking_intensity = x
        _request_track_update()

@export_group("Connectivity")
@export var next_track: NodePath:
    set(x):
        if x == next_track:
            return
        next_track = x
        _request_track_update()
@export var previous_track: NodePath:
    set(x):
        if x == previous_track:
            return
        previous_track = x
        _request_track_update()

var _track_rid: int = 0
var _dirty = false
var _is_updating = false
var _update_timer = 0.0
const DEBOUNCE_TIME = 0.05

func _enter_tree():
    _ensure_curve()
    _ensure_track()
    _ensure_virtual_track()
    set_notify_transform(true)
    set_notify_local_transform(true)

func _exit_tree():
    _free_track()
    _free_virtual_track()

func _ready():
    set_process(true)
    _ensure_curve()
    _ensure_track()
    _ensure_virtual_track()
    _apply_track_settings()
    _sync_track_state()
    set_notify_transform(true)
    set_notify_local_transform(true)
    _update_track()

func _notification(what):
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED, NOTIFICATION_ENTER_WORLD, NOTIFICATION_EXIT_WORLD:
            _request_track_update()
        NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
            _request_track_update()

func _ensure_curve():
    if not curve:
        curve = Curve3D.new()
    if not curve.changed.is_connected(_on_curve_changed):
        curve.changed.connect(_on_curve_changed)

func _ensure_track():
    _track_id = TrackRenderingServer.create_track()
    _apply_track_settings()

func _ensure_virtual_track():
    if _virtual_track_rid != RID():
        return
    _virtual_track_rid = TrackManager.add_path_track(PackedVector3Array(), track_id)

func _free_track():
    TrackRenderingServer.free_track(_track_id)

func _free_virtual_track():
    TrackManager.remove_track(_virtual_track_rid)
    _virtual_track_rid = RID()


func _apply_track_settings():
    TrackRenderingServer.set_sleeper_model_name(_track_id, sleeper_model_name)
    TrackRenderingServer.set_sleeper_spacing(_track_id, sleeper_spacing)
    TrackRenderingServer.set_rail_spacing(_track_id, rail_spacing)
    TrackRenderingServer.set_rail_height(_track_id, rail_height)
    TrackRenderingServer.set_ballast_height(_track_id, ballast_height)
    TrackRenderingServer.set_ballast_offset(_track_id, ballast_offset)
    TrackRenderingServer.set_ballast_uv_scale(_track_id, ballast_uv_scale)
    TrackRenderingServer.set_ballast_width_tiling(_track_id, ballast_width_tiling)
    TrackRenderingServer.set_ballast_length_tiling(_track_id, ballast_length_tiling)
    TrackRenderingServer.set_ballast_enabled(_track_id, ballast_enabled)
    TrackRenderingServer.set_ballast_texture(_track_id, ballast_texture)
    TrackRenderingServer.set_sleeper_model_name(_track_id, sleeper_model_name)
    TrackRenderingServer.set_sleeper_model_skin(_track_id, sleeper_model_skin)

func _sync_track_state():
    TrackRenderingServer.set_track_transform(_track_id, global_transform)
    TrackRenderingServer.set_track_visible(_track_id, is_visible_in_tree())
    TrackRenderingServer.set_track_scenario(_track_id, get_world_3d().scenario)

func _on_curve_changed():
    if _is_updating: return
    _request_track_update()

func _request_track_update():
    _dirty = true
    _update_timer = 0.0
    if Engine.is_editor_hint() and is_inside_tree():
        call_deferred("_update_track")

func _process(delta):
    if _dirty:
        _update_timer += delta
        if _update_timer >= DEBOUNCE_TIME or not Engine.is_editor_hint():
            _dirty = false
            _update_track()

func _update_track():
    _is_updating = true

    if auto_smooth:
        _apply_3_point_logic_optimized()

    if curve:
        curve.bake_interval = 0.1

    var path_data = _prepare_path_data()

    TrackRenderingServer.set_ballast_height(_track_id, ballast_height)
    TrackRenderingServer.set_sleeper_height(_track_id, sleeper_height)
    TrackRenderingServer.set_ballast_offset(_track_id, ballast_offset)
    TrackRenderingServer.set_ballast_length_tiling(_track_id, ballast_length_tiling)
    TrackRenderingServer.set_ballast_width_tiling(_track_id, ballast_width_tiling)
    TrackRenderingServer.set_ballast_uv_scale(_track_id, ballast_uv_scale)
    TrackRenderingServer.set_ballast_texture(_track_id, ballast_texture)
    TrackRenderingServer.set_ballast_enabled(_track_id, ballast_enabled)
    TrackRenderingServer.set_rail_height(_track_id, rail_height)
    TrackRenderingServer.set_rail_spacing(_track_id, rail_spacing)
    TrackRenderingServer.set_sleeper_spacing(_track_id, sleeper_spacing)
    TrackRenderingServer.set_sleeper_model_skin(_track_id, sleeper_model_skin)
    TrackRenderingServer.set_sleeper_model_name(_track_id, sleeper_model_name)
    TrackRenderingServer.update_track_data(_track_id, path_data["points"], path_data["quats"], path_data["length"])    
    TrackRenderingServer.update_rail_mesh(_track_id, path_data["length"], rail_spacing, rail_height, curve_precision)    
    TrackRenderingServer.update_ballast_mesh(_track_id, path_data["length"], curve_precision)    
    _sync_virtual_track()
    _notify_train_set_previews()

    _is_updating = false


func _apply_3_point_logic_optimized():
    if not curve or curve.point_count < 2: return
    curve.set_block_signals(true)

    for i in range(curve.point_count):
        var p = curve.get_point_position(i)
        if i == 0:
            var next_p = curve.get_point_position(1)
            curve.set_point_in(i, Vector3.ZERO)
            curve.set_point_out(i, (next_p - p).normalized() * (p.distance_to(next_p) * 0.35))
            curve.set_point_tilt(i, 0)
        elif i == curve.point_count - 1:
            var prev_p = curve.get_point_position(i - 1)
            curve.set_point_in(i, -(p - prev_p).normalized() * (p.distance_to(prev_p) * 0.35))
            curve.set_point_out(i, Vector3.ZERO)
            curve.set_point_tilt(i, 0)
        else:
            var p_prev = curve.get_point_position(i - 1)
            var p_next = curve.get_point_position(i + 1)
            var dir_prev = (p - p_prev).normalized()
            var dir_next = (p_next - p).normalized()
            
            if abs(p.x - p_prev.x) + abs(p_next.x - p.x) < 0.001 or dir_prev.dot(dir_next) > 0.9999:
                curve.set_point_in(i, Vector3.ZERO)
                curve.set_point_out(i, Vector3.ZERO)
                curve.set_point_tilt(i, 0)
                continue

            var tangent = (p_next - p_prev).normalized()
            var radius = _calculate_3_point_radius(p_prev, p, p_next)
            var soft = clamp(radius / min_turn_radius, 0.1, 1.0) if radius < min_turn_radius else 1.0
            curve.set_point_in(i, -tangent * (p.distance_to(p_prev) * 0.35 * soft))
            curve.set_point_out(i, tangent * (p.distance_to(p_next) * 0.35 * soft))

            if banking_intensity > 0:
                var angle = atan2(dir_next.x, dir_next.z) - atan2(dir_prev.x, dir_prev.z)
                if angle > PI: angle -= 2*PI
                if angle < -PI: angle += 2*PI
                curve.set_point_tilt(i, angle * banking_intensity)
    
    curve.set_block_signals(false)
    curve.emit_changed()

func _calculate_3_point_radius(p1: Vector3, p2: Vector3, p3: Vector3) -> float:
    var a = p1.distance_to(p2); var b = p2.distance_to(p3); var c = p3.distance_to(p1)
    if a == 0 or b == 0 or c == 0: return INF
    var s = (a + b + c) / 2.0
    var area_sq = s * (s - a) * (s - b) * (s - c)
    return (a * b * c) / (4.0 * sqrt(max(0, area_sq))) if area_sq > 0 else INF

func _prepare_path_data() -> Dictionary:
    var length = curve.get_baked_length()
    var max_points = 256
    var interval = max(0.1, length / float(max_points - 1))
    
    var points = PackedVector4Array()
    var quats = PackedVector4Array()
    
    var current_dist = 0.0
    while current_dist <= length + 0.001 and points.size() < max_points:
        var t = curve.sample_baked_with_rotation(current_dist)
        var tilt = 0.0
        if curve.has_method("sample_baked_tilt"):
            tilt = curve.sample_baked_tilt(current_dist)
        
        points.append(Vector4(t.origin.x, t.origin.y, t.origin.z, tilt))
        
        var q = t.basis.get_rotation_quaternion()
        quats.append(Vector4(q.x, q.y, q.z, q.w))
        
        current_dist += interval
        if interval <= 0: break
    
    return {
        "points": points,
        "quats": quats,
        "count": points.size(),
        "length": length
    }

func _sync_virtual_track():

    var baked_points := curve.get_baked_points() if curve != null else PackedVector3Array()
    var world_points := PackedVector3Array()

    if baked_points.is_empty():
        world_points.push_back(global_position)
    else:
        for point in baked_points:
            world_points.push_back(to_global(point))

    TrackManager.modify_path_track(
        _virtual_track_rid,
        world_points,
        track_id,
        _resolve_connected_track_rid(previous_track),
        _resolve_connected_track_rid(next_track)
    )
    
    TrackManager.set_track_heights(
        _virtual_track_rid,
        ballast_height + sleeper_height,
        rail_height
    )

func _resolve_connected_track_rid(target_path: NodePath) -> RID:
    if target_path.is_empty():
        return RID()

    var target = get_node_or_null(target_path)
    if target == null:
        return RID()

    if target is MaszynaTrack3D:
        return (target as MaszynaTrack3D).get_virtual_track_rid()

    if target is MaszynaSwitch3D:
        var resolved = (target as MaszynaSwitch3D).resolve_track(self)
        if resolved != null:
            return resolved.get_virtual_track_rid()

    return RID()

func _notify_train_set_previews() -> void:
    if not is_inside_tree():
        return

    get_tree().call_group_flags(SceneTree.GROUP_CALL_DEFERRED, &"train_set_3d_editor_preview", "_queue_editor_layout")

func get_rail_top_vertical_offset() -> float:
    return ballast_height + sleeper_height + rail_height
    

func get_virtual_track_rid():
    return _virtual_track_rid
