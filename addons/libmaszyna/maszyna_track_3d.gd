@tool
extends Path3D
class_name MaszynaTrack3D

@export_group("Rails")
@export var rail_spacing: float = 1.6:
    set(x):
        rail_spacing = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_rail_spacing(_track_id, x)
        _request_track_update()

@export_group("Sleepers")
@export var sleeper_model_name: String = "":
    set(x):
        sleeper_model_name = x
        var server = _get_track_server()
        if is_inside_tree() and server and _track_id != 0:
            server.set_sleeper_model_name(_track_id, x)
        _request_track_update()

@export var sleeper_model_skin: String = "":
    set(x):
        sleeper_model_skin = x
        var server = _get_track_server()
        if is_inside_tree() and server and _track_id != 0:
            server.set_sleeper_model_skin(_track_id, x)
        _request_track_update()

@export var sleeper_spacing: float = 1.0:
    set(x):
        sleeper_spacing = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_sleeper_spacing(_track_id, x)
        _request_track_update()

@export var sleeper_height: float = -0.05:
    set(x):
        sleeper_height = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_sleeper_height(_track_id, x)
        _request_track_update()

@export_group("Ballast")
@export var ballast_enabled: bool = true:
    set(x):
        ballast_enabled = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_enabled(_track_id, x)
        _request_track_update()

@export var ballast_texture: String = "":
    set(x):
        ballast_texture = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_texture(_track_id, x)
        _request_track_update()

@export var ballast_uv_scale: float = 1.0:
    set(x):
        ballast_uv_scale = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_uv_scale(_track_id, x)
        _request_track_update()

@export var ballast_width_tiling: float = 1.0:
    set(x):
        ballast_width_tiling = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_width_tiling(_track_id, x)
        _request_track_update()

@export var ballast_length_tiling: float = 1.0:
    set(x):
        ballast_length_tiling = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_length_tiling(_track_id, x)
        _request_track_update()

@export var ballast_height: float = 0.4:
    set(x):
        ballast_height = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_height(_track_id, x)
        _request_track_update()

@export var ballast_offset: float = -0.05:
    set(x):
        ballast_offset = x
        var server = _get_track_server()
        if server and _track_id != 0:
            server.set_ballast_offset(_track_id, x)
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
@export var next_track: NodePath
@export var previous_track: NodePath

var _track_id: int = 0
var _dirty = false
var _is_updating = false
var _update_timer = 0.0
const DEBOUNCE_TIME = 0.05

func _enter_tree():
    _ensure_curve()
    _ensure_track()
    _sync_track_state()

func _exit_tree():
    _free_track()

func _ready():
    _ensure_curve()
    _ensure_track()
    set_process(true)
    set_notify_transform(true)
    _apply_track_settings()
    _sync_track_state()
    if Engine.is_editor_hint():
        _update_track()
    else:
        await get_tree().process_frame
        _update_track()

func _notification(what):
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED, NOTIFICATION_ENTER_WORLD, NOTIFICATION_EXIT_WORLD:
            _sync_track_state()

func _ensure_curve():
    if not curve:
        curve = Curve3D.new()
    if not curve.changed.is_connected(_on_curve_changed):
        curve.changed.connect(_on_curve_changed)

func _get_track_server():
    return TrackRenderingServer if Engine.has_singleton("TrackRenderingServer") else null

func _ensure_track():
    if _track_id != 0:
        return
    var server = _get_track_server()
    if not server:
        return
    _track_id = server.create_track()
    _apply_track_settings()

func _free_track():
    if _track_id == 0:
        return
    var server = _get_track_server()
    if server:
        server.free_track(_track_id)
    _track_id = 0

func _apply_track_settings():
    var server = _get_track_server()
    if not server or _track_id == 0:
        return

    server.set_sleeper_height(_track_id, sleeper_height)
    server.set_sleeper_spacing(_track_id, sleeper_spacing)
    server.set_rail_spacing(_track_id, rail_spacing)
    server.set_ballast_height(_track_id, ballast_height)
    server.set_ballast_offset(_track_id, ballast_offset)
    server.set_ballast_uv_scale(_track_id, ballast_uv_scale)
    server.set_ballast_width_tiling(_track_id, ballast_width_tiling)
    server.set_ballast_length_tiling(_track_id, ballast_length_tiling)
    server.set_ballast_enabled(_track_id, ballast_enabled)
    server.set_ballast_texture(_track_id, ballast_texture)
    server.set_sleeper_model_name(_track_id, sleeper_model_name)
    server.set_sleeper_model_skin(_track_id, sleeper_model_skin)

func _sync_track_state():
    var server = _get_track_server()
    if not server or _track_id == 0:
        return

    server.set_track_transform(_track_id, global_transform)
    server.set_track_visible(_track_id, is_visible_in_tree())
    if is_inside_tree() and get_world_3d():
        server.set_track_scenario(_track_id, get_world_3d().scenario)
    else:
        server.set_track_scenario(_track_id, RID())

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
    var server = _get_track_server()
    if not is_inside_tree() or _is_updating or not server or _track_id == 0: return
    _is_updating = true

    if auto_smooth:
        _apply_3_point_logic_optimized()

    if curve:
        curve.bake_interval = 0.1

    var path_data = _prepare_path_data()

    server.update_track_data(_track_id, path_data["points"], path_data["quats"], path_data["length"])
    _update_rail_mesh(path_data["length"])
    _update_ballast_mesh(path_data["length"])

    _is_updating = false

func _update_rail_mesh(length: float):
    var server = _get_track_server()
    if server and _track_id != 0:
        server.update_rail_mesh(_track_id, length, rail_spacing, curve_precision)

func _update_ballast_mesh(length: float):
    var server = _get_track_server()
    if server and _track_id != 0 and ballast_enabled:
        server.update_ballast_mesh(_track_id, length, curve_precision)

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

func get_rail_instance_rid() -> RID:
    var server = _get_track_server()
    return server.get_rail_mesh_instance(_track_id) if server and _track_id != 0 else RID()

func get_rail_top_vertical_offset() -> float:
    return sleeper_height + 0.16
