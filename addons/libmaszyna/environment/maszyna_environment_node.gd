@tool
extends Node
class_name MaszynaEnvironmentNode


@export var data_path: String = "models":
    set(value):
        if value == data_path:
            return
        data_path = value
        _update_sky_shader()

@export var model_filename: String = "":
    set(value):
        if value == model_filename:
            return
        model_filename = value
        _update_sky_shader()


func _ready() -> void:
    if not UserSettings.config_changed.is_connected(_update_sky_shader):
        UserSettings.config_changed.connect(_update_sky_shader)
    _update_sky_shader()


func _exit_tree() -> void:
    if UserSettings.config_changed.is_connected(_update_sky_shader):
        UserSettings.config_changed.disconnect(_update_sky_shader)


func _update_sky_shader() -> void:
    if not is_node_ready() or not MaterialManager:
        return

    var sky_material: ShaderMaterial = _get_sky_material()
    if not sky_material:
        return

    var sky_data: Dictionary = _extract_sky_shader_data()
    if not sky_data.get("texture"):
        return

    RenderingServer.global_shader_parameter_set("sky_texture", sky_data.get("texture"))
    sky_material.set_shader_parameter("sky_offset", sky_data.get("offset", Vector2.ZERO))
    sky_material.set_shader_parameter("sky_scale", sky_data.get("scale", Vector2.ONE))


func _get_sky_material() -> ShaderMaterial:
    var environment:Environment = get_viewport().get_camera_3d().get_world_3d().environment
    if not environment:
        return null

    if not environment or not environment.sky:
        return null

    return environment.sky.sky_material as ShaderMaterial


func _extract_sky_shader_data() -> Dictionary:
    var output: Dictionary = {
        "texture": null,
        "offset": Vector2.ZERO,
        "scale": Vector2.ONE,
    }
    if model_filename.is_empty():
        return output

    var model: E3DModel = E3DModelManager.load_model(data_path, model_filename)
    if not model:
        return output

    var state: Dictionary = {
        "material_name": "",
        "texture": null,
        "uv_min": Vector2(INF, INF),
        "uv_max": Vector2(-INF, -INF),
    }
    _collect_sky_data(model.submodels, state)
    if not state["texture"]:
        return output

    var uv_min: Vector2 = state["uv_min"] as Vector2
    var uv_max: Vector2 = state["uv_max"] as Vector2
    output["texture"] = state["texture"]
    output["offset"] = Vector2((uv_min.x + uv_max.x) * 0.5, uv_min.y)
    output["scale"] = Vector2((uv_max.x - uv_min.x) * 0.5, uv_max.y - uv_min.y)
    return output


func _collect_sky_data(submodels: Array, state: Dictionary) -> void:
    for item: Variant in submodels:
        var submodel: E3DSubModel = item as E3DSubModel
        if not submodel:
            continue

        if submodel.submodel_type == E3DSubModel.SubModelType.SUBMODEL_GL_TRIANGLES:
            _append_submodel_uvs(submodel, state)

        if submodel.submodels.size() > 0:
            _collect_sky_data(submodel.submodels, state)


func _append_submodel_uvs(submodel: E3DSubModel, state: Dictionary) -> void:
    if not submodel.mesh or submodel.material_name.is_empty():
        return

    var material_name: String = submodel.material_name
    if String(state["material_name"]).is_empty():
        state["material_name"] = material_name
        state["texture"] = _load_sky_texture(material_name)
    elif material_name != String(state["material_name"]):
        return

    if not state["texture"] or submodel.mesh.get_surface_count() == 0:
        return

    var arrays: Array = submodel.mesh.surface_get_arrays(0)
    var uvs: PackedVector2Array = arrays[Mesh.ARRAY_TEX_UV] as PackedVector2Array
    for uv: Vector2 in uvs:
        var uv_min: Vector2 = state["uv_min"] as Vector2
        var uv_max: Vector2 = state["uv_max"] as Vector2
        state["uv_min"] = uv_min.min(uv)
        state["uv_max"] = uv_max.max(uv)


func _load_sky_texture(material_name: String) -> Texture2D:
    var material: MaszynaMaterial = MaterialParser.parse(data_path, material_name)
    if not material or material.albedo_texture_path.is_empty():
        return null
    return MaterialManager.load_texture(data_path, material.albedo_texture_path) as Texture2D
