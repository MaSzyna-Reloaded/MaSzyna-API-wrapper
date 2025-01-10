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

@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _reload()

@export var model_filename:String = "":
    set(x):
        if not x == model_filename:
            model_filename = x
            _reload()

@export var skins:Array = []:
    set(x):
        if not x == skins:
            skins = x
            _reload()

@export var exclude_node_names:Array = []:
    set(x):
        if not x == exclude_node_names:
            exclude_node_names = x
            _reload()

# Probably instancer should be set project-wide
@export var instancer = Instancer.NODES:
    set(x):
        if not x == instancer:
            instancer = x
            _reload()

var submodels_aabb:AABB = AABB()
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _reload()

func _get_aabb() -> AABB:
    return submodels_aabb

func _reload():
    E3DModelInstanceManager.reload_instance(self)

func _notification(what: int) -> void:
    match what:
        NOTIFICATION_ENTER_TREE:
            E3DModelInstanceManager.register_instance(self)
        NOTIFICATION_EXIT_TREE:
            E3DModelInstanceManager.unregister_instance(self)

func _ready():
    _reload()
