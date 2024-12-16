@tool
extends Node

var _colored_material = preload("res://addons/libmaszyna/e3d/colored.material")

func instantiate(model:E3DModel, target_node:E3DModelInstance):
    _do_add_submodels(target_node, target_node, model.submodels)


func _do_add_submodels(target_node:E3DModelInstance, parent, submodels):
    for submodel in submodels:
        var child = _create_submodel_instance(target_node, submodel)
        if child:
            _update_submodel_material(target_node, child, submodel)
            parent.add_child(child)
            if submodel.submodels:
                _do_add_submodels(target_node, child, submodel.submodels)


func _create_submodel_instance(target_node: E3DModelInstance, submodel: E3DSubModel):
    var obj

    match submodel.submodel_type:
        E3DSubModel.SubModelType.TRANSFORM:
            obj = Node3D.new()
            obj.name = submodel.name
            obj.transform = submodel.transform
        E3DSubModel.SubModelType.GL_TRIANGLES:
            var is_name_excluded = target_node.exclude_node_names.any(
                func(name): return submodel.name == name
            )

            if not is_name_excluded:
                obj = MeshInstance3D.new()
                obj.name = submodel.name
                obj.mesh = submodel.mesh
                obj.transform = submodel.transform
                obj.visibility_range_begin = submodel.visibility_range_begin
                obj.visibility_range_end = submodel.visibility_range_end

    return obj

func _update_submodel_material(target_node:E3DModelInstance, subnode:Node3D, submodel:E3DSubModel):
    var unprefixed_model_path = "/".join(target_node.data_path.split("/").slice(1))
    if submodel.dynamic_material:
        if target_node.skins.size() < submodel.dynamic_material_index + 1:
            push_warning("Model %s has no skins set, but submodel requires material #%s" % [target_node.name, submodel.dynamic_material_index])
        else:
            var transparency = (
                MaterialManager.Transparency.AlphaScissor
                if submodel.material_transparent
                else MaterialManager.Transparency.Disabled
            )
            var skin = target_node.skins[submodel.dynamic_material_index]
            var material = MaterialManager.get_material(unprefixed_model_path, skin, transparency)
            subnode.material_override = material
    else:
        if submodel.material_colored:
            subnode.material_override = _colored_material
            subnode.set_instance_shader_parameter("albedo_color", submodel.diffuse_color)
        elif submodel.material_name:
            subnode.material_override = MaterialManager.get_material(
                unprefixed_model_path,
                submodel.material_name,
                (
                    MaterialManager.Transparency.Alpha
                    if submodel.material_transparent
                    else MaterialManager.Transparency.Disabled
                ),
                false, # model.is_sky,  # unshaded if sky
                submodel.diffuse_color,
            )
