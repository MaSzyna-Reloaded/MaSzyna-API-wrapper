@tool
extends Node3D
class_name MaszynaTrack3D

static var _unknown_material = preload("res://addons/libmaszyna/materials/unknown.material")
static var _rail_material = preload("res://addons/libmaszyna/materials/rail.material")

@export_group("Domain")
@export_enum("flat", "bridge", "tunnel", "mountains", "canyon", "bank") var environment:String = "flat"
@export_flags("Destroyed:128") var damage_flag:int = 0
@export var quality_flag:int
@export var friction:float = 0.0
@export var length:float = 0.0
@export var parameters:Dictionary = {}
@export var curve:MaszynaTrackCurve:
    set(x):
        curve = x
        _dirty = true
@export var curve2:MaszynaTrackCurve:
    set(x):
        curve2 = x
        _dirty = true

@export_group("Rails")
@export var width: float = 1.6:
    set(value):
        if is_equal_approx(width, value):
            return
        width = value
        _dirty = true

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

@export_group("Sleepers")
@export var sleeper_model_name: String = "":
    set(x):
        sleeper_model_name = x
        _dirty = true

@export var sleeper_model_skin: String = "":
    set(x):
        sleeper_model_skin = x
        _dirty = true

@export var sleeper_spacing: float = 1.0:
    set(x):
        sleeper_spacing = x
        _dirty = true

@export var sleeper_height: float = -0.05:
    set(x):
        sleeper_height = x
        _dirty = true

@export_group("Trackbed")
@export var ballast_enabled: bool = true:
    set(x):
        ballast_enabled = x
        _dirty = true

@export var material2: String = "":
    set(value):
        if material2 == value:
            return
        material2 = value
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

@export var ballast_uv_scale: float = 1.0:
    set(x):
        ballast_uv_scale = x
        _dirty = true

@export var ballast_width_tiling: float = 1.0:
    set(x):
        ballast_width_tiling = x
        _dirty = true

@export var ballast_length_tiling: float = 1.0:
    set(x):
        ballast_length_tiling = x
        _dirty = true

@export var ballast_height: float = 0.4:
    set(x):
        ballast_height = x
        _dirty = true

@export var ballast_offset: float = -0.05:
    set(x):
        ballast_offset = x
        _dirty = true

@export_group("Connectivity")
@export var next_track: NodePath
@export var previous_track: NodePath
## Minimum distance from camera required to render the track.
## When set to [code]0.0[/code], there is no near distance limit.
@export var visibility_range_begin: float = 0.0:
    set(x):
        if not is_equal_approx(x, visibility_range_begin):
            visibility_range_begin = x
            _mark_dirty()

## Maximum distance from camera allowed to render the track.
## When set to [code]0.0[/code], there is no far distance limit.
@export var visibility_range_end: float = 0.0:
    set(x):
        if not is_equal_approx(x, visibility_range_end):
            visibility_range_end = x
            _mark_dirty()

var _track_rid: RID
var _dirty = false
var _is_updating = false
var _update_timer = 0.0
var _curve:Curve3D
var _curve2:Curve3D
var _trackbed_material: StandardMaterial3D = _unknown_material

const DEBOUNCE_TIME = 0.1
var _last_debouce:float
var _range_check_interval: float = 1.0
var _range_check_elapsed: float = 0.0

func _enter_tree():
    _initialize_curve_3d()
    _apply_visibility_state(false)
    
func _exit_tree():
    _free_track()
    _free_curve_3d()

func _initialize_curve_3d():
    _curve = _create_runtime_curve()
    _curve2 = _create_runtime_curve()
    if curve:
        curve.changed.connect(_update_curve3d)
    if curve2:
        curve2.changed.connect(_update_curve3d)
    _update_curve3d()


func _create_runtime_curve() -> Curve3D:
    var runtime_curve := Curve3D.new()
    runtime_curve.closed = false
    runtime_curve.bake_interval = _get_curve_bake_interval(curve)
    runtime_curve.changed.connect(_mark_dirty)
    return runtime_curve
    
func _free_curve_3d():
    if _curve:
        _curve.changed.disconnect(_mark_dirty)
        _curve = null
    if _curve2:
        _curve2.changed.disconnect(_mark_dirty)
        _curve2 = null
    if curve:
        curve.changed.disconnect(_update_curve3d)
    if curve2:
        curve2.changed.disconnect(_update_curve3d)
    
