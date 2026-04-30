@tool
extends Node

var _colored_material: Material = preload("res://addons/libmaszyna/e3d/colored.material")

func clear_children(target_node: E3DModelInstance) -> void:
    for child in target_node.get_children(true):
        target_node.remove_child(child)
        child.queue_free()


func instantiate(model, target_node: E3DModelInstance, editable: bool = false) -> void:
    clear_children(target_node)
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

func _do_add_submodels(
    target_node: E3DModelInstance, parent: Node, submodels: Array, editable: bool
) -> void:
    for submodel in submodels:
        var child: Node = _create_submodel_instance(target_node, submodel)
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

func _create_submodel_instance(target_node: E3DModelInstance, submodel) -> Node3D:
    var obj: Node3D

    if submodel.skip_rendering:
        return null

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

func _update_submodel_material(
    target_node: E3DModelInstance, subnode: Node3D, submodel
) -> void:
    if not subnode is MeshInstance3D:
        return

    var mesh_instance: MeshInstance3D = subnode as MeshInstance3D
    var material: Material = resolve_submodel_material(target_node, submodel)
    mesh_instance.material_override = material

    var base_material: BaseMaterial3D = material as BaseMaterial3D
    if base_material and base_material.transparency == BaseMaterial3D.TRANSPARENCY_ALPHA_DEPTH_PRE_PASS:
        mesh_instance.sorting_offset = -1


func resolve_submodel_material(target_node: E3DModelInstance, submodel) -> Material:
    var override_material: Material = target_node.get_submodel_material_override(submodel.name)
    if override_material:
        return override_material

    var unprefixed_model_path: String = "/".join(target_node.data_path.split("/").slice(1))
    if submodel.dynamic_material:
        if target_node.skins.size() < submodel.dynamic_material_index + 1:
            push_warning(
                "Model %s has no skins set, but submodel requires material #%s"
                % [target_node.name, submodel.dynamic_material_index]
            )
        else:
            var transparency: int = (
                MaterialManager.Transparency.AlphaScissor
                if submodel.material_transparent
                else MaterialManager.Transparency.Disabled
            )

            var skin: String = str(target_node.skins[submodel.dynamic_material_index])
            return MaterialManager.get_material(unprefixed_model_path, skin, transparency)
    else:
        if submodel.material_colored:
            var colored_material: Material = _colored_material.duplicate()
            var shader_material: ShaderMaterial = colored_material as ShaderMaterial
            if shader_material:
                shader_material.set_shader_parameter("albedo_color", submodel.diffuse_color)
                return shader_material

            var fallback_material: StandardMaterial3D = StandardMaterial3D.new()
            fallback_material.albedo_color = submodel.diffuse_color
            return fallback_material
        if submodel.material_name:
            var transparency: int = (
                MaterialManager.Transparency.AlphaScissor
                if submodel.material_transparent
                else MaterialManager.Transparency.Disabled
            )
            return MaterialManager.get_material(
                unprefixed_model_path,
                submodel.material_name,
                transparency,
                false,
                submodel.diffuse_color,
            )
    return null
