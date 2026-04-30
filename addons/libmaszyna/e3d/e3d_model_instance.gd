@tool
extends VisualInstance3D
class_name E3DModelInstance

signal e3d_loaded

enum Instancer {
    OPTIMIZED,
    NODES,
    EDITABLE_NODES,
}

var default_aabb_size: Vector3 = Vector3(1, 1, 1)
var e3d_model:E3DModel

var _dirty := true

@export var data_path: String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _dirty = true

@export var model_filename: String = "":
    set(x):
        if not x == model_filename:
            model_filename = x
            _dirty = true

@export var skins: Array = []:
    set(x):
        if not x == skins:
            skins = x
            _dirty = true

@export var exclude_node_names: Array = []:
    set(x):
        if not x == exclude_node_names:
            exclude_node_names = x
            _dirty = true

# Probably instancer should be set project-wide
@export var instancer: Instancer = Instancer.NODES:
    set(x):
        if not x == instancer:
            instancer = x
            _dirty = true

var submodels_aabb: AABB = AABB()
var editable_in_editor: bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _dirty = true

var _submodel_material_overrides: Dictionary = {}


func _init() -> void:
    set_notify_transform(true)
    set_notify_local_transform(true)


func reload() -> void:
    E3DModelInstanceManager.reload_instance(self)


func _notification(what: int) -> void:
    match what:
        NOTIFICATION_ENTER_TREE:
            E3DModelInstanceManager.register_instance(self)
            E3DModelInstanceManager.reload_instance(self)
        NOTIFICATION_EXIT_TREE:
            E3DModelInstanceManager.unregister_instance(self)
            E3DInstanceFactory.teardown(self)
        NOTIFICATION_PREDELETE:
            E3DInstanceFactory.teardown(self)


func _ready() -> void:
    reload()


func _process(_delta:float) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(_delta)
            _dirty = false


func _process_dirty(_delta:float) -> void:
    reload()


func set_submodel_material_override(submodel_name: String, material: Material) -> void:
    if submodel_name.is_empty():
        return

    _submodel_material_overrides[submodel_name] = material
    #_reload()


func get_submodel_material_override(submodel_name: String) -> Material:
    return _submodel_material_overrides.get(submodel_name) as Material


func clear_submodel_material_override(submodel_name: String) -> void:
    _submodel_material_overrides.erase(submodel_name)
    #_reload()
