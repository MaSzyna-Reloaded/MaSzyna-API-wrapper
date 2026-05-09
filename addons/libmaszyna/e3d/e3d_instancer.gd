@tool
extends Node
class_name E3DInstancer

const DEFAULT_LIGHT_ENERGY := 0.8
const DEFAULT_HIGHBEAM_LIGHT_ENERGY := 2.0
const DEFAULT_HEAD_LIGHT_ENERGY := 1.0
const DEFAULT_END_LIGHT_ENERGY := 1.0
const DEFAULT_LIGHT_SPOT_RANGE := 40.0
const FORCED_END_LIGHT_SPOT_RANGE := 3.0


var _colored_material: Material = preload("res://addons/libmaszyna/e3d/colored.material")


func instantiate(target_node: E3DModelInstance, model: E3DModel, editable: bool = false) -> void:
    push_error("%s.instantiate() must be implemented" % get_script().resource_path)


func clear(target_node: E3DModelInstance) -> void:
    push_error("%s.clear() must be implemented" % get_script().resource_path)


func sync(target_node: E3DModelInstance) -> void:
    push_error("%s.sync() must be implemented" % get_script().resource_path)


## Synchronizes the visibility of light nodes based on the target node's light state.
## Implementations should evaluate [method E3DModelInstance.lightts_state] and enable/disable
## appropriate visual nodes.
func sync_lights(target_node: E3DModelInstance) -> void:
    push_error("%s.sync_lights() must be implemented" % get_script().resource_path)


func configure_spotlight(spotlight: SpotLight3D, light_name: String, submodel: E3DSubModel) -> void:
    spotlight.light_energy = _determine_submodel_spotlight_energy(submodel, light_name)
    spotlight.spot_range = _get_spotlight_range(submodel, light_name)
    spotlight.spot_attenuation = _get_spotlight_distance_attenuation(submodel)
    spotlight.spot_angle_attenuation = _get_spotlight_angle_attenuation(submodel)


func _is_submodel_valid(target_node: E3DModelInstance, submodel: E3DSubModel) -> bool:
    if submodel.skip_rendering:
        return false

    match submodel.submodel_type:
        E3DSubModel.SubModelType.SUBMODEL_TRANSFORM, \
        E3DSubModel.SubModelType.SUBMODEL_FREE_SPOTLIGHT:
            return true
        E3DSubModel.SubModelType.SUBMODEL_GL_TRIANGLES:
            return not target_node.exclude_node_names.any(
                func(node_name: Variant) -> bool:
                    return submodel.resource_name == str(node_name)
            )

    return false


func _get_material_override(target_node: E3DModelInstance, submodel: E3DSubModel) -> Material:
    var unprefixed_model_path: String = "/".join(target_node.data_path.split("/").slice(1))
    var options = MaterialManager.MaterialOptions.new()

    # TODO: handle more material options here (selfillum, diffuse_color, etc)
    options.force_transparent = submodel.material_transparent
    options.selfillum_energy = options.selfillum_color.a  # legacy renderer
    options.selfillum_color = (
        submodel.self_illumination
        if submodel.self_illumination and not submodel.self_illumination == Color.BLACK
        else Color.WHITE
    )
    options.selfillum_enabled = options.selfillum_energy > 0.0 and submodel.lights_on_threshold >= 1.0  # legacy renderer logic

    if submodel.dynamic_material:
        if target_node.skins.size() < submodel.dynamic_material_index + 1:
            push_warning(
                "Model %s has no skins set, but submodel requires material #%s"
                % [target_node.name, submodel.dynamic_material_index]
            )
            return null

        var dynamic_transparency: int = (
            MaterialManager.Transparency.AlphaScissor
            if submodel.material_transparent
            else MaterialManager.Transparency.Disabled
        )
        var skin: Variant = target_node.skins[submodel.dynamic_material_index]
        return MaterialManager.get_material(unprefixed_model_path, skin, options)

    if submodel.material_colored:
        return _colored_material

    if submodel.material_name:
        var named_transparency: int = (
            MaterialManager.Transparency.AlphaScissor
            if submodel.material_transparent
            else MaterialManager.Transparency.Disabled
        )
        return MaterialManager.get_material(
            unprefixed_model_path,
            submodel.material_name,
            options,
        )

    return null


func _requires_alpha_depth_prepass_sorting(material: Material) -> bool:
    if material is BaseMaterial3D:
        var base_material: BaseMaterial3D = material as BaseMaterial3D
        return base_material.transparency == BaseMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS
    return false

func _determine_submodel_spotlight_energy(submodel: E3DSubModel, light_name: String) -> float:
    if light_name.begins_with("headlamp"):
        return DEFAULT_HEAD_LIGHT_ENERGY
    elif light_name.begins_with("highbeam"):
        return DEFAULT_HIGHBEAM_LIGHT_ENERGY
    elif light_name.begins_with("endsignal"):
        return DEFAULT_END_LIGHT_ENERGY
    elif light_name.begins_with("endtab"):
        return DEFAULT_END_LIGHT_ENERGY
    elif submodel.light_energy > 0.0:
        return submodel.light_energy
    else:
        return DEFAULT_LIGHT_ENERGY


func _get_spotlight_distance_attenuation(submodel: E3DSubModel) -> float:
    match submodel.far_attenuation_decay:
        0:
            return 0.0
        1:
            return 1.0
        2:
            return 2.0
        _:
            return 1.0


func _get_spotlight_angle_attenuation(submodel: E3DSubModel) -> float:
    var clamped_hotspot_cos: float = clampf(submodel.cos_hotspot_angle, -1.0, 1.0)
    var inner_angle: float = rad_to_deg(acos(clamped_hotspot_cos))
    var outer_angle: float = maxf(submodel.light_angle, 0.001)
    var penumbra_ratio: float = clampf((outer_angle - inner_angle) / outer_angle, 0.0, 1.0)
    return lerpf(2.0, 0.0, penumbra_ratio)


func _get_spotlight_range(submodel: E3DSubModel, light_name: String) -> float:
    if light_name.begins_with("endsignal") or light_name.begins_with("endtab"):
        return FORCED_END_LIGHT_SPOT_RANGE
    elif submodel.light_range > 0.0:
        return submodel.light_range
    else:
        return DEFAULT_LIGHT_SPOT_RANGE
