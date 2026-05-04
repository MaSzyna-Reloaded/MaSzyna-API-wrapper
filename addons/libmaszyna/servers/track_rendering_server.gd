@tool
extends Node


class TrackState:
    var rail_mesh_instance: RID = RID()
    var rail_material: ShaderMaterial
    var rail_mesh: Mesh
    var curve: Curve3D
    
    var sleeper_chunk_multimesh_instances: Array[RID] = []
    var sleeper_chunk_multimeshes: Array[MultiMesh] = []
    var sleeper_material: ShaderMaterial

    var ballast_mesh_instance: RID = RID()
    var ballast_mesh: Mesh
    var ballast_material: ShaderMaterial
    
    var visible: bool = true
    var transform: Transform3D = Transform3D.IDENTITY


const sleepers_multimesh_max_distance: float = 150.0

const _RAIL_POLY: Array[Vector2] = [
    Vector2(-0.075, 0.0),
    Vector2(0.075, 0.0),
    Vector2(0.075, 0.015),
    Vector2(0.02, 0.03),
    Vector2(0.02, 0.12),
    Vector2(0.04, 0.13),
    Vector2(0.04, 0.16),
    Vector2(-0.04, 0.16),
    Vector2(-0.04, 0.13),
    Vector2(-0.02, 0.12),
    Vector2(-0.02, 0.03),
    Vector2(-0.075, 0.015),
]

var _tracks: Dictionary[RID, TrackState] = {}
var _next_track_id: int = 0


func create_track() -> RID:
    var state: TrackState = TrackState.new()
    state.rail_mesh_instance = RenderingServer.instance_create()
    state.ballast_mesh_instance = RenderingServer.instance_create()
    
    _next_track_id += 1
    var track_rid: RID = rid_from_int64(_next_track_id)
    _tracks[track_rid] = state
    return track_rid


func _free_sleeper_chunks(state: TrackState) -> void:
    for multimesh_instance in state.sleeper_chunk_multimesh_instances:
        if multimesh_instance.is_valid():
            RenderingServer.free_rid(multimesh_instance)

    state.sleeper_chunk_multimeshes.clear()
    state.sleeper_chunk_multimesh_instances.clear()


func _free_track_mesh_instances(state: TrackState) -> void:
    if state.rail_mesh_instance.is_valid():
        RenderingServer.free_rid(state.rail_mesh_instance)
    if state.ballast_mesh_instance.is_valid():
        RenderingServer.free_rid(state.ballast_mesh_instance)
    _free_sleeper_chunks(state)


func free_track(track_rid: RID) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    _free_track_mesh_instances(state)

    state.rail_mesh = null
    state.ballast_mesh = null
    state.rail_material = null
    state.sleeper_material = null
    state.ballast_material = null
    
    _tracks.erase(track_rid)


func set_track_transform(track_rid: RID, transform: Transform3D) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    state.transform = transform
    RenderingServer.instance_set_transform(state.rail_mesh_instance, state.transform)
    RenderingServer.instance_set_transform(state.ballast_mesh_instance, state.transform)
    for multimesh_instance in state.sleeper_chunk_multimesh_instances:
        RenderingServer.instance_set_transform(multimesh_instance, state.transform)


func get_track_transform(track_rid: RID) -> Transform3D:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return Transform3D.IDENTITY
    return state.transform


func set_track_visible(track_rid: RID, visible: bool) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    state.visible = visible
    RenderingServer.instance_set_visible(state.rail_mesh_instance, state.visible)
    RenderingServer.instance_set_visible(state.ballast_mesh_instance, state.visible)
    for multimesh_instance in state.sleeper_chunk_multimesh_instances:
        RenderingServer.instance_set_visible(multimesh_instance, state.visible)


func is_track_visible(track_rid: RID) -> bool:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return false
    return state.visible


func set_track_scenario(track_rid: RID, scenario: RID) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    RenderingServer.instance_set_scenario(state.rail_mesh_instance, scenario)
    RenderingServer.instance_set_scenario(state.ballast_mesh_instance, scenario)
    for multimesh_instance in state.sleeper_chunk_multimesh_instances:
        RenderingServer.instance_set_scenario(multimesh_instance, scenario)


