@tool
extends Node


class TrackState:
    var primary_rail_mesh_instance: RID = RID()
    var primary_rail_mesh: Mesh
    var secondary_rail_mesh_instance: RID = RID()
    var secondary_rail_mesh: Mesh
    var curve: Curve3D
    var curve2: Curve3D

    var trackbed_mesh_instance: RID = RID()
    var trackbed_mesh: Mesh

    var visible: bool = true
    var transform: Transform3D = Transform3D.IDENTITY


const _DEFAULT_RAIL_PROFILE: Array = [
    [0.111, -0.180, 1.000, 0.000, 0.00],
    [0.046, -0.150, 0.707, 0.707, 0.15],
    [0.044, -0.050, 0.707, -0.707, 0.25],
    [0.073, -0.038, 0.707, -0.707, 0.35],
    [0.072, -0.010, 0.707, 0.707, 0.40],
    [0.052, -0.000, 0.000, 1.000, 0.45],
    [0.020, -0.000, 0.000, 1.000, 0.55],
    [0.000, -0.010, -0.707, 0.707, 0.60],
    [-0.001, -0.038, -0.707, -0.707, 0.65],
    [0.028, -0.050, -0.707, -0.707, 0.75],
    [0.026, -0.150, -0.707, 0.707, 0.85],
    [-0.039, -0.180, -1.000, 0.000, 1.00],
]

var _tracks: Dictionary[RID, TrackState] = {}
var _next_track_id: int = 0
var _rail_profile_cache: Dictionary = {}
var _rail_profile_regex: RegEx
const _SWITCH_BLADE_RATIO: float = 0.65
const _SWITCH_TRACKBED_Z_FIGHT_OFFSET: float = 0.025


func create_track() -> RID:
    var state: TrackState = TrackState.new()
    state.primary_rail_mesh_instance = RenderingServer.instance_create()
    state.secondary_rail_mesh_instance = RenderingServer.instance_create()
    state.trackbed_mesh_instance = RenderingServer.instance_create()

    _next_track_id += 1
    var track_rid: RID = rid_from_int64(_next_track_id)
    _tracks[track_rid] = state
    return track_rid


func free_track(track_rid: RID) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    _reset_track_instances(state)

    if state.primary_rail_mesh_instance.is_valid():
        RenderingServer.free_rid(state.primary_rail_mesh_instance)
    if state.secondary_rail_mesh_instance.is_valid():
        RenderingServer.free_rid(state.secondary_rail_mesh_instance)
    if state.trackbed_mesh_instance.is_valid():
        RenderingServer.free_rid(state.trackbed_mesh_instance)

    state.primary_rail_mesh = null
    state.secondary_rail_mesh = null
    state.trackbed_mesh = null
    _tracks.erase(track_rid)


func _reset_track_instances(state: TrackState) -> void:
    if state.primary_rail_mesh_instance.is_valid():
        RenderingServer.instance_set_base(state.primary_rail_mesh_instance, RID())
    if state.secondary_rail_mesh_instance.is_valid():
        RenderingServer.instance_set_base(state.secondary_rail_mesh_instance, RID())
    if state.trackbed_mesh_instance.is_valid():
        RenderingServer.instance_set_base(state.trackbed_mesh_instance, RID())


func set_track_transform(track_rid: RID, transform: Transform3D) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    state.transform = transform
    RenderingServer.instance_set_transform(state.primary_rail_mesh_instance, state.transform)
    RenderingServer.instance_set_transform(state.secondary_rail_mesh_instance, state.transform)
    RenderingServer.instance_set_transform(state.trackbed_mesh_instance, state.transform)


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
    RenderingServer.instance_set_visible(state.primary_rail_mesh_instance, state.visible)
    RenderingServer.instance_set_visible(state.secondary_rail_mesh_instance, state.visible)
    RenderingServer.instance_set_visible(state.trackbed_mesh_instance, state.visible)


func is_track_visible(track_rid: RID) -> bool:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return false
    return state.visible


