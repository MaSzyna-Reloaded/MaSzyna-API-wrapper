@tool
extends Node3D
class_name MaszynaTraction3D

const TRACTION_MATERIAL = preload("res://addons/libmaszyna/materials/traction.material")

#static var eu07_exporter = load("res://addons/maszyna/exporters/node_traction.gd").new()
#static var eu07_importer = load("res://addons/maszyna/importers/node_traction.gd").new()

enum Wires {JEZDNY, JEDNY_NOSNY, DWA_JEZDNE_NOSNY, DWA_JEZDNE_DWA_NOSNE}

func str_or_none(x):
    return str(x) if x else "none"

var _dirty := false

@export var curve1:MaszynaTrackCurve
@export var curve2:MaszynaTrackCurve
@export var power_supply_name:String
@export var nominal_voltage:float
@export var max_current:float
@export var resistivity:float
@export_enum("cu", "al") var material:String
@export var wire_thickness:float:  # mm
    set(x):
        wire_thickness = x
        _dirty = true

@export_flags("Patyna:1", "Zerwanie:128") var damage_flag:int
@export var min_height:float
@export var segment_length:float
@export var wires:Wires
@export var wire_offset:float
@export var parallel:String = ""



var _path1:Path3D
var _path2:Path3D
var _poly1:CSGPolygon3D
var _poly2:CSGPolygon3D


func _generate_circle_polygon(radius: float, num_sides: int, position: Vector2) -> PackedVector2Array:
    var angle_delta: float = (PI * 2) / num_sides
    var vector: Vector2 = Vector2(radius, 0)
    var polygon: PackedVector2Array

    for _i in num_sides:
        polygon.append(vector + position)
        vector = vector.rotated(angle_delta)

    return polygon


func _init():
    _path1 = Path3D.new()
    _path2 = Path3D.new()
    _poly1 = CSGPolygon3D.new()
    _poly2 = CSGPolygon3D.new()


    _poly1.position = Vector3.ZERO
    _poly2.position = Vector3.ZERO



    _poly1.path_interval = 10.0
    _poly1.path_interval_type = CSGPolygon3D.PATH_INTERVAL_DISTANCE
    _poly2.path_interval = 10.0
    _poly2.path_interval_type = CSGPolygon3D.PATH_INTERVAL_DISTANCE

const VIS_WIDTH = 0.2

func _update():
    _update_path(_path1, curve1)
    var center_y = Vector2(0, (curve2.p1-curve1.p1).y * 0.5)
    var thickness_scale = 0.001
    var wire_circle = _generate_circle_polygon(wire_thickness * thickness_scale, 6, Vector2.ZERO)
    

    _poly1.polygon = wire_circle
    _poly2.polygon = wire_circle
    _poly2.visible = not wires == Wires.JEZDNY
    _poly1.mode = CSGPolygon3D.MODE_PATH

    

    if not wires == Wires.JEZDNY:
        _update_path(_path2, curve2)
        _poly2.visible = true
        _poly2.mode = CSGPolygon3D.MODE_PATH
    else:
        _poly2.mode = CSGPolygon3D.MODE_DEPTH



func _enter_tree():
    add_child(_path1)
    add_child(_path2)
    add_child(_poly1)
    add_child(_poly2)

    _poly1.material = TRACTION_MATERIAL
    _poly2.material = TRACTION_MATERIAL
    
    _poly1.path_node = _poly1.get_path_to(_path1)
    _poly2.path_node = _poly2.get_path_to(_path2)
    
    _update()


func _exit_tree():
    _poly1.path_node = NodePath("")
    _poly2.path_node = NodePath("")
    
    remove_child(_path1)
    remove_child(_path2)
    remove_child(_poly1)
    remove_child(_poly2)
    

func _update_path(path:Path3D, curve:MaszynaTrackCurve):
    var _curve = Curve3D.new()
    _curve.add_point(curve.p1, Vector3.ZERO, Vector3.ZERO)
    _curve.add_point(curve.p2, Vector3.ZERO, Vector3.ZERO)
    path.curve = _curve
    

func calculate_p1_direction():
    var off = _path1.curve.get_closest_offset(curve1.p1)
    var s1 = _path1.curve.sample_baked(off, true)
    var s2 = _path1.curve.sample_baked(off+0.1, true)
    if s1 == s2:
        return (curve1.p2-curve1.p1).normalized()
    else:
        return (s2-s1).normalized()

func calculate_p2_direction():
    var off = _path1.curve.get_closest_offset(curve1.p2)
    var s1 = _path1.curve.sample_baked(off, true)
    var s2 = _path1.curve.sample_baked(off-0.1, true)
    if s1 == s2:
        return (curve1.p1-curve1.p2).normalized()
    else:
        return (s2-s1).normalized()

func get_curve_center():
    var p1 = curve1.p1
    var p2 = curve1.p2
    return p1+(p2-p1)*0.5