func set_track_curve(
    track_rid: RID,
    curve: Curve3D,
    rail_spacing: float,
    sleeper_spacing: float,
    sleeper_height: float,
    ballast_enabled: bool = false,
    ballast_height: float = 0.0,
    ballast_offset: float = 0.0,
    ballast_width_tiling: float = 0.0,
    ballast_length_tiling: float = 0.0,
    ballast_uv_scale: float = 1.0
) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return
    state.curve = curve

    RenderingServer.instance_set_base(state.rail_mesh_instance, RID())
    RenderingServer.instance_set_base(state.ballast_mesh_instance, RID())
    
    if not state.curve or state.curve.point_count < 2:
        state.rail_mesh = null
        state.ballast_mesh = null
        _free_sleeper_chunks(state)
        return

    var length: float = state.curve.get_baked_length()
    if length <= 0.001:
        state.rail_mesh = null
        state.ballast_mesh = null
        _free_sleeper_chunks(state)
        return

    state.rail_mesh = _create_rail_mesh(state.curve, rail_spacing)
    RenderingServer.instance_set_base(state.rail_mesh_instance, state.rail_mesh.get_rid())

    if ballast_enabled:
        state.ballast_mesh = _create_ballast_mesh(
            state.curve,
            ballast_height,
            ballast_offset,
            ballast_width_tiling,
            ballast_length_tiling,
            ballast_uv_scale
        )
        RenderingServer.instance_set_base(state.ballast_mesh_instance, state.ballast_mesh.get_rid())
    else:
        state.ballast_mesh = null
        RenderingServer.instance_set_base(state.ballast_mesh_instance, RID())

    _create_sleeper_chunks(state, state.curve, length, sleeper_spacing, sleeper_height)


func set_sleeper_mesh(track_rid: RID, mesh: Mesh) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    if not mesh:
        _free_sleeper_chunks(state)
        return

    for multimesh in state.sleeper_chunk_multimeshes:
        multimesh.mesh = mesh


func set_track_materials(track_rid: RID, ballast_material: RID, rail_material: RID, sleeper_material: RID) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return    
    RenderingServer.instance_geometry_set_material_override(
        state.ballast_mesh_instance, ballast_material)
    RenderingServer.instance_geometry_set_material_override(
        state.rail_mesh_instance, rail_material)
    for multimesh_instance in state.sleeper_chunk_multimesh_instances:
        RenderingServer.instance_geometry_set_material_override(
            multimesh_instance, sleeper_material)

func _create_ballast_mesh(
    curve: Curve3D,
    height: float,
    offset: float,
    width_tiling: float,
    length_tiling: float,
    uv_scale: float
) -> ArrayMesh:
    var baked_points: PackedVector3Array = curve.get_baked_points()
    if baked_points.size() < 2:
        return ArrayMesh.new()

    var safe_uv_scale: float = uv_scale if uv_scale > 0.001 else 1.0

    var ballast_poly: PackedVector2Array = PackedVector2Array([
        Vector2(-1.25, 0.0),
        Vector2(1.25, 0.0),
        Vector2(2.0, -height),
        Vector2(-2.0, -height),
    ])

    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()
    var poly_size: int = ballast_poly.size()
    var distance: float = 0.0

    for sample_index: int in range(baked_points.size()):
        var v_u: float = distance
        var frame: Transform3D = curve.sample_baked_with_rotation(distance, false, true)
        frame.origin = baked_points[sample_index]

        for point_index: int in range(poly_size):
            var point: Vector2 = ballast_poly[point_index]
            vertices.push_back(frame * Vector3(point.x, point.y + offset, 0.0))

            var normal: Vector3 = Vector3.UP
            if point_index == 1 or point_index == 2:
                normal = Vector3(0.47, 0.88, 0.0)
            elif point_index == 3 or point_index == 0:
                normal = Vector3(-0.47, 0.88, 0.0)

            normals.push_back(frame.basis * normal)
            uvs.push_back(Vector2(point.x * width_tiling * safe_uv_scale, v_u * length_tiling * safe_uv_scale))

        if sample_index < baked_points.size() - 1:
            distance += baked_points[sample_index].distance_to(baked_points[sample_index + 1])

    _append_extruded_indices(indices, baked_points.size(), poly_size, 0, poly_size, false)

    return _build_array_mesh(vertices, normals, uvs, indices)


