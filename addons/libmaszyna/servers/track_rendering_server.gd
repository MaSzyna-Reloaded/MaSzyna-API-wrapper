@tool
extends Node

const TRACK_SHADER_PATH: String = "res://addons/libmaszyna/materials/track_path_deform.gdshader"
const SLEEPER_SHADER_PATH: String = "res://addons/libmaszyna/materials/sleeper_path_deform.gdshader"
const BALLAST_SHADER_PATH: String = "res://addons/libmaszyna/materials/ballast_path_deform.gdshader"
const MATERIAL_TRANSPARENCY_DISABLED: int = 0
const MATERIAL_TRANSPARENCY_ALPHA: int = 1


class TrackState:
    var scenario: RID = RID()

    var rail_mesh_instance: RID = RID()
    var internal_rail_mesh: ArrayMesh
    var rail_material: ShaderMaterial

    var sleeper_multimesh_instance: RID = RID()
    var sleeper_multimesh: MultiMesh
    var sleeper_mesh: Mesh
    var sleeper_material: ShaderMaterial

    var ballast_mesh_instance: RID = RID()
    var internal_ballast_mesh: ArrayMesh
    var ballast_material: ShaderMaterial

    var path_points: PackedVector4Array = PackedVector4Array()
    var path_quats: PackedVector4Array = PackedVector4Array()
    var path_length: float = 0.0
    var sleeper_model_name: String = ""
    var sleeper_model_skin: String = ""
    var ballast_texture_path: String = ""

    var sleeper_height: float = -0.25
    var sleeper_spacing: float = 1.0
    var rail_spacing: float = 1.6
    var ballast_height: float = 0.4
    var ballast_offset: float = -0.05
    var ballast_uv_scale: float = 1.0
    var ballast_width_tiling: float = 1.0
    var ballast_length_tiling: float = 1.0

    var ballast_enabled: bool = true
    var visible: bool = true
    var transform: Transform3D = Transform3D.IDENTITY


var _tracks: Dictionary = {}
var _next_track_id: int = 1


func _exit_tree() -> void:
    var track_ids: Array = _tracks.keys()
    for track_id_variant: Variant in track_ids:
        free_track(int(track_id_variant))


func _get_track_state(track_id: int) -> TrackState:
    var state: TrackState = _tracks.get(track_id) as TrackState
    if state == null:
        push_warning("[TrackRenderingServer] Unknown track id: %d" % track_id)
    return state


func _free_track_state(state: TrackState) -> void:
    if state == null:
        return

    if state.rail_mesh_instance.is_valid():
        RenderingServer.free_rid(state.rail_mesh_instance)
    if state.ballast_mesh_instance.is_valid():
        RenderingServer.free_rid(state.ballast_mesh_instance)
    if state.sleeper_multimesh_instance.is_valid():
        RenderingServer.free_rid(state.sleeper_multimesh_instance)

    state.internal_rail_mesh = null
    state.internal_ballast_mesh = null
    state.sleeper_multimesh = null
    state.sleeper_mesh = null
    state.rail_material = null
    state.sleeper_material = null
    state.ballast_material = null


func _update_render_instances(state: TrackState) -> void:
    var active_scenario: RID = state.scenario if state.visible else RID()

    RenderingServer.instance_set_scenario(state.rail_mesh_instance, active_scenario)
    RenderingServer.instance_set_scenario(state.sleeper_multimesh_instance, active_scenario)
    RenderingServer.instance_set_scenario(
        state.ballast_mesh_instance,
        state.scenario if state.visible and state.ballast_enabled else RID()
    )

    RenderingServer.instance_set_transform(state.rail_mesh_instance, state.transform)
    RenderingServer.instance_set_transform(state.sleeper_multimesh_instance, state.transform)
    RenderingServer.instance_set_transform(state.ballast_mesh_instance, state.transform)


func _create_shader_material(shader_path: String) -> ShaderMaterial:
    var material: ShaderMaterial = ShaderMaterial.new()
    material.shader = load(shader_path)
    return material


func _apply_material_override(instance: RID, material: ShaderMaterial) -> void:
    RenderingServer.instance_geometry_set_material_override(
        instance,
        material.get_rid() if material != null else RID()
    )


