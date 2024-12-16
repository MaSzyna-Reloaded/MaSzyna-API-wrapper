@tool
extends Node

var _colored_material = preload("res://addons/libmaszyna/e3d/colored.material")

func instantiate(model:E3DModel, target_node:E3DModelInstance, editable:bool = false):
    for child in target_node.get_children(true):
        target_node.remove_child(child)
        child.queue_free()
    _do_add_submodels(target_node, target_node, model.submodels, editable)

# Helper function to traverse and merge AABBs of VisualInstance3D descendants
func _traverse_and_extend(node: Node, combined_aabb: AABB, has_initialized_aabb: bool):
    for child in node.get_children():
        if child is VisualInstance3D:
            # Get the child's AABB and transform it to world space
            var child_aabb:AABB = child.get_aabb()
            #var transformed_aabb = child_aabb.transformed(child.global_transform)
            var transformed_aabb:AABB = child_aabb

            # Merge into the combined AABB
            if not has_initialized_aabb:
                combined_aabb.position = transformed_aabb.position
                combined_aabb.size = transformed_aabb.size
            else:
                combined_aabb = combined_aabb.merge(transformed_aabb)

            return _traverse_and_extend(child, combined_aabb, true)
    return combined_aabb

func _do_add_submodels(target_node:E3DModelInstance, parent, submodels, editable:bool):
    for submodel in submodels:
        var child:Node = _create_submodel_instance(target_node, submodel)
        if child:
            _update_submodel_material(target_node, child, submodel)
            var internal = InternalMode.INTERNAL_MODE_DISABLED if editable else InternalMode.INTERNAL_MODE_BACK
            parent.add_child(child, false, internal)

            # IMPORTANT: applying transform **after** adding to the tree
            # Applying transform before adding may cause issues (especially on windows)
            if submodel.transform:
                child.transform = submodel.transform

            if Engine.is_editor_hint():
                child.owner = target_node.owner if editable else target_node
            if submodel.submodels:
                _do_add_submodels(target_node, child, submodel.submodels, editable)

func _create_submodel_instance(target_node: E3DModelInstance, submodel: E3DSubModel):
    var obj

    if submodel.skip_rendering:
        return

    match submodel.submodel_type:
        E3DSubModel.SubModelType.TRANSFORM:
            obj = Node3D.new()
            obj.name = submodel.name

        E3DSubModel.SubModelType.GL_TRIANGLES:
            var is_name_excluded = target_node.exclude_node_names.any(
                    func(name): return submodel.name == name
                )

            if not is_name_excluded:
                obj = MeshInstance3D.new()
                obj.name = submodel.name
                obj.mesh = submodel.mesh
                obj.visibility_range_begin = submodel.visibility_range_begin
                obj.visibility_range_end = submodel.visibility_range_end

    if obj:
        obj.visible = submodel.visible
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
            var transparency = (
                MaterialManager.Transparency.AlphaScissor
                if submodel.material_transparent
                else MaterialManager.Transparency.Disabled
            )
            subnode.material_override = MaterialManager.get_material(
                unprefixed_model_path,
                submodel.material_name,
                transparency,
                false, # model.is_sky,  # unshaded if sky
                submodel.diffuse_color,
            )
            # FIXME: a workaround to tweak sorting for ALPHA materials
            if subnode is MeshInstance3D:
                var mi:MeshInstance3D = subnode as MeshInstance3D
                var _m:BaseMaterial3D = subnode.material_override
                if _m and _m.transparency == BaseMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS:
                    mi.sorting_offset = -1  # FIXME: guessing
