@tool
extends E3DInstancer


func instantiate(target_node: E3DModelInstance, model: E3DModel, editable: bool = false) -> void:
    var lights = []
    _do_add_submodels(target_node, target_node, model.submodels, editable, lights)
    _link_lights(target_node, lights)


func _link_lights(target_node: E3DModelInstance, lights: Array) -> void:
    var all_nodes = target_node.find_children("*", "Node3D", true, true)
    var node_map = {}
    for node in all_nodes:
        node_map[node.name.to_lower()] = node
    
    for light in lights:
        var base_name = light.name.to_lower()
        
        var on_suffixes = ["_on", "_xon"]
        for suffix in on_suffixes:
            var on_node = node_map.get(base_name + suffix)
            if on_node:
                light.meshes_on.append(on_node)
        
        var off_node = node_map.get(base_name + "_off")
        if off_node:
            light.meshes_off.append(off_node)
        
        light._update_state()


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
    editable:bool,
    lights: Array = []
) -> void:
    for submodel in submodels:
        if _is_submodel_valid(target_node, submodel):
            var child:Node = _create_submodel_instance(target_node, submodel)
            if not child:
                continue

            if child.has_method("_update_state"):
                lights.append(child)
            
            _update_submodel_material(target_node, child, submodel)
            var internal = InternalMode.INTERNAL_MODE_DISABLED if editable else InternalMode.INTERNAL_MODE_BACK
            parent.add_child(child, false, internal)

            # IMPORTANT: applying transform **after** adding to the tree
            # Applying transform before adding may cause issues (especially on windows)
            if child is Node3D and submodel.transform:
                var child_node:Node3D = child as Node3D
                if child is SpotLight3D:
                    # Do not scale SpotLight3D to avoid configuration warnings
                    child_node.position = submodel.transform.origin
                    child_node.basis = submodel.transform.basis.orthonormalized()
                else:
                    child_node.transform = submodel.transform

            if Engine.is_editor_hint():
                child.owner = target_node.owner if editable else target_node
            if submodel.submodels:
                _do_add_submodels(target_node, child, submodel.submodels, editable, lights)


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

        E3DSubModel.SubModelType.SUBMODEL_FREE_SPOTLIGHT:
            obj = SpotLight3D.new()
            obj.set_script(preload("res://addons/libmaszyna/e3d/e3d_light.gd"))
            obj.name = submodel.resource_name
            obj.light_color = submodel.diffuse_color
            obj.light_energy = 10.0
            obj.shadow_enabled = true
            obj.distance_fade_enabled = true
            obj.spot_range = submodel.light_range if submodel.light_range > 0.0 else 20.0
            obj.spot_angle = submodel.light_angle
            obj.spot_attenuation = submodel.light_attenuation
            obj.distance_fade_begin = submodel.near_atten_start
            obj.enabled = submodel.visible

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