func _mark_dirty():
    if _is_updating:
        return
    _dirty = true
    _range_check_elapsed = _range_check_interval
    
func _ready():
    set_process(true)
    set_notify_transform(true)
    _apply_visibility_state(false)

func _notification(what):
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED:
            if _track_rid and _track_rid.is_valid() and TrackRenderingServer:
                TrackRenderingServer.set_track_transform(_track_rid, global_transform)
                TrackRenderingServer.set_track_visible(_track_rid, visible)
        

func _process(delta: float) -> void:
    if _dirty:
        _process_dirty(delta)
        return

    _range_check_elapsed += delta
    if _range_check_elapsed >= _range_check_interval:
        _apply_visibility_state()


func _process_dirty(_delta: float) -> void:
    _dirty = false
    var _current := (Time.get_ticks_msec()/1000.0)
    if _current - _last_debouce > DEBOUNCE_TIME:
        _update_track_rendering()
        _apply_visibility_state()
        _last_debouce = _current


func _update_track_curve():
    _update_track_rendering()


func _update_track_rendering():
    var server := TrackRenderingServer
    if not server or not _track_rid or not _track_rid.is_valid():
        return

    var primary_rail_material: StandardMaterial3D = _rail_material
    if not material1.is_empty():
        primary_rail_material = MaterialManager.get_material("", material1)

    var secondary_rail_material: StandardMaterial3D = primary_rail_material
    if _is_switch() and not material2.is_empty():
        secondary_rail_material = MaterialManager.get_material("", material2)

    _trackbed_material = _unknown_material
    var trackbed_enabled: bool = not material2.is_empty()
    var switch_trackbed: Dictionary = {}
    if _is_switch():
        switch_trackbed = _get_switch_trackbed_profile()
        trackbed_enabled = not switch_trackbed.is_empty()
    elif not material2.is_empty():
        _trackbed_material = MaterialManager.get_material("", material2)

    var next_trackbed: Dictionary = _get_next_trackbed_profile()
    var start_tex_height: float = tex_height + _get_roll_fix_height(curve.roll1 if curve else 0.0)

    server.set_track_curve(
        _track_rid,
        _curve,
        _curve2,
        width,
        tex_length,
        start_tex_height,
        tex_width,
        tex_slope,
        railprofile,
        curve.roll1 if curve else 0.0,
        curve.roll2 if curve else 0.0,
        curve2.roll1 if curve2 else 0.0,
        curve2.roll2 if curve2 else 0.0,
        next_trackbed,
        switch_trackbed,
        trackbed_enabled
    )
    server.set_track_scenario(_track_rid, get_world_3d().scenario)
    server.set_track_transform(_track_rid, global_transform)
    server.set_track_visible(_track_rid, visible)
    server.set_track_materials(
        _track_rid,
        _trackbed_material.get_rid(),
        primary_rail_material.get_rid(),
        secondary_rail_material.get_rid()
    )


func _update_curve3d():
    if not _curve or not _curve2:
        return

    _is_updating = true
    _populate_runtime_curve(_curve, curve)
    _populate_runtime_curve(_curve2, curve2)
    _is_updating = false
    _mark_dirty()


func _populate_runtime_curve(runtime_curve: Curve3D, source_curve: MaszynaTrackCurve) -> void:
    runtime_curve.clear_points()
    if not source_curve:
        runtime_curve.emit_changed()
        return

    var start_roll_fix: float = _get_roll_fix_height(source_curve.roll1)
    var end_roll_fix: float = _get_roll_fix_height(source_curve.roll2)

    runtime_curve.bake_interval = _get_curve_bake_interval(source_curve)
    runtime_curve.add_point(source_curve.p1 + Vector3(0.0, start_roll_fix, 0.0), Vector3.ZERO, source_curve.c1)
    runtime_curve.add_point(source_curve.p2 + Vector3(0.0, end_roll_fix, 0.0), source_curve.c2, Vector3.ZERO)
    runtime_curve.set_point_tilt(0, source_curve.roll1)
    runtime_curve.set_point_tilt(1, source_curve.roll2)
    runtime_curve.emit_changed()