func set_track_scenario(track_rid: RID, scenario: RID) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    RenderingServer.instance_set_scenario(state.primary_rail_mesh_instance, scenario)
    RenderingServer.instance_set_scenario(state.secondary_rail_mesh_instance, scenario)
    RenderingServer.instance_set_scenario(state.trackbed_mesh_instance, scenario)


func set_track_curve(
    track_rid: RID,
    curve: Curve3D,
    curve2: Curve3D,
    width: float,
    tex_length: float,
    tex_height: float,
    tex_width: float,
    tex_slope: float,
    railprofile: String,
    roll1: float,
    roll2: float,
    roll3: float,
    roll4: float,
    next_trackbed: Dictionary,
    switch_trackbed: Dictionary,
    trackbed_enabled: bool
) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return
    state.curve = curve
    state.curve2 = curve2

    RenderingServer.instance_set_base(state.primary_rail_mesh_instance, RID())
    RenderingServer.instance_set_base(state.secondary_rail_mesh_instance, RID())
    RenderingServer.instance_set_base(state.trackbed_mesh_instance, RID())

    if not state.curve or state.curve.point_count < 2:
        state.primary_rail_mesh = null
        state.secondary_rail_mesh = null
        state.trackbed_mesh = null
        return

    var length: float = state.curve.get_baked_length()
    if length <= 0.001:
        state.primary_rail_mesh = null
        state.secondary_rail_mesh = null
        state.trackbed_mesh = null
        return

    var rail_profile: Dictionary = _get_rail_profile(railprofile)
    state.primary_rail_mesh = null
    state.secondary_rail_mesh = null

    state.trackbed_mesh = null
    if _is_switch_curve(state.curve2):
        var switch_geometry: Dictionary = _create_switch_rail_meshes(
            state.curve,
            state.curve2,
            width,
            tex_length,
            roll1,
            roll2,
            roll3,
            roll4,
            rail_profile
        )
        state.primary_rail_mesh = switch_geometry.get("primary")
        state.secondary_rail_mesh = switch_geometry.get("secondary")
        if trackbed_enabled and not switch_trackbed.is_empty():
            state.trackbed_mesh = _create_switch_trackbed_mesh(
                state.curve,
                state.curve2,
                width,
                tex_length,
                switch_trackbed,
                roll1,
                roll2,
                roll3,
                roll4,
                rail_profile
            )
    else:
        state.primary_rail_mesh = _create_rail_mesh(
            state.curve,
            width,
            roll1,
            roll2,
            rail_profile["rail"],
            tex_length
        )
        if trackbed_enabled:
            state.trackbed_mesh = _create_trackbed_mesh(
                state.curve,
                width,
                tex_length,
                tex_height,
                tex_width,
                tex_slope,
                roll1,
                roll2,
                next_trackbed,
                rail_profile["rail"]
            )

    if state.primary_rail_mesh:
        RenderingServer.instance_set_base(state.primary_rail_mesh_instance, state.primary_rail_mesh.get_rid())
    if state.secondary_rail_mesh:
        RenderingServer.instance_set_base(state.secondary_rail_mesh_instance, state.secondary_rail_mesh.get_rid())
    if state.trackbed_mesh:
        RenderingServer.instance_set_base(state.trackbed_mesh_instance, state.trackbed_mesh.get_rid())


func set_sleeper_mesh(_track_rid: RID, _mesh: Mesh) -> void:
    pass


func set_track_materials(track_rid: RID, trackbed_material: Material, primary_rail_material: Material, secondary_rail_material: Material) -> void:
    var state: TrackState = _tracks.get(track_rid)
    if not state:
        return

    if state.primary_rail_mesh and primary_rail_material:
        state.primary_rail_mesh.surface_set_material(0, primary_rail_material)
    if state.secondary_rail_mesh and secondary_rail_material:
        state.secondary_rail_mesh.surface_set_material(0, secondary_rail_material)
    if state.trackbed_mesh and trackbed_material:
        state.trackbed_mesh.surface_set_material(0, trackbed_material)


func _is_switch_curve(curve: Curve3D) -> bool:
    return curve != null and curve.point_count >= 2 and curve.get_baked_length() > 0.001


