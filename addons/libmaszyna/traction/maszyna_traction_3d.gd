@tool
extends Node3D
class_name MaszynaTraction3D

const TRACTION_MATERIAL = preload("res://addons/libmaszyna/materials/traction.material")

#static var eu07_exporter = load("res://addons/maszyna/exporters/node_traction.gd").new()
#static var eu07_importer = load("res://addons/maszyna/importers/node_traction.gd").new()

enum Wires {JEZDNY, JEDNY_NOSNY, DWA_JEZDNE_NOSNY, DWA_JEZDNE_DWA_NOSNE}

func str_or_none(x):
    return str(x) if x else "none"

@export var curve1:MaszynaTrackCurve
@export var curve2:MaszynaTrackCurve
@export var power_supply_name:String
@export var nominal_voltage:float
@export var max_current:float
@export var resistivity:float
@export_enum("cu", "al") var material:String
@export var wire_thickness:float  # mm
@export_flags("Patyna:1", "Zerwanie:128") var damage_flag:int
@export var min_height:float
@export var segment_length:float
@export var wires:Wires
@export var wire_offset:float
@export var parallel:String = ""

var _traction_rid: RID


func _make_curve(source_curve: MaszynaTrackCurve) -> Curve3D:
    var runtime_curve := Curve3D.new()
    runtime_curve.bake_interval = 10.0
    if not source_curve:
        return runtime_curve

    runtime_curve.add_point(source_curve.p1, Vector3.ZERO, Vector3.ZERO)
    runtime_curve.add_point(source_curve.p2, Vector3.ZERO, Vector3.ZERO)
    return runtime_curve


func _update():
    if not TractionRenderingServer or not _traction_rid or not _traction_rid.is_valid():
        return

    var runtime_curve1 := _make_curve(curve1)
    var runtime_curve2 := _make_curve(curve2)

    TractionRenderingServer.set_traction_geometry(_traction_rid, runtime_curve1, runtime_curve2, wire_thickness, wires)
    TractionRenderingServer.set_traction_transform(_traction_rid, global_transform)
    TractionRenderingServer.set_traction_visible(_traction_rid, visible)
    TractionRenderingServer.set_traction_scenario(_traction_rid, get_world_3d().scenario)
    TractionRenderingServer.set_traction_material(_traction_rid, TRACTION_MATERIAL.get_rid())


func _enter_tree():
    if TractionRenderingServer:
        _traction_rid = TractionRenderingServer.create_traction()
        _update()


func _exit_tree():
    if TractionRenderingServer and _traction_rid and _traction_rid.is_valid():
        TractionRenderingServer.free_traction(_traction_rid)
        _traction_rid = RID()


func _notification(what):
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED:
            if _traction_rid and TractionRenderingServer:
                TractionRenderingServer.set_traction_transform(_traction_rid, global_transform)
                TractionRenderingServer.set_traction_visible(_traction_rid, visible)


func calculate_p1_direction():
    if not curve1:
        return Vector3.ZERO
    return (curve1.p2-curve1.p1).normalized()


func calculate_p2_direction():
    if not curve1:
        return Vector3.ZERO
    return (curve1.p1-curve1.p2).normalized()


func get_curve_center():
    if not curve1:
        return Vector3.ZERO
    var p1 = curve1.p1
    var p2 = curve1.p2
    return p1+(p2-p1)*0.5