func _clear_sleeper_material(state: TrackState) -> void:
    if state == null or state.sleeper_material == null:
        return

    state.sleeper_material.set_shader_parameter("albedo_tex", null)
    state.sleeper_material.set_shader_parameter("albedo", Color(1.0, 1.0, 1.0, 1.0))


func _get_material_manager() -> Object:
    if Engine.has_singleton("MaterialManager"):
        return Engine.get_singleton("MaterialManager")
    return null


func _get_model_manager() -> Node:
    return get_node_or_null("/root/E3DModelManager")


func _find_first_submodel_with_mesh(submodels: Array) -> Dictionary:
    var stack: Array[E3DSubModel] = []
    for submodel: E3DSubModel in submodels:
        stack.append(submodel)

    while not stack.is_empty():
        var current: E3DSubModel = stack.pop_back()
        if current == null:
            continue
        if current.mesh != null:
            return {
                "mesh": current.mesh,
                "material_name": current.material_name,
                "diffuse_color": current.diffuse_color,
                "material_colored": current.material_colored,
                "material_transparent": current.material_transparent,
            }
        for child: E3DSubModel in current.submodels:
            stack.append(child)

    return {}


func _reload_sleeper_model(track_id: int, state: TrackState) -> void:
    var model_path: String = state.sleeper_model_name.strip_edges()
    if model_path.begins_with("/"):
        model_path = model_path.substr(1)

    if model_path.is_empty():
        set_sleeper_mesh(track_id, null)
        _clear_sleeper_material(state)
        return

    var split_index: int = model_path.rfind("/")
    var data_path: String = model_path.substr(0, split_index) if split_index >= 0 else ""
    var model_filename: String = model_path.substr(split_index + 1) if split_index >= 0 else model_path
    if model_filename.is_empty():
        set_sleeper_mesh(track_id, null)
        _clear_sleeper_material(state)
        return

    var model_manager: Node = _get_model_manager()
    if model_manager == null:
        push_warning("[TrackRenderingServer] Missing E3DModelManager for sleeper loading")
        set_sleeper_mesh(track_id, null)
        _clear_sleeper_material(state)
        return

    var model: E3DModel = model_manager.call("load_model", data_path, model_filename) as E3DModel
    if model == null:
        set_sleeper_mesh(track_id, null)
        _clear_sleeper_material(state)
        return

    var found: Dictionary = _find_first_submodel_with_mesh(model.submodels)
    if found.is_empty():
        set_sleeper_mesh(track_id, null)
        _clear_sleeper_material(state)
        return

    var found_mesh: Mesh = found.get("mesh") as Mesh
    if found_mesh == null:
        set_sleeper_mesh(track_id, null)
        _clear_sleeper_material(state)
        return

    set_sleeper_mesh(track_id, found_mesh)

    var material_manager: Object = _get_material_manager()
    if material_manager == null:
        _clear_sleeper_material(state)
        return

    var found_material_name: String = str(found.get("material_name", ""))
    var found_diffuse: Color = found.get("diffuse_color", Color(1.0, 1.0, 1.0))
    var found_colored: bool = found.get("material_colored", false)
    var found_transparent: bool = found.get("material_transparent", false)
    var sleeper_model_skin: String = state.sleeper_model_skin.strip_edges()

    if not sleeper_model_skin.is_empty():
        found_material_name = sleeper_model_skin

    var material: StandardMaterial3D
    if found_colored:
        material = StandardMaterial3D.new()
        material.albedo_color = found_diffuse
    else:
        material = material_manager.call(
            "get_material",
            data_path,
            found_material_name,
            MATERIAL_TRANSPARENCY_ALPHA if found_transparent else MATERIAL_TRANSPARENCY_DISABLED,
            false,
            found_diffuse
        ) as StandardMaterial3D

    if material != null:
        state.sleeper_material.set_shader_parameter(
            "albedo_tex",
            material.get_texture(BaseMaterial3D.TEXTURE_ALBEDO)
        )
        state.sleeper_material.set_shader_parameter("albedo", material.albedo_color)
    else:
        _clear_sleeper_material(state)


