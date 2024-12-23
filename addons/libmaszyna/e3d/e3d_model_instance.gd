@tool
extends VisualInstance3D
class_name E3DModelInstance

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

@export var head_display_material:Material:
    set(x):
        if not x == head_display_material:
            head_display_material = x
            _update_head_display()

@export_node_path("MeshInstance3D") var head_display_node_path = NodePath(""):
    set(x):
        if not x == head_display_node_path:
            head_display_node_path = x
            _update_head_display()

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


func _update_head_display():
    if head_display_node_path:
        var node:MeshInstance3D = get_node(head_display_node_path)
        if node:
            node.material_override = head_display_material


func _reload():
    if is_inside_tree() and model_filename and data_path:
        if data_path and model_filename:
            var model = E3DModelManager.load_model(data_path, model_filename)
            if model:
                match instancer:
                    Instancer.NODES:
                        E3DNodesInstancer.instantiate(model, self, editable_in_editor)
                        _update_head_display()
                    _:
                        push_error("Selected instancer is not supported!")


func _ready():
    _reload()
    UserSettings.game_dir_changed.connect(_reload)
    UserSettings.cache_cleared.connect(_reload)
    UserSettings.models_reload_requested.connect(_reload)
