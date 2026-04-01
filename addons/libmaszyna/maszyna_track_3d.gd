@tool
extends Path3D
class_name MaszynaTrack3D

@export var sleeper_mesh: Mesh:
    set(x):
        sleeper_mesh = x
        if _renderer: _renderer.sleeper_mesh = x
        _dirty = true

@export var sleeper_spacing: float = 0.6:
    set(x):
        sleeper_spacing = x
        if _renderer: _renderer.sleeper_spacing = x
        _dirty = true

@export var rail_material: ShaderMaterial:
    set(x):
        rail_material = x
        if _renderer: _renderer.rail_material = x
        _dirty = true

@export var sleeper_material: ShaderMaterial:
    set(x):
        sleeper_material = x
        if _renderer: _renderer.sleeper_material = x
        _dirty = true

@export var rail_spacing: float = 1.6:
    set(x):
        rail_spacing = x
        if _renderer: _renderer.rail_spacing = x
        _dirty = true

@export var collision_enabled: bool = true:
    set(x):
        collision_enabled = x
        if _renderer: _renderer.collision_enabled = x
        _dirty = true

@export var curve_precision: float = 0.5:
    set(x):
        curve_precision = x
        _dirty = true

@export var auto_smooth: bool = true:
    set(x):
        auto_smooth = x
        _dirty = true

@export var min_turn_radius: float = 50.0:
    set(x):
        min_turn_radius = x
        _dirty = true

@export var banking_intensity: float = 0.0:
    set(x):
        banking_intensity = x
        _dirty = true

@export var sleeper_height: float = 0.0:
    set(x):
        sleeper_height = x
        if _renderer: _renderer.sleeper_height = x
        _dirty = true

@export_group("Connectivity")
@export var next_track: NodePath
@export var previous_track: NodePath

var _renderer: TrackRenderer
var _rail_mesh: ArrayMesh
var _dirty = false
var _is_updating = false
var _update_timer = 0.0
const DEBOUNCE_TIME = 0.05 

func _ready():
    if not curve:
        curve = Curve3D.new()
    if not curve.changed.is_connected(_on_curve_changed):
        curve.changed.connect(_on_curve_changed)
    
    if not _renderer:
        _renderer = TrackRenderer.new()
        _renderer.name = "TrackRenderer"
        add_child(_renderer, false, Node.INTERNAL_MODE_FRONT)
    
    if not rail_material:
        rail_material = ShaderMaterial.new()
        rail_material.shader = load("res://addons/libmaszyna/materials/track_path_deform.gdshader")
    
    if not sleeper_material:
        sleeper_material = ShaderMaterial.new()
        sleeper_material.shader = load("res://addons/libmaszyna/materials/sleeper_path_deform.gdshader")
    
    _renderer.rail_material = rail_material
    _renderer.sleeper_material = sleeper_material
    _renderer.sleeper_mesh = sleeper_mesh
    _renderer.sleeper_height = sleeper_height
    _renderer.sleeper_spacing = sleeper_spacing
    _renderer.rail_spacing = rail_spacing
    _renderer.collision_enabled = collision_enabled
    
    if Engine.is_editor_hint():
        _update_track()
    else:
        # Wait a frame to let other nodes (like RailVehicle3D) initialize their track refs
        await get_tree().process_frame
        _update_track()

func _on_curve_changed():
    if _is_updating: return
    _dirty = true
    _update_timer = 0.0

func _process(delta):
    if _dirty:
        _update_timer += delta
        if _update_timer >= DEBOUNCE_TIME or not Engine.is_editor_hint():
            _dirty = false
            _update_track()

func _update_track():
    if not is_inside_tree() or _is_updating or not _renderer: return
    _is_updating = true
    
    if auto_smooth:
        _apply_3_point_logic_optimized()
    
    if curve:
        curve.bake_interval = 0.1
    
    var path_data = _prepare_path_data()
    
    # Update C++ renderer
    _renderer.update_track_data(path_data["points"], path_data["quats"], path_data["length"])
    
    # 3. UPDATE AABB FOR VISIBILITY
    var points = curve.get_baked_points()
    if points.size() > 0:
        var aabb = AABB(points[0], Vector3.ZERO)
        for p in points:
            aabb = aabb.expand(p)
        aabb = aabb.grow(5.0)
        RenderingServer.instance_set_custom_aabb(_renderer.get_rail_mesh_instance(), aabb)
        RenderingServer.instance_set_custom_aabb(_renderer.get_sleeper_multimesh_instance(), aabb)
    
    # Rail template mesh still needs to be generated (for now)
    _update_rail_mesh(path_data["length"])
    
    _is_updating = false

func _update_rail_mesh(length: float):
    if length < 0.1: return
    _generate_template_mesh(length)

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

func _generate_template_mesh(length: float):
    if _renderer:
        _renderer.update_rail_mesh(length, rail_spacing, curve_precision)

# Helper to get the RID from the renderer (we should add this to C++)
func get_rail_instance_rid() -> RID:
    return _renderer.get_rail_mesh_instance() if _renderer else RID()

func get_rail_top_vertical_offset() -> float:
    return sleeper_height + 0.16