func create_track() -> int:
    var state: TrackState = TrackState.new()
    state.rail_mesh_instance = RenderingServer.instance_create()
    state.ballast_mesh_instance = RenderingServer.instance_create()
    state.sleeper_multimesh_instance = RenderingServer.instance_create()
    state.sleeper_multimesh = MultiMesh.new()
    state.sleeper_multimesh.transform_format = MultiMesh.TRANSFORM_3D

    RenderingServer.instance_set_base(state.sleeper_multimesh_instance, state.sleeper_multimesh.get_rid())

    state.rail_material = _create_shader_material(TRACK_SHADER_PATH)
    _apply_material_override(state.rail_mesh_instance, state.rail_material)

    state.sleeper_material = _create_shader_material(SLEEPER_SHADER_PATH)
    _apply_material_override(state.sleeper_multimesh_instance, state.sleeper_material)

    state.ballast_material = _create_shader_material(BALLAST_SHADER_PATH)
    _apply_material_override(state.ballast_mesh_instance, state.ballast_material)

    var track_id: int = _next_track_id
    _next_track_id += 1
    _tracks[track_id] = state
    return track_id


func free_track(track_id: int) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    _free_track_state(state)
    _tracks.erase(track_id)


func set_track_transform(track_id: int, transform: Transform3D) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.transform = transform
    _update_render_instances(state)


func get_track_transform(track_id: int) -> Transform3D:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return Transform3D.IDENTITY
    return state.transform


func set_track_visible(track_id: int, visible: bool) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.visible = visible
    _update_render_instances(state)


func is_track_visible(track_id: int) -> bool:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return false
    return state.visible


func set_track_scenario(track_id: int, scenario: RID) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.scenario = scenario
    _update_render_instances(state)


func get_track_scenario(track_id: int) -> RID:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return RID()
    return state.scenario