func _get_curve_bake_interval(source_curve: MaszynaTrackCurve) -> float:
    if not source_curve:
        return 10.0

    if not is_zero_approx(source_curve.radius):
        return clamp(abs(source_curve.radius) * 0.02, 2.0, 10.0)

    if source_curve.c1 == Vector3.ZERO and source_curve.c2 == Vector3.ZERO:
        return 10.0

    return clamp(source_curve.p1.distance_to(source_curve.p2) * 0.1, 2.0, 10.0)


func _get_next_trackbed_profile() -> Dictionary:
    if _is_switch():
        return {}

    var neighbor: MaszynaTrack3D = _resolve_track_node(next_track)
    if not neighbor or neighbor._is_switch():
        return {}

    var neighbor_roll1: float = neighbor.curve.roll1 if neighbor.curve else 0.0
    return {
        "width": neighbor.width,
        "tex_height": neighbor.tex_height + _get_roll_fix_height(neighbor_roll1),
        "tex_width": neighbor.tex_width,
        "tex_slope": neighbor.tex_slope,
    }


func _get_switch_trackbed_profile() -> Dictionary:
    var neighbor: MaszynaTrack3D = _resolve_track_node(previous_track)
    if not neighbor or neighbor._is_switch():
        neighbor = _resolve_track_node(next_track)
    if not neighbor or neighbor._is_switch() or neighbor.material2.is_empty():
        return {}

    var neighbor_roll1: float = neighbor.curve.roll1 if neighbor.curve else 0.0
    _trackbed_material = MaterialManager.get_material("", neighbor.material2)
    return {
        "width": neighbor.width,
        "tex_height": neighbor.tex_height + _get_roll_fix_height(neighbor_roll1),
        "tex_width": neighbor.tex_width,
        "tex_slope": neighbor.tex_slope,
    }


func _resolve_track_node(track_path: NodePath) -> MaszynaTrack3D:
    if track_path.is_empty():
        return null

    var node: Node = get_node_or_null(track_path)
    if node is MaszynaTrack3D:
        return node as MaszynaTrack3D
    return null


func _get_roll_fix_height(roll_degrees: float) -> float:
    return abs(sin(deg_to_rad(roll_degrees)) * 0.75)


func _apply_visibility_state(force_reload: bool = false) -> void:
    _range_check_elapsed = 0.0

    if _should_render():
        if force_reload or not (_track_rid and _track_rid.is_valid()):
            _create_track()
            _update_track_rendering()
    else:
        _free_track()


func _should_render() -> bool:
    if not is_inside_tree():
        return false

    if is_zero_approx(visibility_range_begin) and is_zero_approx(visibility_range_end):
        return true

    var camera: Camera3D = _get_active_camera()
    if not is_instance_valid(camera):
        return false

    var distance: float = camera.global_position.distance_to(_get_visibility_reference_position())
    if visibility_range_begin > 0.0 and distance < visibility_range_begin:
        return false
    if visibility_range_end > 0.0 and distance > visibility_range_end:
        return false
    return true


func _get_active_camera() -> Camera3D:
    if Engine.is_editor_hint():
        return E3DEditorCameraTracker.get_camera()

    var viewport: Viewport = get_viewport()
    if viewport:
        return viewport.get_camera_3d()
    return null


func _get_visibility_reference_position() -> Vector3:
    var centers: Array[Vector3] = []
    if curve:
        centers.append(curve.p1 + (curve.p2 - curve.p1) * 0.5)
    if curve2:
        centers.append(curve2.p1 + (curve2.p2 - curve2.p1) * 0.5)
    if not centers.is_empty():
        var local_center := Vector3.ZERO
        for point: Vector3 in centers:
            local_center += point
        local_center /= float(centers.size())
        return global_transform * local_center
    return global_position


func _is_switch() -> bool:
    return curve2 != null


func _create_track() -> void:
    if not TrackRenderingServer:
        return

    _free_track()
    _track_rid = TrackRenderingServer.create_track()
    TrackRenderingServer.set_track_scenario(_track_rid, get_world_3d().scenario)


func _free_track() -> void:
    if TrackRenderingServer and _track_rid and _track_rid.is_valid():
        TrackRenderingServer.free_track(_track_rid)
    _track_rid = RID()
