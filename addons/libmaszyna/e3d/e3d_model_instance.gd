@tool
extends VisualInstance3D
class_name E3DModelInstance

signal e3d_loaded

enum Instancer {
    OPTIMIZED,
    NODES,
    EDITABLE_NODES,
}


class OptimizedInstanceRecord:
    var instance_rid: RID = RID()
    var local_transform: Transform3D = Transform3D.IDENTITY
    var visible: bool = true
    var name: String = ""


var default_aabb_size: Vector3 = Vector3(1, 1, 1)

@export var data_path: String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _reload()

@export var model_filename: String = "":
    set(x):
        if not x == model_filename:
            model_filename = x
            _reload()

@export var skins: Array = []:
    set(x):
        if not x == skins:
            skins = x
            _reload()

@export var exclude_node_names: Array = []:
    set(x):
        if not x == exclude_node_names:
            exclude_node_names = x
            _reload()

# Probably instancer should be set project-wide
@export var instancer: Instancer = Instancer.NODES:
    set(x):
        if not x == instancer:
            instancer = x
            _reload()

var submodels_aabb: AABB = AABB()
var editable_in_editor: bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _reload()

var _loaded_model: E3DModel = null
var _submodel_material_overrides: Dictionary = {}
var _optimized_instances: Array = []


func _init() -> void:
    set_notify_transform(true)
    set_notify_local_transform(true)


func _get_aabb() -> AABB:
    if submodels_aabb.has_surface():
        return submodels_aabb
    if _loaded_model:
        return _loaded_model.get_aabb()
    return AABB(-default_aabb_size / 2.0, default_aabb_size)


func _reload() -> void:
    E3DModelInstanceManager.reload_instance(self)


func _notification(what: int) -> void:
    match what:
        NOTIFICATION_ENTER_TREE:
            E3DModelInstanceManager.register_instance(self)
        NOTIFICATION_ENTER_WORLD, NOTIFICATION_VISIBILITY_CHANGED, NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
            if instancer == Instancer.OPTIMIZED:
                _sync_optimized_instances()
        NOTIFICATION_EXIT_TREE:
            E3DModelInstanceManager.unregister_instance(self)
            _clear_optimized_instances()


func _ready() -> void:
    _reload()


func _apply_loaded_model(model: E3DModel) -> void:
    _loaded_model = model
    submodels_aabb = model.get_aabb() if model else AABB()
    _clear_optimized_instances()

    if instancer == Instancer.OPTIMIZED:
        E3DNodesInstancer.clear_children(self)
        if model:
            _collect_optimized_submodels(model.submodels, Transform3D.IDENTITY, true)
            _sync_optimized_instances()
    else:
        if model:
            E3DNodesInstancer.instantiate(
                model,
                self,
                instancer == Instancer.EDITABLE_NODES
                or (instancer == Instancer.NODES and editable_in_editor)
            )
        else:
            E3DNodesInstancer.clear_children(self)

    e3d_loaded.emit()


func _collect_optimized_submodels(
    submodels: Array, parent_transform: Transform3D, parent_visible: bool
) -> void:
    for submodel: E3DSubModel in submodels:
        if submodel == null or submodel.skip_rendering:
            continue

        var current_transform: Transform3D = parent_transform * submodel.transform
        var current_visible: bool = parent_visible and submodel.visible

        if not submodel.submodels.is_empty():
            _collect_optimized_submodels(submodel.submodels, current_transform, current_visible)

        if submodel.submodel_type != E3DSubModel.SubModelType.GL_TRIANGLES:
            continue
        if exclude_node_names.has(submodel.name):
            continue

        var mesh: Mesh = submodel.mesh
        if mesh == null:
            continue

        var instance_rid: RID = RenderingServer.instance_create2(mesh.get_rid(), RID())
        RenderingServer.instance_geometry_set_visibility_range(
            instance_rid,
            submodel.visibility_range_begin,
            submodel.visibility_range_end,
            0.0,
            0.0,
            RenderingServer.VISIBILITY_RANGE_FADE_DISABLED
        )

        var material: Material = E3DNodesInstancer.resolve_submodel_material(self, submodel)
        if material != null:
            RenderingServer.instance_geometry_set_material_override(
                instance_rid,
                material.get_rid()
            )

        var record: OptimizedInstanceRecord = OptimizedInstanceRecord.new()
        record.instance_rid = instance_rid
        record.local_transform = current_transform
        record.visible = current_visible
        record.name = submodel.name
        _optimized_instances.append(record)


func _clear_optimized_instances() -> void:
    for record: OptimizedInstanceRecord in _optimized_instances:
        if record.instance_rid.is_valid():
            RenderingServer.free_rid(record.instance_rid)
    _optimized_instances.clear()


func _sync_optimized_instances() -> void:
    if not is_inside_tree() or _optimized_instances.is_empty():
        return

    var current_world: World3D = get_world_3d()
    if current_world == null:
        return

    var scenario: RID = current_world.scenario
    var tree_visible: bool = is_visible_in_tree()
    var global_transform_3d: Transform3D = global_transform
    var layer_mask: int = get_layer_mask()

    for record: OptimizedInstanceRecord in _optimized_instances:
        RenderingServer.instance_set_scenario(record.instance_rid, scenario)
        RenderingServer.instance_set_transform(
            record.instance_rid,
            global_transform_3d * record.local_transform
        )
        RenderingServer.instance_set_visible(record.instance_rid, tree_visible and record.visible)
        RenderingServer.instance_set_layer_mask(record.instance_rid, layer_mask)

        var override_material: Material = get_submodel_material_override(record.name)
        if override_material != null:
            RenderingServer.instance_geometry_set_material_override(
                record.instance_rid,
                override_material.get_rid()
            )


func set_submodel_material_override(submodel_name: String, material: Material) -> void:
    if submodel_name.is_empty():
        return

    _submodel_material_overrides[submodel_name] = material
    if instancer == Instancer.OPTIMIZED:
        _sync_optimized_instances()
    else:
        _reload()


func get_submodel_material_override(submodel_name: String) -> Material:
    return _submodel_material_overrides.get(submodel_name) as Material


func clear_submodel_material_override(submodel_name: String) -> void:
    _submodel_material_overrides.erase(submodel_name)
    if instancer == Instancer.OPTIMIZED:
        _sync_optimized_instances()
    else:
        _reload()
