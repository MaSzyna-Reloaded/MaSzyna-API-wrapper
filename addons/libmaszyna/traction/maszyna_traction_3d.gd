@tool
extends Node3D
class_name MaszynaTraction3D

const TRACTION_AL_MATERIAL = preload("res://addons/libmaszyna/materials/traction_al.material")
const TRACTION_CU_MATERIAL = preload("res://addons/libmaszyna/materials/traction_cu.material")
const TRACTION_AL_PATINA_MATERIAL = preload("res://addons/libmaszyna/materials/traction_al_patina.material")
const TRACTION_CU_PATINA_MATERIAL = preload("res://addons/libmaszyna/materials/traction_cu_patina.material")

const RANGE_CHECK_INTERVAL := 1.0

enum DamageFlag {
    PATINA=1,
    BREAK=128
}

enum TractionMaterial {COPPER, ALUMINIUM}

func str_or_none(x):
    return str(x) if x else "none"

@export var wires:TractionRenderingServer.Wires:
    set(x):
        if not x == wires:
            _dirty = true
            wires = x

@export var contact_line: MaszynaTractionLine:
    set(x):
        if not contact_line == x:
            _dirty = true
            if contact_line:
                contact_line.changed.disconnect(_on_line_changed)
            contact_line = x
        if x:
            contact_line.changed.connect(_on_line_changed)

@export var support_line: MaszynaTractionLine:
    set(x):
        if not support_line == x:
            _dirty = true
            if support_line:
                support_line.changed.disconnect(_on_line_changed)
            support_line = x
        if x:
            support_line.changed.connect(_on_line_changed)

@export var material:TractionMaterial:
    set(x):
        if not material == x:
            material = x
            _dirty = true
            
@export_flags("Patina:1", "Break:128") var damage_flag:int = 0:
    set(x):
        if not damage_flag == x:
            damage_flag = x
            _dirty = true
    
@export var power_supply_name:String
@export var nominal_voltage:float
@export var max_current:float
@export var resistivity:float
@export var wire_thickness:float  # mm
@export var min_height:float
@export var segment_length:float

@export var wire_offset:float
@export var parallel:String = ""
## Minimum distance from camera required to render the traction.
## When set to [code]0.0[/code], there is no near distance limit.
@export var visibility_range_begin: float = 0.0:
    set(x):
        if not is_equal_approx(x, visibility_range_begin):
            visibility_range_begin = x
            _dirty = true
            _range_check_elapsed = RANGE_CHECK_INTERVAL

## Maximum distance from camera allowed to render the traction.
## When set to [code]0.0[/code], there is no far distance limit.
@export var visibility_range_end: float = 0.0:
    set(x):
        if not is_equal_approx(x, visibility_range_end):
            visibility_range_end = x
            _dirty = true
            _range_check_elapsed = RANGE_CHECK_INTERVAL

var _traction_rid: RID
var _dirty: bool = false
var _range_check_elapsed: float = 0.0
var _lines_center:Vector3


func _ready() -> void:
    set_process(true)
    set_notify_transform(true)
    _apply_visibility_state()
    _update()


func _enter_tree():
    visibility_changed.connect(_update_visibility)
    _apply_visibility_state()
        

func _exit_tree():
    if contact_line:
        contact_line.changed.disconnect(_on_line_changed)
    if support_line:
        support_line.changed.disconnect(_on_line_changed)
    visibility_changed.disconnect(_update_visibility)

    if TractionRenderingServer and _traction_rid:
        TractionRenderingServer.free_traction(_traction_rid)
        _traction_rid = RID()


func _notification(what):
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED:
            if _traction_rid and TractionRenderingServer:
                TractionRenderingServer.set_traction_transform(_traction_rid, global_transform)
                TractionRenderingServer.set_traction_visible(_traction_rid, visible)


func _process(delta: float) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(delta)
            _dirty = false

    _range_check_elapsed += delta
    if _range_check_elapsed >= RANGE_CHECK_INTERVAL:
        _apply_visibility_state()


func _process_dirty(_delta: float) -> void:
    _update()


func _update():
    if not TractionRenderingServer or not _traction_rid or not _traction_rid.is_valid():
        return

    var contact_p1: Vector3 = contact_line.p1 if contact_line else Vector3.ZERO
    var contact_p2: Vector3 = contact_line.p2 if contact_line else Vector3.ZERO
    var support_p1: Vector3 = support_line.p1 if support_line else Vector3.ZERO
    var support_p2: Vector3 = support_line.p2 if support_line else Vector3.ZERO

    _lines_center = contact_line.p1+(contact_line.p2-contact_line.p1)*0.5

    TractionRenderingServer.set_traction_geometry(
        _traction_rid,
        contact_p1,
        contact_p2,
        support_p1,
        support_p2,
        wire_thickness,
        wires,
        wire_offset,
        min_height,
        segment_length
    )
    TractionRenderingServer.set_traction_transform(_traction_rid, global_transform)
    TractionRenderingServer.set_traction_visible(_traction_rid, visible)
    TractionRenderingServer.set_traction_scenario(_traction_rid, get_world_3d().scenario)
    TractionRenderingServer.set_traction_material(_traction_rid, _get_material().get_rid())


func _on_line_changed() -> void:
    _dirty = true


func _update_visibility():
    if _traction_rid and TractionRenderingServer:
        TractionRenderingServer.set_traction_visible(_traction_rid, visible)


func _get_material():
    if damage_flag & DamageFlag.PATINA:
        return (
            TRACTION_CU_PATINA_MATERIAL if material == TractionMaterial.COPPER
            else TRACTION_AL_PATINA_MATERIAL
        )
    else:
        return (
            TRACTION_CU_MATERIAL if material == TractionMaterial.COPPER
            else TRACTION_AL_MATERIAL
        )


func _apply_visibility_state() -> void:
    _range_check_elapsed = 0.0
    if not TractionRenderingServer:
        return

    var should_render = false

    if not visibility_range_begin and not visibility_range_end:
        should_render = true
    else:
        var camera: Camera3D = _get_active_camera()
        if camera:
            var distance := camera.global_position.distance_to(global_transform * _lines_center)
            if (
                (not visibility_range_end and distance >= visibility_range_begin)
                or (not visibility_range_begin and distance <= visibility_range_end)
            ):
                should_render = true

    if not _traction_rid and should_render:
        _traction_rid = TractionRenderingServer.create_traction()
        _update()
    elif _traction_rid and not should_render:
        TractionRenderingServer.free_traction(_traction_rid)
        _traction_rid = RID()


func _get_active_camera() -> Camera3D:
    if Engine.is_editor_hint():
        return E3DEditorCameraTracker.get_camera()

    var viewport: Viewport = get_viewport()
    if viewport:
        return viewport.get_camera_3d()
    return null