func _create_rail_mesh(
    curve: Curve3D,
    width: float,
    roll1: float,
    roll2: float,
    rail_profile: Array,
    tex_length: float
) -> ArrayMesh:
    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()
    var rail_height: float = abs(float(rail_profile[0][1]))

    var right_start: Array = _build_rail_section(rail_profile, width, roll1, false)
    var right_end: Array = _build_rail_section(rail_profile, width, roll2, false)
    _append_loft_geometry(vertices, normals, uvs, indices, curve, right_start, right_end, tex_length, rail_height)

    var left_start: Array = _build_rail_section(rail_profile, width, roll1, true)
    var left_end: Array = _build_rail_section(rail_profile, width, roll2, true)
    _append_loft_geometry(vertices, normals, uvs, indices, curve, left_start, left_end, tex_length, rail_height)

    return _build_array_mesh(vertices, normals, uvs, indices)


func _create_trackbed_mesh(
    curve: Curve3D,
    width: float,
    tex_length: float,
    tex_height: float,
    tex_width: float,
    tex_slope: float,
    roll1: float,
    roll2: float,
    next_trackbed: Dictionary,
    rail_profile: Array
) -> ArrayMesh:
    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()

    var rail_height: float = abs(float(rail_profile[0][1]))
    var end_width: float = width
    var end_tex_height: float = tex_height
    var end_tex_width: float = tex_width
    var end_tex_slope: float = tex_slope

    if not next_trackbed.is_empty():
        end_width = float(next_trackbed.get("width", width))
        end_tex_height = float(next_trackbed.get("tex_height", tex_height))
        end_tex_width = float(next_trackbed.get("tex_width", tex_width))
        end_tex_slope = float(next_trackbed.get("tex_slope", tex_slope))

    var start_section: Array = _build_trackbed_section(
        width,
        tex_height,
        tex_width,
        tex_slope,
        roll1,
        rail_height,
        tex_length
    )
    var end_section: Array = _build_trackbed_section(
        end_width,
        end_tex_height,
        end_tex_width,
        end_tex_slope,
        roll2,
        rail_height,
        tex_length
    )
    _append_loft_geometry(vertices, normals, uvs, indices, curve, start_section, end_section, tex_length, rail_height)

    return _build_array_mesh(vertices, normals, uvs, indices)


func _create_switch_rail_meshes(
    primary_curve: Curve3D,
    secondary_curve: Curve3D,
    width: float,
    tex_length: float,
    roll1: float,
    roll2: float,
    roll3: float,
    roll4: float,
    rail_profile: Dictionary
) -> Dictionary:
    var primary_vertices: PackedVector3Array = PackedVector3Array()
    var primary_normals: PackedVector3Array = PackedVector3Array()
    var primary_uvs: PackedVector2Array = PackedVector2Array()
    var primary_indices: PackedInt32Array = PackedInt32Array()
    var secondary_vertices: PackedVector3Array = PackedVector3Array()
    var secondary_normals: PackedVector3Array = PackedVector3Array()
    var secondary_uvs: PackedVector2Array = PackedVector2Array()
    var secondary_indices: PackedInt32Array = PackedInt32Array()

    var regular_profile: Array = rail_profile["rail"]
    var blade_profile: Array = rail_profile["blade"]
    var rail_height: float = abs(float(regular_profile[0][1]))
    var is_right_switch: bool = _is_right_switch(primary_curve, secondary_curve)
    var blade_sample_count: int = _get_switch_blade_sample_count(primary_curve)

    var primary_inner_mirrored: bool = is_right_switch
    var primary_outer_mirrored: bool = not primary_inner_mirrored
    var secondary_inner_mirrored: bool = not is_right_switch
    var secondary_outer_mirrored: bool = not secondary_inner_mirrored

    _append_switch_rail_geometry(
        primary_vertices,
        primary_normals,
        primary_uvs,
        primary_indices,
        primary_curve,
        regular_profile,
        blade_profile,
        width,
        roll1,
        roll2,
        tex_length,
        rail_height,
        primary_inner_mirrored,
        primary_outer_mirrored,
        blade_sample_count
    )
    _append_switch_rail_geometry(
        secondary_vertices,
        secondary_normals,
        secondary_uvs,
        secondary_indices,
        secondary_curve,
        regular_profile,
        blade_profile,
        width,
        roll3,
        roll4,
        tex_length,
        rail_height,
        secondary_inner_mirrored,
        secondary_outer_mirrored,
        blade_sample_count
    )

    return {
        "primary": _build_array_mesh(primary_vertices, primary_normals, primary_uvs, primary_indices),
        "secondary": _build_array_mesh(secondary_vertices, secondary_normals, secondary_uvs, secondary_indices),
    }


