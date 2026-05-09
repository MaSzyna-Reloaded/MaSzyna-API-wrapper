@tool
extends E3DInstancer

class LightNodeInfo:
    var light_name: String = ""
    var light_on_node_path:NodePath = NodePath("")
    var light_off_node_path:NodePath = NodePath("")
    var spotlight_node_path:NodePath = NodePath("")
    var submodel: E3DSubModel

var instances_lights : Dictionary = {}

func instantiate(target_node: E3DModelInstance, model: E3DModel, editable: bool = false) -> void:
    var lights: Dictionary = {}
    var entries: Array = []  # first pass

    for light_name: String in model.lights.keys():
        var light_info: E3DModelLightDefinition = model.lights[light_name]
        if not light_info:
            continue

        lights[light_name] = {
            "on": model.get_node_or_null(light_info.on_submodel_path),
            "off": model.get_node_or_null(light_info.off_submodel_path),
        }

    _do_add_submodels(target_node, model, target_node, model.submodels, editable, entries, lights)

    # prebuild LightNodeInfo dictionary
    var _instance_lights: Dictionary = {}
    var _spotlights: Array[SpotLight3D] = []

    for entry in entries:
        var light_name_value: Variant = entry[0]
        var light_name: String = "" if light_name_value == null else str(light_name_value)
        var entry_type: String = entry[1]
        var node: Node = entry[2]
        var submodel:E3DSubModel = entry[3]
        if light_name:
            var resolved_light_info: LightNodeInfo = _instance_lights.get(light_name)
            if not resolved_light_info:
                resolved_light_info = LightNodeInfo.new()
                resolved_light_info.light_name = light_name
                resolved_light_info.submodel = submodel
                _instance_lights[light_name] = resolved_light_info
            match entry_type:
                "on":
                    resolved_light_info.light_on_node_path = target_node.get_path_to(node)
                "off":
                    resolved_light_info.light_off_node_path = target_node.get_path_to(node)
        else:
            if entry_type == "spotlight":
                _spotlights.append(node)

    var _light_on_nodes: Dictionary = {}
    for light_name: String in _instance_lights.keys():
        var resolved_light_info: LightNodeInfo = _instance_lights[light_name]
        if not resolved_light_info.light_on_node_path.is_empty():
            _light_on_nodes[resolved_light_info.light_on_node_path] = resolved_light_info

    # assign instance lights to the global registry
    instances_lights[target_node] = _instance_lights

    for spotlight: SpotLight3D in _spotlights:
        var parent: Node = spotlight.get_parent()
        while parent:
            var parent_path: NodePath = target_node.get_path_to(parent)
            var resolved_light_info: LightNodeInfo = _light_on_nodes.get(parent_path)
            if resolved_light_info:
                resolved_light_info.spotlight_node_path = target_node.get_path_to(spotlight)
                configure_spotlight(spotlight, resolved_light_info.light_name, resolved_light_info.submodel)
                break
            parent = parent.get_parent()

    sync_lights(target_node)


func sync_lights(target_node: E3DModelInstance) -> void:
    var state: Dictionary[String, bool] = target_node.lights_state
    var instance_lights: Dictionary = instances_lights.get(target_node, {})
    for light_name: String in state.keys():
        var light_info: LightNodeInfo = instance_lights.get(light_name)
        if light_info:
            var enabled: bool = state[light_name]
            var on_node: Node3D = target_node.get_node_or_null(light_info.light_on_node_path)
            var off_node: Node3D = target_node.get_node_or_null(light_info.light_off_node_path)
            var spotlight: SpotLight3D = target_node.get_node_or_null(light_info.spotlight_node_path)

            if on_node:
                on_node.visible = enabled
            elif not on_node and light_info.light_on_node_path:
                push_error("[%s] Light on node not found: %s" % [target_node.name, light_info.light_on_node_path])
            if off_node:
                off_node.visible = not enabled
            elif not off_node and light_info.light_off_node_path:
                push_error("[%s] Light off node not found: %s" % [target_node.name, light_info.light_off_node_path])
            if spotlight:
                spotlight.visible = enabled
            elif not spotlight and light_info.spotlight_node_path:
                push_error("[%s] Spotlight node not found: %s" % [target_node.name, light_info.spotlight_node_path])
        else:
            push_warning("[%s] LightInfo not found for light: %s" % [target_node.name, light_name])


func clear(target_node: E3DModelInstance) -> void:
    instances_lights.erase(target_node)
    for child: Node in target_node.get_children(true):
        target_node.remove_child(child)
        child.queue_free()


func sync(target_node: E3DModelInstance) -> void:
    pass


func _do_add_submodels(
    target_node:E3DModelInstance,
    model: E3DModel,
    parent,
    submodels: Array[E3DSubModel],
    editable: bool,
    entries: Array,
    lights: Dictionary
) -> void:
    for submodel: E3DSubModel in submodels:
        if _is_submodel_valid(target_node, submodel):
            var child: Node = _create_submodel_instance(target_node, submodel, model, entries)
            if not child:
                continue
            for light_name: String in lights.keys():
                var resolved_info: Dictionary = lights[light_name]
                var on_submodel: E3DSubModel = resolved_info.get("on")
                var off_submodel: E3DSubModel = resolved_info.get("off")
                if submodel == on_submodel:
                    entries.append([light_name, "on", child, submodel])
                elif submodel == off_submodel:
                    entries.append([light_name, "off", child, submodel])

            _update_submodel_material(target_node, child, submodel)
            var internal = InternalMode.INTERNAL_MODE_DISABLED if editable else InternalMode.INTERNAL_MODE_BACK
            parent.add_child(child, false, internal)

            # IMPORTANT: applying transform **after** adding to the tree
            # Applying transform before adding may cause issues (especially on windows)
            if child is Node3D and submodel.transform:
                var child_node:Node3D = child as Node3D
                if submodel.submodel_type == E3DSubModel.SUBMODEL_FREE_SPOTLIGHT:
                    # Do not scale SpotLight3D to avoid configuration warnings
                    child_node.position = submodel.transform.origin
                    child_node.basis = submodel.transform.basis.orthonormalized()
                else:
                    child_node.transform = submodel.transform

            if Engine.is_editor_hint():
                child.owner = target_node.owner if editable else target_node
            if submodel.submodels:
                _do_add_submodels(target_node, model, child, submodel.submodels, editable, entries, lights)


func _create_submodel_instance(target_node: E3DModelInstance, submodel: E3DSubModel, model: E3DModel, entries: Array):
    var obj: Node

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
            obj.name = submodel.resource_name
            obj.light_color = submodel.diffuse_color
            obj.light_color.a = 1.0
            obj.light_volumetric_fog_energy = 4.0  # FIXME: guessing
            obj.shadow_enabled = true
            obj.distance_fade_enabled = true
            obj.distance_fade_begin = 150.0
            obj.distance_fade_shadow = 100.0
            obj.distance_fade_length = 200.0
            obj.spot_range = submodel.light_range
            obj.spot_angle = submodel.light_angle

            entries.append([null, "spotlight", obj, submodel])

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
