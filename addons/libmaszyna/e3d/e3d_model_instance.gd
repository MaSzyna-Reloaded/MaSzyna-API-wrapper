@tool
extends VisualInstance3D

## Displays a MaSzyna E3D model in 3D space.
##
## [E3DModelInstance] loads an [E3DModel] and instantiates it using one of the
## supported instancers. [code]NODES[/code] and [code]EDITABLE_NODES[/code]
## build a regular node hierarchy. [code]OPTIMIZED[/code] renders through
## [code]RenderingServer[/code] and does not create child mesh nodes.
## If [member model] is set, it will be used to instnatiate. Otherwise the
## [member data_path] and [member model_filename] will be used to load with [E3DModelManager].
class_name E3DModelInstance

## Emitted after the current model instance has been created.
signal e3d_loading
signal e3d_loaded

## Selected instancing backend.
enum Instancer {
    OPTIMIZED,  ## Renders using [code]RenderingServer[/code] without creating child mesh nodes.
    NODES,  ## Creates a regular hierarchy of generated 3D nodes.
    EDITABLE_NODES,  ## Creates generated 3D nodes intended to stay editable in the editor.
}

var _model: E3DModel
var _dirty: bool = false
var _e3d_loaded: bool = false
var _current_instancer: E3DInstancer = null
var _current_editable: bool = false

var default_aabb_size: Vector3 = Vector3(1, 1, 1)


## E3DModel to instantiate (leave empty, if you want to lazy load with [member model_filename])
@export var model: E3DModel:
    set(x):
        if not x == model:
            model = x
            _dirty = true

## Base MaSzyna data path used to resolve model files and materials.
@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _dirty = true

## Name of the [code].e3d[/code] model file to load (if [member model] is not used).
@export var model_filename:String = "":
    set(x):
        if not x == model_filename:
            model_filename = x
            _dirty = true

## Optional material skin overrides used by dynamic submodel materials.
@export var skins:Array = []:
    set(x):
        if not x == skins:
            skins = x
            _dirty = true

## Submodel names that should be skipped during instantiation.
@export var exclude_node_names:Array = []:
    set(x):
        if not x == exclude_node_names:
            exclude_node_names = x
            _dirty = true

## Selected instancing backend. The default is [code]NODES[/code].
@export var instancer = Instancer.NODES:
    set(x):
        if not x == instancer:
            instancer = x
            _dirty = true

var submodels_aabb:AABB = AABB()
## When enabled, node-based instancing keeps generated children editable in the editor.
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _dirty = true


func _get_aabb() -> AABB:
    return submodels_aabb


func _resolve_instancer() -> E3DInstancer:
    match instancer:
        Instancer.NODES, Instancer.EDITABLE_NODES:
            return E3DNodesInstancer
        Instancer.OPTIMIZED:
            return E3DOptimizedInstancer
    return null


func _process(_delta: float) -> void:
    if Engine.is_editor_hint():
        if _dirty:
            _process_dirty(_delta)
            _dirty = false


func _process_dirty(_delta: float) -> void:
    reload()


## Reloads the configured E3D model and recreates the current instance using the selected instancer.
func reload() -> void:
    if is_inside_tree() and (model or model_filename):
        _e3d_loaded = false
        e3d_loading.emit()
        if _current_instancer:
            _current_instancer.clear(self)
        _current_instancer = _resolve_instancer()
        if model:
            _model = model
        else:
            _model = E3DModelManager.load_model(data_path, model_filename)
        if _model:
            submodels_aabb = E3DModelTool.get_aabb(_model)
            _current_editable = editable_in_editor or instancer == Instancer.EDITABLE_NODES
            if _current_instancer:
                _current_instancer.clear(self)
                _current_instancer.instantiate(self, _model, _current_editable)
                _e3d_loaded = true
                e3d_loaded.emit()
            else:
                push_error("Selected instancer is not supported!")
    

func _notification(what: int) -> void:
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED:
            if _current_instancer:
                _current_instancer.sync(self)


func _ready() -> void:
    set_notify_transform(true)
    reload()
    
    
func _enter_tree() -> void:
    if _model and _current_instancer:
        _current_instancer.instantiate(self, _model, _current_editable)


func _exit_tree() -> void:
    if _current_instancer:
        _current_instancer.clear(self)


func is_e3d_loaded():
    return _e3d_loaded