func update_track_data(track_id: int, points: PackedVector4Array, quats: PackedVector4Array, length: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.path_points = points
    state.path_quats = quats
    state.path_length = length

    var safe_length: float = length if length > 0.001 else 0.001
    if points.size() > 0:
        var aabb: AABB = AABB(
            Vector3(points[0].x, points[0].y, points[0].z),
            Vector3.ZERO
        )
        for index: int in range(1, points.size()):
            var point: Vector4 = points[index]
            aabb = aabb.expand(Vector3(point.x, point.y, point.z))
        aabb = aabb.grow(5.0)

        RenderingServer.instance_set_custom_aabb(state.rail_mesh_instance, aabb)
        RenderingServer.instance_set_custom_aabb(state.sleeper_multimesh_instance, aabb)
        RenderingServer.instance_set_custom_aabb(state.ballast_mesh_instance, aabb)

    RenderingServer.instance_geometry_set_shader_parameter(state.rail_mesh_instance, "path_length", safe_length)
    RenderingServer.instance_geometry_set_shader_parameter(
        state.rail_mesh_instance,
        "path_point_count",
        points.size()
    )
    if state.rail_material != null:
        state.rail_material.set_shader_parameter("path_points", points)
        state.rail_material.set_shader_parameter("path_quats", quats)

    RenderingServer.instance_geometry_set_shader_parameter(state.ballast_mesh_instance, "path_length", safe_length)
    RenderingServer.instance_geometry_set_shader_parameter(
        state.ballast_mesh_instance,
        "path_point_count",
        points.size()
    )
    if state.ballast_material != null:
        state.ballast_material.set_shader_parameter("path_points", points)
        state.ballast_material.set_shader_parameter("path_quats", quats)
        state.ballast_material.set_shader_parameter("uv_scale", state.ballast_uv_scale)

    RenderingServer.instance_geometry_set_shader_parameter(
        state.sleeper_multimesh_instance,
        "path_length",
        safe_length
    )
    RenderingServer.instance_geometry_set_shader_parameter(
        state.sleeper_multimesh_instance,
        "path_point_count",
        points.size()
    )
    RenderingServer.instance_geometry_set_shader_parameter(
        state.sleeper_multimesh_instance,
        "sleeper_height",
        state.sleeper_height
    )

    var safe_spacing: float = state.sleeper_spacing if state.sleeper_spacing > 0.001 else 0.001
    RenderingServer.instance_geometry_set_shader_parameter(
        state.sleeper_multimesh_instance,
        "sleeper_spacing",
        safe_spacing
    )

    if state.sleeper_material != null:
        state.sleeper_material.set_shader_parameter("path_points", points)
        state.sleeper_material.set_shader_parameter("path_quats", quats)

    if state.sleeper_multimesh != null and state.sleeper_mesh != null:
        var count: int = int(floor(safe_length / safe_spacing))
        state.sleeper_multimesh.mesh = state.sleeper_mesh
        state.sleeper_multimesh.instance_count = max(count, 0)
        state.sleeper_multimesh.visible_instance_count = -1

        for index: int in range(count):
            state.sleeper_multimesh.set_instance_transform(index, Transform3D.IDENTITY)
    elif state.sleeper_multimesh != null:
        state.sleeper_multimesh.mesh = null
        state.sleeper_multimesh.instance_count = 0
        state.sleeper_multimesh.visible_instance_count = -1

    _update_render_instances(state)


func set_sleeper_model_name(track_id: int, sleeper_model_name: String) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    if state.sleeper_model_name == sleeper_model_name:
        return

    state.sleeper_model_name = sleeper_model_name
    _reload_sleeper_model(track_id, state)


func set_sleeper_model_skin(track_id: int, sleeper_model_skin: String) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    if state.sleeper_model_skin == sleeper_model_skin:
        return

    state.sleeper_model_skin = sleeper_model_skin
    _reload_sleeper_model(track_id, state)


func set_sleeper_mesh(track_id: int, mesh: Mesh) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.sleeper_mesh = mesh
    if state.sleeper_multimesh != null:
        state.sleeper_multimesh.mesh = mesh
        state.sleeper_multimesh.visible_instance_count = -1

    update_track_data(track_id, state.path_points, state.path_quats, state.path_length)


func get_sleeper_mesh(track_id: int) -> Mesh:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return null
    return state.sleeper_mesh


func set_ballast_uv_scale(track_id: int, scale: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.ballast_uv_scale = scale
    if state.ballast_material != null:
        state.ballast_material.set_shader_parameter("uv_scale", state.ballast_uv_scale)


func get_ballast_uv_scale(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.ballast_uv_scale


func set_ballast_texture(track_id: int, path: String) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    var texture_path: String = path.strip_edges()
    if state.ballast_texture_path == texture_path:
        return

    state.ballast_texture_path = texture_path
    if state.ballast_material == null:
        return

    var material_manager: Object = _get_material_manager()
    if material_manager == null:
        push_warning("[TrackRenderingServer] Failed to get MaterialManager singleton")
        return

    var texture: Texture2D = material_manager.call("get_texture", texture_path) as Texture2D
    if texture == null:
        push_warning("[TrackRenderingServer] Ballast texture is not supported")
        return

    state.ballast_material.set_shader_parameter("albedo_tex", texture)
    state.ballast_material.set_shader_parameter("albedo", Color(1.0, 1.0, 1.0, 1.0))


func update_ballast_mesh(track_id: int, length: float, curve_precision: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null or length < 0.1:
        return

    var old_length: float = 0.0
    var old_height: float = 0.0
    var old_offset: float = 0.0
    var old_width_tiling: float = 0.0
    var old_length_tiling: float = 0.0

    if state.internal_ballast_mesh != null:
        old_length = float(state.internal_ballast_mesh.get_meta("length", 0.0))
        old_height = float(state.internal_ballast_mesh.get_meta("ballast_height", 0.0))
        old_offset = float(state.internal_ballast_mesh.get_meta("ballast_offset", 0.0))
        old_width_tiling = float(state.internal_ballast_mesh.get_meta("ballast_width_tiling", 0.0))
        old_length_tiling = float(state.internal_ballast_mesh.get_meta("ballast_length_tiling", 0.0))

    var should_generate: bool = state.internal_ballast_mesh == null
    if not should_generate:
        should_generate = (
            abs(old_length - length) > 1.0
            or abs(old_height - state.ballast_height) > 0.001
            or abs(old_offset - state.ballast_offset) > 0.001
            or abs(old_width_tiling - state.ballast_width_tiling) > 0.001
            or abs(old_length_tiling - state.ballast_length_tiling) > 0.001
        )

    if not should_generate:
        return

    var safe_curve_precision: float = curve_precision if curve_precision > 0.001 else 0.001
    var steps: int = max(2, int(floor(length / safe_curve_precision)))

    var ballast_poly: PackedVector2Array = PackedVector2Array([
        Vector2(-1.25, 0.0),
        Vector2(1.25, 0.0),
        Vector2(2.0, -state.ballast_height),
        Vector2(-2.0, -state.ballast_height),
    ])

    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()
    var poly_size: int = ballast_poly.size()

    for step: int in range(steps + 1):
        var ratio: float = float(step) / float(steps)
        var z_position: float = ratio * length
        var v_u: float = ratio * length

        for point_index: int in range(poly_size):
            var point: Vector2 = ballast_poly[point_index]
            vertices.push_back(Vector3(point.x, point.y + state.ballast_offset, z_position))

            var normal: Vector3 = Vector3.UP
            if point_index == 1 or point_index == 2:
                normal = Vector3(0.47, 0.88, 0.0)
            elif point_index == 3 or point_index == 0:
                normal = Vector3(-0.47, 0.88, 0.0)

            normals.push_back(normal)
            uvs.push_back(Vector2(point.x * state.ballast_width_tiling, v_u * state.ballast_length_tiling))

    for step: int in range(steps):
        for point_index: int in range(poly_size):
            var next_point_index: int = (point_index + 1) % poly_size
            var current: int = step * poly_size + point_index
            var next_current: int = step * poly_size + next_point_index
            var next_step_current: int = (step + 1) * poly_size + point_index
            var next_step_next: int = (step + 1) * poly_size + next_point_index

            indices.push_back(current)
            indices.push_back(next_step_current)
            indices.push_back(next_current)
            indices.push_back(next_current)
            indices.push_back(next_step_current)
            indices.push_back(next_step_next)

    var arrays: Array = []
    arrays.resize(Mesh.ARRAY_MAX)
    arrays[Mesh.ARRAY_VERTEX] = vertices
    arrays[Mesh.ARRAY_NORMAL] = normals
    arrays[Mesh.ARRAY_TEX_UV] = uvs
    arrays[Mesh.ARRAY_INDEX] = indices

    var new_mesh: ArrayMesh = ArrayMesh.new()
    new_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
    new_mesh.set_meta("length", length)
    new_mesh.set_meta("ballast_height", state.ballast_height)
    new_mesh.set_meta("ballast_offset", state.ballast_offset)
    new_mesh.set_meta("ballast_width_tiling", state.ballast_width_tiling)
    new_mesh.set_meta("ballast_length_tiling", state.ballast_length_tiling)

    state.internal_ballast_mesh = new_mesh
    RenderingServer.instance_set_base(state.ballast_mesh_instance, state.internal_ballast_mesh.get_rid())


func update_rail_mesh(track_id: int, length: float, rail_spacing: float, curve_precision: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null or length < 0.1:
        return

    var old_length: float = 0.0
    var old_spacing: float = 0.0
    if state.internal_rail_mesh != null:
        old_length = float(state.internal_rail_mesh.get_meta("length", 0.0))
        old_spacing = float(state.internal_rail_mesh.get_meta("rail_spacing", 0.0))

    var should_generate: bool = state.internal_rail_mesh == null
    if not should_generate:
        should_generate = abs(old_length - length) > 1.0 or abs(old_spacing - rail_spacing) > 0.001

    if not should_generate:
        return

    var safe_curve_precision: float = curve_precision if curve_precision > 0.001 else 0.001
    var steps: int = max(2, int(floor(length / safe_curve_precision)))

    var rail_poly: PackedVector2Array = PackedVector2Array([
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
    ])

    var vertices: PackedVector3Array = PackedVector3Array()
    var normals: PackedVector3Array = PackedVector3Array()
    var uvs: PackedVector2Array = PackedVector2Array()
    var indices: PackedInt32Array = PackedInt32Array()
    var offset: float = rail_spacing * 0.5
    var poly_size: int = rail_poly.size()

    for step: int in range(steps + 1):
        var ratio: float = float(step) / float(steps)
        var z_position: float = ratio * length
        var v_u: float = ratio * length

        for side: int in [-1, 1]:
            for point_index: int in range(poly_size):
                var point: Vector2 = rail_poly[point_index]
                var x_position: float = side * offset + point.x * side
                vertices.push_back(Vector3(x_position, point.y, z_position))
                normals.push_back(Vector3(point.x, point.y - 0.08, 0.0).normalized())
                uvs.push_back(Vector2(float(point_index) / float(poly_size - 1), v_u))

    var vertices_per_step: int = poly_size * 2
    for step: int in range(steps):
        for side_index: int in range(2):
            var side_offset: int = side_index * poly_size
            for point_index: int in range(poly_size):
                var next_point_index: int = (point_index + 1) % poly_size
                var current: int = step * vertices_per_step + side_offset + point_index
                var next_current: int = step * vertices_per_step + side_offset + next_point_index
                var next_step_current: int = (step + 1) * vertices_per_step + side_offset + point_index
                var next_step_next: int = (step + 1) * vertices_per_step + side_offset + next_point_index

                if side_index == 1:
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

    var arrays: Array = []
    arrays.resize(Mesh.ARRAY_MAX)
    arrays[Mesh.ARRAY_VERTEX] = vertices
    arrays[Mesh.ARRAY_NORMAL] = normals
    arrays[Mesh.ARRAY_TEX_UV] = uvs
    arrays[Mesh.ARRAY_INDEX] = indices

    var new_mesh: ArrayMesh = ArrayMesh.new()
    new_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
    new_mesh.set_meta("length", length)
    new_mesh.set_meta("rail_spacing", rail_spacing)

    state.internal_rail_mesh = new_mesh
    RenderingServer.instance_set_base(state.rail_mesh_instance, state.internal_rail_mesh.get_rid())


func set_sleeper_height(track_id: int, height: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.sleeper_height = height
    update_track_data(track_id, state.path_points, state.path_quats, state.path_length)


func get_sleeper_height(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.sleeper_height


func set_sleeper_spacing(track_id: int, spacing: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.sleeper_spacing = spacing
    update_track_data(track_id, state.path_points, state.path_quats, state.path_length)


func get_sleeper_spacing(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.sleeper_spacing


func set_rail_spacing(track_id: int, spacing: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.rail_spacing = spacing
    update_track_data(track_id, state.path_points, state.path_quats, state.path_length)


func get_rail_spacing(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.rail_spacing


func set_ballast_height(track_id: int, height: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.ballast_height = height
    update_ballast_mesh(track_id, state.path_length, 0.5)


func get_ballast_height(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.ballast_height


func set_ballast_offset(track_id: int, offset: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.ballast_offset = offset
    update_ballast_mesh(track_id, state.path_length, 0.5)


func get_ballast_offset(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.ballast_offset


func set_ballast_width_tiling(track_id: int, tiling: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.ballast_width_tiling = tiling
    update_ballast_mesh(track_id, state.path_length, 0.5)


func get_ballast_width_tiling(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.ballast_width_tiling


func set_ballast_length_tiling(track_id: int, tiling: float) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.ballast_length_tiling = tiling
    update_ballast_mesh(track_id, state.path_length, 0.5)


func get_ballast_length_tiling(track_id: int) -> float:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return 0.0
    return state.ballast_length_tiling


func set_ballast_enabled(track_id: int, enabled: bool) -> void:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return

    state.ballast_enabled = enabled
    _update_render_instances(state)


func get_ballast_enabled(track_id: int) -> bool:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return false
    return state.ballast_enabled


func get_instance_rid(track_id: int) -> RID:
    return get_rail_mesh_instance(track_id)


func get_rail_mesh_instance(track_id: int) -> RID:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return RID()
    return state.rail_mesh_instance


func get_sleeper_multimesh_instance(track_id: int) -> RID:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return RID()
    return state.sleeper_multimesh_instance


func get_ballast_mesh_instance(track_id: int) -> RID:
    var state: TrackState = _get_track_state(track_id)
    if state == null:
        return RID()
    return state.ballast_mesh_instance
