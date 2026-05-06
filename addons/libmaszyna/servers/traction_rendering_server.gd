@tool
extends Node


class TractionState:
    var primary_mesh_instance: RID = RID()
    var secondary_mesh_instance: RID = RID()
    var primary_mesh: Mesh
    var secondary_mesh: Mesh
    var visible: bool = true
    var transform: Transform3D = Transform3D.IDENTITY


const _MIN_WIRE_RADIUS: float = 0.001
const _SINGLE_WIRE_MODE: int = 0

var _tractions: Dictionary[RID, TractionState] = {}
var _next_traction_id: int = 0


func create_traction() -> RID:
    var state: TractionState = TractionState.new()
    state.primary_mesh_instance = RenderingServer.instance_create()
    state.secondary_mesh_instance = RenderingServer.instance_create()

    _next_traction_id += 1
    var traction_rid: RID = rid_from_int64(_next_traction_id)
    _tractions[traction_rid] = state
    return traction_rid


func free_traction(traction_rid: RID) -> void:
    var state: TractionState = _tractions.get(traction_rid)
    if not state:
        return

    if state.primary_mesh_instance.is_valid():
        RenderingServer.free_rid(state.primary_mesh_instance)
    if state.secondary_mesh_instance.is_valid():
        RenderingServer.free_rid(state.secondary_mesh_instance)

    state.primary_mesh = null
    state.secondary_mesh = null
    _tractions.erase(traction_rid)


func set_traction_scenario(traction_rid: RID, scenario: RID) -> void:
    var state: TractionState = _tractions.get(traction_rid)
    if not state:
        return

    RenderingServer.instance_set_scenario(state.primary_mesh_instance, scenario)
    RenderingServer.instance_set_scenario(state.secondary_mesh_instance, scenario)


func set_traction_transform(traction_rid: RID, transform: Transform3D) -> void:
    var state: TractionState = _tractions.get(traction_rid)
    if not state:
        return

    state.transform = transform
    RenderingServer.instance_set_transform(state.primary_mesh_instance, transform)
    RenderingServer.instance_set_transform(state.secondary_mesh_instance, transform)


func set_traction_visible(traction_rid: RID, visible: bool) -> void:
    var state: TractionState = _tractions.get(traction_rid)
    if not state:
        return

    state.visible = visible
    RenderingServer.instance_set_visible(state.primary_mesh_instance, visible)
    RenderingServer.instance_set_visible(state.secondary_mesh_instance, visible)


func set_traction_material(traction_rid: RID, material_rid: RID) -> void:
    var state: TractionState = _tractions.get(traction_rid)
    if not state:
        return

    RenderingServer.instance_geometry_set_material_override(state.primary_mesh_instance, material_rid)
    RenderingServer.instance_geometry_set_material_override(state.secondary_mesh_instance, material_rid)


func set_traction_geometry(
    traction_rid: RID,
    curve1: Curve3D,
    curve2: Curve3D,
    wire_thickness: float,
    wires: int
) -> void:
    var state: TractionState = _tractions.get(traction_rid)
    if not state:
        return

    state.primary_mesh = _create_wire_mesh(curve1, wire_thickness)
    RenderingServer.instance_set_base(state.primary_mesh_instance, state.primary_mesh.get_rid() if state.primary_mesh else RID())

    var has_secondary_wire: bool = wires != _SINGLE_WIRE_MODE
    if has_secondary_wire:
        state.secondary_mesh = _create_wire_mesh(curve2, wire_thickness)
        RenderingServer.instance_set_base(state.secondary_mesh_instance, state.secondary_mesh.get_rid() if state.secondary_mesh else RID())
    else:
        state.secondary_mesh = null
        RenderingServer.instance_set_base(state.secondary_mesh_instance, RID())

    RenderingServer.instance_set_visible(state.primary_mesh_instance, state.visible)
    RenderingServer.instance_set_visible(state.secondary_mesh_instance, state.visible and has_secondary_wire)


func _create_wire_profile(thickness: float) -> PackedVector2Array:
    var radius: float = max(thickness * 0.004, _MIN_WIRE_RADIUS)
    return PackedVector2Array([
        Vector2(-radius, -radius * 0.25),
        Vector2(radius, -radius * 0.25),
        Vector2(radius, radius * 0.25),
        Vector2(-radius, radius * 0.25),
    ])


func _create_wire_mesh(curve: Curve3D, wire_thickness: float) -> ArrayMesh:
    var mesh: ArrayMesh = ArrayMesh.new()
    if not curve or curve.point_count < 2:
        return mesh

    var baked_points: PackedVector3Array = curve.get_baked_points()
    if baked_points.size() < 2:
        return mesh

    var profile: PackedVector2Array = _create_wire_profile(wire_thickness)
    var profile_size: int = profile.size()
    var surface_tool: SurfaceTool = SurfaceTool.new()
    var distance: float = 0.0

    surface_tool.begin(Mesh.PRIMITIVE_TRIANGLES)

    for sample_index: int in range(baked_points.size()):
        if sample_index > 0:
            distance += baked_points[sample_index - 1].distance_to(baked_points[sample_index])

        var frame: Transform3D = curve.sample_baked_with_rotation(distance, false, true)
        frame.origin = baked_points[sample_index]

        for profile_index: int in range(profile_size):
            var profile_point: Vector2 = profile[profile_index]
            surface_tool.set_uv(Vector2(float(profile_index) / float(profile_size), distance))
            surface_tool.add_vertex(frame * Vector3(profile_point.x, profile_point.y, 0.0))

    for segment_index: int in range(baked_points.size() - 1):
        var row_offset: int = segment_index * profile_size
        var next_row_offset: int = row_offset + profile_size
        for profile_index: int in range(profile_size):
            var next_profile_index: int = (profile_index + 1) % profile_size
            var a: int = row_offset + profile_index
            var b: int = row_offset + next_profile_index
            var c: int = next_row_offset + profile_index
            var d: int = next_row_offset + next_profile_index
            surface_tool.add_index(a)
            surface_tool.add_index(b)
            surface_tool.add_index(c)
            surface_tool.add_index(b)
            surface_tool.add_index(d)
            surface_tool.add_index(c)

    surface_tool.generate_normals()
    var committed_mesh: ArrayMesh = surface_tool.commit()
    return committed_mesh
