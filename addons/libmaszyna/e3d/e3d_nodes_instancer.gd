@tool
extends E3DInstancer


func instantiate(target_node: E3DModelInstance, model: E3DModel, editable: bool = false) -> void:
    _do_add_submodels(target_node, target_node, model.submodels, editable)


func clear(target_node: E3DModelInstance) -> void:
    for child: Node in target_node.get_children(true):
        target_node.remove_child(child)
        child.queue_free()


func sync(target_node: E3DModelInstance) -> void:
    pass


func _do_add_submodels(
    target_node:E3DModelInstance,
    parent,
    submodels,
    editable:bool
) -> void:
    for submodel in submodels:
        if _is_submodel_valid(target_node, submodel):
            var child:Node = _create_submodel_instance(target_node, submodel)
            _update_submodel_material(target_node, child, submodel)
            var internal = InternalMode.INTERNAL_MODE_DISABLED if editable else InternalMode.INTERNAL_MODE_BACK
            parent.add_child(child, false, internal)

            # IMPORTANT: applying transform **after** adding to the tree
            # Applying transform before adding may cause issues (especially on windows)
            if child is Node3D and submodel.transform:
                var child_node:Node3D = child as Node3D
                child_node.transform = submodel.transform

            if Engine.is_editor_hint():
                child.owner = target_node.owner if editable else target_node
            if submodel.submodels:
                _do_add_submodels(target_node, child, submodel.submodels, editable)


func _create_submodel_instance(target_node: E3DModelInstance, submodel: E3DSubModel):
    var obj
    
    match submodel.submodel_type:
        E3DSubModel.SubModelType.SUBMODEL_TRANSFORM:
            obj = Node3D.new()
            obj.name = submodel.resource_name

        E3DSubModel.SubModelType.SUBMODEL_GL_TRIANGLES:
            obj = MeshInstance3D.new()
            obj.name = submodel.resource_name
            obj.mesh = submodel.mesh
            obj.visibility_range_begin = submodel.visibility_range_begin
            obj.visibility_range_end = submodel.visibility_range_end

    if obj:
        obj.visible = submodel.visible
    return obj


func _update_submodel_material(
    target_node: E3DModelInstance,
    subnode: Node,
    submodel: E3DSubModel
) -> void:
    if not subnode is GeometryInstance3D:
        return

    var geometry_instance:GeometryInstance3D = subnode as GeometryInstance3D
    var material:Material = _get_material_override(target_node, submodel)
    if material:
        geometry_instance.material_override = material
    if submodel.material_colored:
        geometry_instance.set_instance_shader_parameter("albedo_color", submodel.diffuse_color)

    if subnode is MeshInstance3D and _requires_alpha_depth_prepass_sorting(material):
        var mesh_instance: MeshInstance3D = subnode as MeshInstance3D
        mesh_instance.sorting_offset = -1
