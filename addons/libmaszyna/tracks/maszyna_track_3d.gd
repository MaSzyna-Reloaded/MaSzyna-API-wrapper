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

@export_group("Rails")
@export var rail_spacing: float = 1.6:
    set(x):
        rail_spacing = x
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

@export_group("Ballast")
@export var ballast_enabled: bool = true:
    set(x):
        ballast_enabled = x
        _dirty = true

@export var ballast_material: String = "":
    set(x):
        ballast_material = x
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
var _sleeper_mesh: Mesh
var _sleeper_material: StandardMaterial3D = _unknown_material
var _ballast_material: StandardMaterial3D = _unknown_material

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
    _curve = Curve3D.new()
    _curve.closed = false
    _curve.bake_interval = 10
    _curve.changed.connect(_mark_dirty)
    if curve:
        curve.changed.connect(_update_curve3d)
        _update_curve3d()
    
func _free_curve_3d():
    if _curve:
        _curve.changed.disconnect(_mark_dirty)
        _curve = null
    if curve:
        curve.changed.disconnect(_update_curve3d)
    
func _mark_dirty():
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
        _free_curve_3d()
        _initialize_curve_3d()
        _apply_visibility_state(true)
        _last_debouce = _current


func _update_track_curve():
    _update_track_rendering()


func _update_track_rendering():
    var server := TrackRenderingServer
    if not server or not _track_rid or not _track_rid.is_valid():
        return

    _ballast_material = MaterialManager.get_material("", ballast_material)
    _sleeper_material = MaterialManager.get_material("", sleeper_model_skin)
    var sleeper_model = E3DModelManager.load_model("", sleeper_model_name)

    _sleeper_mesh = null
    if sleeper_model:
        _sleeper_mesh = E3DModelTool.find_first_mesh(sleeper_model)

    server.set_track_curve(
        _track_rid,
        _curve,
        rail_spacing,
        sleeper_spacing,
        sleeper_height,
        ballast_enabled,
        ballast_height,
        ballast_offset,
        ballast_width_tiling,
        ballast_length_tiling,
        ballast_uv_scale
    )
    server.set_track_scenario(_track_rid, get_world_3d().scenario)
    server.set_sleeper_mesh(_track_rid, _sleeper_mesh)
    server.set_track_transform(_track_rid, global_transform)
    server.set_track_visible(_track_rid, visible)
    server.set_track_materials(_track_rid, _ballast_material.get_rid(), _rail_material.get_rid(), _sleeper_material.get_rid())


func _update_curve3d():
    if not _curve:
        return
    
    _curve.clear_points()
    _curve.add_point(curve.p1, Vector3.ZERO, curve.c1)
    _curve.add_point(curve.p2, curve.c2, Vector3.ZERO)
    _curve.emit_changed()


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
    if curve:
        var local_center: Vector3 = curve.p1 + (curve.p2 - curve.p1) * 0.5
        return global_transform * local_center
    return global_position


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