func _append_switch_rail_geometry(
    vertices: PackedVector3Array,
    normals: PackedVector3Array,
    uvs: PackedVector2Array,
    indices: PackedInt32Array,
    curve: Curve3D,
    regular_profile: Array,
    blade_profile: Array,
    width: float,
    roll_start: float,
    roll_end: float,
    tex_length: float,
    rail_height: float,
    inner_mirrored: bool,
    outer_mirrored: bool,
    blade_sample_count: int
) -> void:
    var outer_start: Array = _build_rail_section(regular_profile, width, roll_start, outer_mirrored)
    var outer_end: Array = _build_rail_section(regular_profile, width, roll_end, outer_mirrored)
    _append_loft_geometry(vertices, normals, uvs, indices, curve, outer_start, outer_end, tex_length, rail_height)

    var regular_start: Array = _build_rail_section(regular_profile, width, roll_start, inner_mirrored)
    var regular_end: Array = _build_rail_section(regular_profile, width, roll_end, inner_mirrored)
    var blade_start: Array = _build_rail_section(blade_profile, width, roll_start, inner_mirrored)

    _append_loft_geometry_range(
        vertices,
        normals,
        uvs,
        indices,
        curve,
        blade_start,
        regular_end,
        tex_length,
        rail_height,
        0,
        blade_sample_count
    )
    _append_loft_geometry_range(
        vertices,
        normals,
        uvs,
        indices,
        curve,
        regular_start,
        regular_end,
        tex_length,
        rail_height,
        max(blade_sample_count - 1, 0),
        -1
    )


func _create_switch_trackbed_mesh(
    primary_curve: Curve3D,
    secondary_curve: Curve3D,
    width: float,
    tex_length: float,
    trackbed_profile: Dictionary,
    roll1: float,
    roll2: float,
    roll3: float,
    roll4: float,
    rail_profile: Dictionary
) -> ArrayMesh:
    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()
    var rail_height: float = abs(float(rail_profile["rail"][0][1]))
    var is_right_switch: bool = _is_right_switch(primary_curve, secondary_curve)
    var profile_width: float = float(trackbed_profile.get("width", width))
    var tex_height: float = float(trackbed_profile.get("tex_height", 0.0))
    var tex_width: float = float(trackbed_profile.get("tex_width", 0.0))
    var tex_slope: float = float(trackbed_profile.get("tex_slope", 0.0))

    var primary_start: Array = _build_trackbed_section(profile_width, tex_height, tex_width, tex_slope, roll1, rail_height, tex_length)
    var primary_end: Array = _build_trackbed_section(profile_width, tex_height, tex_width, tex_slope, roll2, rail_height, tex_length)
    var secondary_start: Array = _build_trackbed_section(profile_width, tex_height, tex_width, tex_slope, roll3, rail_height, tex_length)
    var secondary_end: Array = _build_trackbed_section(profile_width, tex_height, tex_width, tex_slope, roll4, rail_height, tex_length)

    if is_right_switch:
        _lower_trackbed_edge(primary_start, true)
        _lower_trackbed_edge(primary_end, true)
        _lower_trackbed_edge(secondary_start, false)
        _lower_trackbed_edge(secondary_end, false)
    else:
        _lower_trackbed_edge(primary_start, false)
        _lower_trackbed_edge(primary_end, false)
        _lower_trackbed_edge(secondary_start, true)
        _lower_trackbed_edge(secondary_end, true)

    _append_loft_geometry(vertices, normals, uvs, indices, primary_curve, primary_start, primary_end, tex_length, rail_height)
    _append_loft_geometry(vertices, normals, uvs, indices, secondary_curve, secondary_start, secondary_end, tex_length, rail_height)

    return _build_array_mesh(vertices, normals, uvs, indices)


