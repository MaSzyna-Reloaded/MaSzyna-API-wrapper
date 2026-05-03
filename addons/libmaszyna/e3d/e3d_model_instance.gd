@tool
extends VisualInstance3D
class_name E3DModelInstance

signal e3d_loaded

enum Instancer {
    OPTIMIZED,
    NODES,
    EDITABLE_NODES,
}

var _dirty: bool = true
var default_aabb_size: Vector3 = Vector3(1, 1, 1)

@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _dirty = true

@export var model_filename:String = "":
    set(x):
        if not x == model_filename:
            model_filename = x
            _dirty = true

@export var skins:Array = []:
    set(x):
        if not x == skins:
            skins = x
            _dirty = true

@export var exclude_node_names:Array = []:
    set(x):
        if not x == exclude_node_names:
            exclude_node_names = x
            _dirty = true

# Probably instancer should be set project-wide
@export var instancer = Instancer.NODES:
    set(x):
        if not x == instancer:
            instancer = x
            _dirty = true

var submodels_aabb:AABB = AABB()
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _dirty = true


func _get_aabb() -> AABB:
    return submodels_aabb


func _process(_delta: float) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(_delta)
            _dirty = false


func _process_dirty(_delta: float) -> void:
    reload()


func reload():
    if is_inside_tree() and model_filename and data_path:
        if data_path and model_filename:
            var _do_load = func():
                var model = E3DModelManager.load_model(data_path, model_filename)
                if model:
                    match instancer:
                        E3DModelInstance.Instancer.NODES:
                            var _do_instantiate = func():
                                E3DNodesInstancer.instantiate(model, self, editable_in_editor)
                                e3d_loaded.emit()
                            SceneryResourceLoader.schedule(
                                "Creating model %s" % name, _do_instantiate
                            )
                        _:
                            push_error("Selected instancer is not supported!")
            SceneryResourceLoader.schedule("Loading %s" % name, _do_load)

func _ready():
    reload()
