@tool
extends E3DInstancer

# InstancerEntry must hold references to materials and RIDs
# to avoid automatic Ref-cleaning.

class InstancerEntry:
    var materials: Array[Material] = []  # overridden materials 
    var rids: Array[RID] = []  # instance RIDs
    var local_transforms: Dictionary[RID, Transform3D] = {}  # local transforms

var _instances: Dictionary[E3DModelInstance, InstancerEntry] = {}


func instantiate(target_node: E3DModelInstance, model: E3DModel, editable: bool = false) -> void:
    var entry := InstancerEntry.new()
    _instances[target_node] = entry
    var scenario:RID = target_node.get_world_3d().scenario
    
    _add_submodels(
        entry,
        target_node,
        model.submodels,
        Transform3D.IDENTITY,
        scenario
    )
    
    sync(target_node)


func clear(target_node: E3DModelInstance) -> void:
    var entry := _instances.get(target_node) as InstancerEntry
    if entry:
        for rid in entry.rids:
            RenderingServer.free_rid(rid)
        entry.rids.clear()
        entry.local_transforms.clear()
        entry.materials.clear()
        _instances.erase(target_node)
    

func sync(target_node: E3DModelInstance) -> void:
    var entry := _instances.get(target_node) as InstancerEntry
    if entry:
        for rid in entry.rids:
            var local_transform = entry.local_transforms[rid]
            RenderingServer.instance_set_transform(
                rid, 
                target_node.global_transform * local_transform
            )


func _add_submodels(
    entry: InstancerEntry,
    target_node: E3DModelInstance,
    submodels: Array[E3DSubModel],
    parent_transform: Transform3D,
    scenario: RID,
) -> void:
    for submodel: E3DSubModel in submodels:
        if not _is_submodel_valid(target_node, submodel):
            continue

        if not submodel.submodel_type == E3DSubModel.SUBMODEL_TRANSFORM:
            if not submodel.mesh:
                continue

        var local_transform: Transform3D = (
            parent_transform * submodel.transform
            if submodel.transform
            else parent_transform
        )
        
        match submodel.submodel_type:
            E3DSubModel.SubModelType.SUBMODEL_GL_TRIANGLES:
                _add_submodel(entry, target_node, submodel, local_transform, scenario)
        if submodel.submodels:
            _add_submodels(entry, target_node, submodel.submodels, local_transform, scenario)


func _add_submodel(
    entry: InstancerEntry,
    target_node: E3DModelInstance,
    submodel: E3DSubModel,
    local_transform: Transform3D,
    scenario: RID,
) -> void:
    var instance: RID = RenderingServer.instance_create()
    RenderingServer.instance_attach_object_instance_id(instance, target_node.get_instance_id())
    RenderingServer.instance_set_base(instance, submodel.mesh.get_rid())
    RenderingServer.instance_set_scenario(instance, scenario)
    RenderingServer.instance_geometry_set_visibility_range(
        instance,
        submodel.visibility_range_begin,
        submodel.visibility_range_end,
        0.0,
        0.0,
        RenderingServer.VISIBILITY_RANGE_FADE_DISABLED
    )

    var material: Material = _get_material_override(target_node, submodel)
    if material:
        entry.materials.append(material)
        RenderingServer.instance_geometry_set_material_override(instance, material.get_rid())
        if _requires_alpha_depth_prepass_sorting(material):
            RenderingServer.instance_set_pivot_data(instance, -1, false)
    
    RenderingServer.instance_set_visible(instance, submodel.visible)
    entry.rids.append(instance)
    entry.local_transforms[instance] = local_transform