func _build_rail_section(profile: Array, width: float, roll: float, mirrored: bool) -> Array:
    var section: Array = []
    var half_width: float = 0.5 * abs(width)
    var roll_radians: float = deg_to_rad(roll)
    var sin_roll: float = sin(roll_radians)
    var cos_roll: float = cos(roll_radians)

    for point_data: Array in profile:
        var x: float = float(point_data[0])
        var y: float = float(point_data[1])
        var nx: float = float(point_data[2])
        var ny: float = float(point_data[3])
        var uv_x: float = float(point_data[4])

        var position: Vector3
        var normal: Vector3
        if mirrored:
            position = Vector3(
                (-half_width - x) * cos_roll + y * sin_roll,
                -(-half_width - x) * sin_roll + y * cos_roll,
                0.0
            )
            normal = Vector3(
                -nx * cos_roll + ny * sin_roll,
                nx * sin_roll + ny * cos_roll,
                0.0
            )
            section.push_front([position, normal.normalized(), uv_x])
        else:
            position = Vector3(
                (half_width + x) * cos_roll + y * sin_roll,
                -(half_width + x) * sin_roll + y * cos_roll,
                0.0
            )
            normal = Vector3(
                nx * cos_roll + ny * sin_roll,
                -nx * sin_roll + ny * cos_roll,
                0.0
            )
            section.append([position, normal.normalized(), uv_x])

    return section


func _build_trackbed_section(
    width: float,
    tex_height: float,
    tex_width: float,
    tex_slope: float,
    roll: float,
    rail_height: float,
    tex_length: float
) -> Array:
    var half_width: float = 0.5 * abs(width)
    var side: float = abs(tex_width)
    var slope_abs: float = abs(tex_slope)
    var spread: float = half_width + side + slope_abs
    var hypot_value: float = sqrt(slope_abs * slope_abs + tex_height * tex_height)
    if is_zero_approx(hypot_value):
        hypot_value = 1.0

    var normal_x: float = tex_height / hypot_value
    var normal_y: float = tex_slope / hypot_value
    if is_zero_approx(normal_x) and is_zero_approx(normal_y):
        normal_x = 1.0
        normal_y = 0.0

    var roll_radians: float = deg_to_rad(roll)
    var sin_roll: float = sin(roll_radians)
    var cos_roll: float = cos(roll_radians)
    var safe_tex_length: float = max(abs(tex_length), 0.001)
    var map_inner: float = (half_width + side) / safe_tex_length
    var map_outer: float = (half_width + side + hypot_value) / safe_tex_length

    return [
        [
            Vector3(spread, -tex_height - rail_height, 0.0),
            Vector3(normal_x, normal_y, 0.0).normalized(),
            0.5 - map_outer
        ],
        [
            Vector3(
                (half_width + side) * cos_roll,
                -(half_width + side) * sin_roll - rail_height,
                0.0
            ),
            Vector3(normal_x, normal_y, 0.0).normalized(),
            0.5 - map_inner
        ],
        [
            Vector3(0.0, -rail_height + 0.01, 0.0),
            Vector3.UP,
            0.5
        ],
        [
            Vector3(
                -(half_width + side) * cos_roll,
                (half_width + side) * sin_roll - rail_height,
                0.0
            ),
            Vector3(-normal_x, normal_y, 0.0).normalized(),
            0.5 + map_inner
        ],
        [
            Vector3(-spread, -tex_height - rail_height, 0.0),
            Vector3(-normal_x, normal_y, 0.0).normalized(),
            0.5 + map_outer
        ],
    ]


func _append_loft_geometry(
    vertices: PackedVector3Array,
    normals: PackedVector3Array,
    uvs: PackedVector2Array,
    indices: PackedInt32Array,
    curve: Curve3D,
    start_section: Array,
    end_section: Array,
    tex_length: float,
    rail_height: float
) -> void:
    _append_loft_geometry_range(
        vertices,
        normals,
        uvs,
        indices,
        curve,
        start_section,
        end_section,
        tex_length,
        rail_height,
        0,
        -1
    )


