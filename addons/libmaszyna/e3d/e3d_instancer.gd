@tool
extends Node
class_name E3DInstancer

var _colored_material: Material = preload("res://addons/libmaszyna/e3d/colored.material")


func instantiate(target_node: E3DModelInstance, model: E3DModel, editable: bool = false) -> void:
    push_error("%s.instantiate() must be implemented" % get_script().resource_path)


func clear(target_node: E3DModelInstance) -> void:
    push_error("%s.clear() must be implemented" % get_script().resource_path)


func sync(target_node: E3DModelInstance) -> void:
    push_error("%s.sync() must be implemented" % get_script().resource_path)


func _is_submodel_valid(target_node: E3DModelInstance, submodel: E3DSubModel) -> bool:
    if submodel.skip_rendering:
        return false

    match submodel.submodel_type:
        E3DSubModel.SubModelType.SUBMODEL_TRANSFORM:
            return true
        E3DSubModel.SubModelType.SUBMODEL_GL_TRIANGLES:
            return not target_node.exclude_node_names.any(
                func(node_name: Variant) -> bool:
                    return submodel.resource_name == str(node_name)
            )

    return false


func _get_material_override(target_node: E3DModelInstance, submodel: E3DSubModel) -> Material:
    var unprefixed_model_path: String = "/".join(target_node.data_path.split("/").slice(1))
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
        return MaterialManager.get_material(unprefixed_model_path, skin, dynamic_transparency)

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
            named_transparency,
            false,
            submodel.diffuse_color,
        )

    return null


func _requires_alpha_depth_prepass_sorting(material: Material) -> bool:
    if material is BaseMaterial3D:
        var base_material: BaseMaterial3D = material as BaseMaterial3D
        return base_material.transparency == BaseMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS
    return false