func _create_rail_mesh(curve: Curve3D, rail_spacing: float) -> Mesh:
    var baked_points: PackedVector3Array = curve.get_baked_points()
    if baked_points.size() < 2:
        return ArrayMesh.new()

    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()
    var offset: float = rail_spacing * 0.5
    var poly_size: int = _RAIL_POLY.size()
    var distance: float = 0.0

    for sample_index: int in range(baked_points.size()):
        var v_u: float = distance
        var frame: Transform3D = curve.sample_baked_with_rotation(distance, false, true)
        frame.origin = baked_points[sample_index]

        for side: int in [-1, 1]:
            for point_index: int in range(poly_size):
                var point: Vector2 = _RAIL_POLY[point_index]
                var x_position: float = side * offset + point.x * side
                vertices.push_back(frame * Vector3(x_position, point.y, 0.0))
                normals.push_back(frame.basis * Vector3(point.x * side, point.y - 0.08, 0.0).normalized())
                uvs.push_back(Vector2(float(point_index) / float(poly_size - 1), v_u))

        if sample_index < baked_points.size() - 1:
            distance += baked_points[sample_index].distance_to(baked_points[sample_index + 1])

    var vertices_per_step: int = poly_size * 2
    _append_extruded_indices(indices, baked_points.size(), poly_size, 0, vertices_per_step, false)
    _append_extruded_indices(indices, baked_points.size(), poly_size, poly_size, vertices_per_step, true)

    return _build_array_mesh(vertices, normals, uvs, indices)


func _append_extruded_indices(
    indices: PackedInt32Array,
    step_count: int,
    section_size: int,
    section_offset: int,
    vertices_per_step: int,
    reverse_winding: bool
) -> void:
    for step: int in range(step_count - 1):
        for point_index: int in range(section_size):
            var next_point_index: int = (point_index + 1) % section_size
            var current: int = step * vertices_per_step + section_offset + point_index
            var next_current: int = step * vertices_per_step + section_offset + next_point_index
            var next_step_current: int = (step + 1) * vertices_per_step + section_offset + point_index
            var next_step_next: int = (step + 1) * vertices_per_step + section_offset + next_point_index

            if reverse_winding:
                indices.push_back(current)
                indices.push_back(next_current)
                indices.push_back(next_step_current)
                indices.push_back(next_current)
                indices.push_back(next_step_next)
                indices.push_back(next_step_current)
            else:
                indices.push_back(current)
                indices.push_back(next_step_current)
                indices.push_back(next_current)
                indices.push_back(next_current)
                indices.push_back(next_step_current)
                indices.push_back(next_step_next)


func _build_array_mesh(
    vertices: PackedVector3Array,
    normals: PackedVector3Array,
    uvs: PackedVector2Array,
    indices: PackedInt32Array
) -> ArrayMesh:
    var arrays: Array = []
    arrays.resize(Mesh.ARRAY_MAX)
    arrays[Mesh.ARRAY_VERTEX] = vertices
    arrays[Mesh.ARRAY_NORMAL] = normals
    arrays[Mesh.ARRAY_TEX_UV] = uvs
    arrays[Mesh.ARRAY_INDEX] = indices

    var mesh: ArrayMesh = ArrayMesh.new()
    mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
    return mesh


func _create_sleeper_chunks(
    state: TrackState,
    curve: Curve3D,
    length: float,
    sleeper_spacing: float,
    sleeper_height: float
) -> void:
    _free_sleeper_chunks(state)

    if sleeper_spacing <= 0.001 or length <= 0.001:
        return

    var chunk_length: float = max(sleepers_multimesh_max_distance * 0.25, sleeper_spacing)
    var sleeper_count: int = int(floor(length / sleeper_spacing)) + 1
    var sleepers_per_chunk: int = max(1, int(floor(chunk_length / sleeper_spacing)))

    var sleeper_index: int = 0
    while sleeper_index < sleeper_count:
        var multimesh_instance = RenderingServer.instance_create()
        var multimesh = MultiMesh.new()
        multimesh.transform_format = MultiMesh.TRANSFORM_3D

        var chunk_sleeper_count: int = min(sleepers_per_chunk, sleeper_count - sleeper_index)
        multimesh.instance_count = chunk_sleeper_count

        RenderingServer.instance_geometry_set_visibility_range(
            multimesh_instance,
            0.0,
            sleepers_multimesh_max_distance,
            0.5,
            2.0,
            RenderingServer.VISIBILITY_RANGE_FADE_SELF
        )
        RenderingServer.instance_set_base(multimesh_instance, multimesh.get_rid())
        state.sleeper_chunk_multimeshes.append(multimesh)
        state.sleeper_chunk_multimesh_instances.append(multimesh_instance)

        for chunk_sleeper_index: int in range(chunk_sleeper_count):
            var distance: float = min(float(sleeper_index + chunk_sleeper_index) * sleeper_spacing, length)
            var frame: Transform3D = curve.sample_baked_with_rotation(distance, false, true)
            var basis: Basis = Basis(-frame.basis.x, frame.basis.y, -frame.basis.z)
            var origin: Vector3 = frame.origin + frame.basis.y * sleeper_height
            multimesh.set_instance_transform(chunk_sleeper_index, Transform3D(basis, origin))

        sleeper_index += chunk_sleeper_count