func _append_loft_geometry_range(
    vertices: PackedVector3Array,
    normals: PackedVector3Array,
    uvs: PackedVector2Array,
    indices: PackedInt32Array,
    curve: Curve3D,
    start_section: Array,
    end_section: Array,
    tex_length: float,
    rail_height: float,
    start_sample: int,
    end_sample: int
) -> void:
    if start_section.size() < 2 or start_section.size() != end_section.size():
        return

    var baked_points: PackedVector3Array = curve.get_baked_points()
    if baked_points.size() < 2:
        return

    var first_sample: int = clampi(start_sample, 0, baked_points.size() - 1)
    var last_sample: int = baked_points.size() - 1 if end_sample < 0 else clampi(end_sample, 0, baked_points.size() - 1)
    if last_sample <= first_sample:
        return

    var safe_tex_length: float = max(abs(tex_length), 0.001)
    var section_size: int = start_section.size()
    var base_vertex: int = vertices.size()
    var distance: float = 0.0
    var total_length: float = max(_get_baked_segment_length(baked_points, first_sample, last_sample), 0.001)

    for sample_index: int in range(first_sample, last_sample + 1):
        var t: float = distance / total_length
        var frame: Transform3D = _build_curve_frame(baked_points, sample_index)
        frame.origin.y += rail_height

        for point_index: int in range(section_size):
            var start_point: Array = start_section[point_index]
            var end_point: Array = end_section[point_index]

            var local_position: Vector3 = (start_point[0] as Vector3).lerp(end_point[0] as Vector3, t)
            var local_normal: Vector3 = (start_point[1] as Vector3).lerp(end_point[1] as Vector3, t).normalized()
            var uv_x: float = lerpf(float(start_point[2]), float(end_point[2]), t)

            vertices.push_back(frame * local_position)
            normals.push_back((frame.basis * local_normal).normalized())
            uvs.push_back(Vector2(uv_x, distance / safe_tex_length))

        if sample_index < last_sample:
            distance += baked_points[sample_index].distance_to(baked_points[sample_index + 1])

    _append_loft_strip_indices(indices, last_sample - first_sample + 1, section_size, base_vertex)


func _get_baked_segment_length(baked_points: PackedVector3Array, start_sample: int, end_sample: int) -> float:
    var length: float = 0.0
    for sample_index: int in range(start_sample, end_sample):
        length += baked_points[sample_index].distance_to(baked_points[sample_index + 1])
    return length


func _build_curve_frame(baked_points: PackedVector3Array, sample_index: int) -> Transform3D:
    var origin: Vector3 = baked_points[sample_index]
    var tangent: Vector3
    if sample_index == 0:
        tangent = baked_points[1] - baked_points[0]
    elif sample_index == baked_points.size() - 1:
        tangent = baked_points[sample_index] - baked_points[sample_index - 1]
    else:
        tangent = baked_points[sample_index + 1] - baked_points[sample_index - 1]

    if tangent.length_squared() <= 0.000001:
        tangent = Vector3.FORWARD
    else:
        tangent = tangent.normalized()

    var reference_up: Vector3 = Vector3.UP
    if abs(tangent.dot(reference_up)) > 0.999:
        reference_up = Vector3.RIGHT

    var right: Vector3 = reference_up.cross(tangent).normalized()
    var up: Vector3 = tangent.cross(right).normalized()
    var basis: Basis = Basis(right, up, tangent).orthonormalized()
    return Transform3D(basis, origin)


func _append_loft_strip_indices(
    indices: PackedInt32Array,
    step_count: int,
    section_size: int,
    base_vertex: int
) -> void:
    for step: int in range(step_count - 1):
        var current_base: int = base_vertex + step * section_size
        var next_base: int = base_vertex + (step + 1) * section_size
        for point_index: int in range(section_size - 1):
            var current: int = current_base + point_index
            var current_next: int = current_base + point_index + 1
            var next_current: int = next_base + point_index
            var next_next: int = next_base + point_index + 1

            indices.push_back(current)
            indices.push_back(next_current)
            indices.push_back(current_next)
            indices.push_back(current_next)
            indices.push_back(next_current)
            indices.push_back(next_next)


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


