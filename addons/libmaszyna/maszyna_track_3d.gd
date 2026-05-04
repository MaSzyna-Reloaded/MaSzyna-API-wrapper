@tool
extends Path3D
class_name MaszynaTrack3D

static var _unknown_material = preload("res://addons/libmaszyna/materials/unknown.material")
static var _rail_material = preload("res://addons/libmaszyna/materials/rail.material")

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

func _enter_tree():
    if TrackRenderingServer:
        _track_rid = TrackRenderingServer.create_track()
        TrackRenderingServer.set_track_scenario(_track_rid, get_world_3d().scenario)
        _update_track_rendering()
    curve_changed.connect(_update_track_rendering)

func _exit_tree():
    if TrackRenderingServer and _track_rid and _track_rid.is_valid():
        TrackRenderingServer.free_track(_track_rid)
        _track_rid = RID()
    curve_changed.disconnect(_update_track_rendering)

func _ready():
    set_process(true)
    set_notify_transform(true)


func _notification(what):
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED:
            if _track_rid and TrackRenderingServer:
                TrackRenderingServer.set_track_transform(_track_rid, global_transform)
                TrackRenderingServer.set_track_visible(_track_rid, visible)
        

func _process(_delta) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(_delta)
            _dirty = false


func _process_dirty(_delta: float) -> void:
    var _current := (Time.get_ticks_msec()/1000.0)
    if _current - _last_debouce > DEBOUNCE_TIME:
        _update_track_rendering()
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
        curve,
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
