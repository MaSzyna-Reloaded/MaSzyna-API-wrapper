@tool
extends Node3D
class_name E3DModelInstance

enum Instancer {
    OPTIMIZED,
    NODES,
    EDITABLE_NODES,
}

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

func _reload():
    if is_inside_tree() and model_filename and data_path:

        for child in get_children():
            remove_child(child)
            child.queue_free()

        if data_path and model_filename:
            var model = E3DModelManager.load_model(data_path, model_filename)
            if model:
                match instancer:
                    Instancer.NODES:
                        E3DNodesInstancer.instantiate(model, self)
                    _:
                        push_error("Selected instancer is not supported!")


func _ready():
    _reload()
    UserSettings.game_dir_changed.connect(_reload)
    UserSettings.cache_cleared.connect(_reload)
    UserSettings.models_reload_requested.connect(_reload)