func _get_rail_profile(profile_name: String) -> Dictionary:
    var normalized_name: String = profile_name.strip_edges()
    if normalized_name.is_empty():
        normalized_name = "default"

    if _rail_profile_cache.has(normalized_name):
        return _rail_profile_cache[normalized_name]

    var profile: Dictionary = _load_rail_profile(normalized_name)
    if profile.is_empty():
        profile = {
            "rail": _DEFAULT_RAIL_PROFILE.duplicate(true),
            "blade": _DEFAULT_RAIL_PROFILE.duplicate(true),
        }
    elif profile["blade"].is_empty():
        profile["blade"] = (profile["rail"] as Array).duplicate(true)

    _rail_profile_cache[normalized_name] = profile
    return profile


func _load_rail_profile(profile_name: String) -> Dictionary:
    var game_dir: String = UserSettings.get_maszyna_game_dir()
    if game_dir.is_empty():
        return {}

    var normalized_name: String = profile_name
    if normalized_name.begins_with("railprofile_"):
        normalized_name = normalized_name.trim_prefix("railprofile_")

    var profile_path: String = game_dir.path_join("models").path_join("tory").path_join(
        "railprofile_%s.txt" % normalized_name
    )
    if not FileAccess.file_exists(profile_path):
        return {}

    var file: FileAccess = FileAccess.open(profile_path, FileAccess.READ)
    if not file:
        return {}

    var rail_profile: Array = []
    var blade_profile: Array = []
    var regex: RegEx = _get_rail_profile_regex()
    var read_blade_profile: bool = false
    while not file.eof_reached():
        var line: String = file.get_line().strip_edges()
        if line.is_empty():
            continue
        if line.begins_with("//"):
            if line.begins_with("// switch blade"):
                read_blade_profile = true
            continue

        var matches: Array[RegExMatch] = regex.search_all(line)
        if matches.size() < 5:
            continue

        var point := [
            float(matches[0].get_string()),
            float(matches[1].get_string()),
            float(matches[2].get_string()),
            float(matches[3].get_string()),
            float(matches[4].get_string()),
        ]
        if read_blade_profile:
            blade_profile.append(point)
        else:
            rail_profile.append(point)

    return {
        "rail": rail_profile,
        "blade": blade_profile,
    }


func _get_rail_profile_regex() -> RegEx:
    if _rail_profile_regex:
        return _rail_profile_regex

    _rail_profile_regex = RegEx.new()
    _rail_profile_regex.compile("-?\\d+(?:\\.\\d+)?")
    return _rail_profile_regex


func _is_right_switch(primary_curve: Curve3D, secondary_curve: Curve3D) -> bool:
    var primary_tangent: Vector3 = _get_curve_tangent(primary_curve, 0)
    var secondary_tangent: Vector3 = _get_curve_tangent(secondary_curve, 0)
    return primary_tangent.signed_angle_to(secondary_tangent, Vector3.UP) < 0.0


func _get_curve_tangent(curve: Curve3D, sample_index: int) -> Vector3:
    var baked_points: PackedVector3Array = curve.get_baked_points()
    if baked_points.size() < 2:
        return Vector3.FORWARD
    var clamped_index: int = clampi(sample_index, 0, baked_points.size() - 2)
    var tangent: Vector3 = baked_points[clamped_index + 1] - baked_points[clamped_index]
    if tangent.length_squared() <= 0.000001:
        return Vector3.FORWARD
    return tangent.normalized()


func _get_switch_blade_sample_count(curve: Curve3D) -> int:
    var sample_count: int = curve.get_baked_points().size()
    return clampi(int(ceil(sample_count * _SWITCH_BLADE_RATIO)), 2, sample_count - 1)


func _lower_trackbed_edge(section: Array, lower_right: bool) -> void:
    var indexes: Array = [0, 1] if lower_right else [3, 4]
    for section_index: int in indexes:
        var point: Array = section[section_index]
        point[0] = (point[0] as Vector3) + Vector3(0.0, -_SWITCH_TRACKBED_Z_FIGHT_OFFSET, 0.0)
        section[section_index] = point
