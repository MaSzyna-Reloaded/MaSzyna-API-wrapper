@tool
extends VisualInstance3D
class_name E3DModelInstance

## Displays a MaSzyna E3D model in 3D space.
##
## [E3DModelInstance] loads an [E3DModel] and instantiates it using one of the
## supported instancers. [code]NODES[/code] and [code]EDITABLE_NODES[/code]
## build a regular node hierarchy. [code]OPTIMIZED[/code] renders through
## [code]RenderingServer[/code] and does not create child mesh nodes.
## If [member model] is set, it will be used to instnatiate. Otherwise the
## [member data_path] and [member model_filename] will be used to load with [E3DModelManager].


## Emitted after the current model instance has been created.
signal e3d_loading
signal e3d_loaded

## Selected instancing backend
enum Instancer {
     OPTIMIZED,  ## Renders using [code]RenderingServer[/code] without creating child mesh nodes.
     NODES,  ## Creates a regular hierarchy of generated 3D nodes.
     EDITABLE_NODES,  ## Creates generated 3D nodes intended to stay editable in the editor.
}

var _model: E3DModel
var _dirty: bool = false
var _e3d_loaded: bool = false
var _current_instancer: E3DInstancer
var _current_editable: bool = false
var _range_check_interval: float = 1.0
var _range_check_elapsed: float = 0.0

var default_aabb_size: Vector3 = Vector3(1, 1, 1)

## E3DModel to instantiate (leave empty, if you want to lazy load with [member model_filename])
@export var model: E3DModel:
    set(x):
        if not x == model:
            model = x
            _mark_dirty()

## Base MaSzyna data path used to resolve model files and materials.
@export var data_path:String = "":
    set(x):
        if not x == data_path:
            data_path = x
            _mark_dirty()

@export var model_filename:String = "":
    set(x):
        if not x == model_filename:
            model_filename = x
            _mark_dirty()

@export var skins:Array = []:
    set(x):
        if not x == skins:
            skins = x
            _mark_dirty()

@export var exclude_node_names:Array = []:
    set(x):
        if not x == exclude_node_names:
            exclude_node_names = x
            _mark_dirty()

# Probably instancer should be set project-wide
@export var instancer = Instancer.NODES:
    set(x):
        if not x == instancer:
            instancer = x
            _mark_dirty()

## Minimum distance from camera required to instantiate the model.
## When set to [code]0.0[/code], there is no near distance limit.
@export var visibility_range_begin: float = 0.0:
    set(x):
        if not is_equal_approx(x, visibility_range_begin):
            visibility_range_begin = x
            _mark_dirty()

## Maximum distance from camera allowed to keep the model instantiated.
## When set to [code]0.0[/code], there is no far distance limit.
@export var visibility_range_end: float = 0.0:
    set(x):
        if not is_equal_approx(x, visibility_range_end):
            visibility_range_end = x
            _mark_dirty()

var submodels_aabb:AABB = AABB()
var editable_in_editor:bool = false:
    set(x):
        if not editable_in_editor == x:
            editable_in_editor = x
            _mark_dirty()


func _get_aabb() -> AABB:
    return submodels_aabb


func _process(delta: float) -> void:
    if _dirty:
        _process_dirty(delta)
        return

    _range_check_elapsed += delta
    if _range_check_elapsed >= _range_check_interval:
        _apply_visibility_state()


func _process_dirty(_delta: float) -> void:
    _dirty = false
    _apply_visibility_state(true)


## Reloads the configured E3D model and recreates the current instance using the selected instancer.
func reload() -> void:
    if not is_inside_tree():
        return

    _apply_visibility_state(true)
    

func _notification(what: int) -> void:
    match what:
        NOTIFICATION_TRANSFORM_CHANGED, NOTIFICATION_VISIBILITY_CHANGED:
            if _e3d_loaded and _current_instancer:
                _current_instancer.sync(self)


func _ready() -> void:
    set_notify_transform(true)
    set_process(true)
    _apply_visibility_state(false)
    
    
func _enter_tree() -> void:
    _range_check_elapsed = 0.0


func _exit_tree() -> void:
    _clear_instance()


func is_e3d_loaded():
    return _e3d_loaded


func _resolve_instancer():
    match instancer:
        Instancer.NODES, Instancer.EDITABLE_NODES:
            return E3DNodesInstancer
        _:
            push_error("Unsupported instancer " + instancer)
            return null


func _mark_dirty() -> void:
    _dirty = true
    _range_check_elapsed = _range_check_interval


func _apply_visibility_state(force_reload: bool = false) -> void:
    _range_check_elapsed = 0.0

    if _should_instance_be_visible():
        if force_reload or not _e3d_loaded:
            _instantiate_model()
    else:
        _clear_instance()


func _should_instance_be_visible() -> bool:
    if not is_inside_tree():
        return false

    if is_zero_approx(visibility_range_begin) and is_zero_approx(visibility_range_end):
        return true

    var camera: Camera3D = _get_active_camera()
    if not is_instance_valid(camera):
        return false

    var distance: float = camera.global_position.distance_to(global_position)
    if visibility_range_begin > 0.0 and distance < visibility_range_begin:
        return false
    if visibility_range_end > 0.0 and distance > visibility_range_end:
        return false
    return true


func _get_active_camera() -> Camera3D:
    if Engine.is_editor_hint():
        return E3DEditorCameraTracker.get_camera()

    var viewport: Viewport = get_viewport()
    if viewport:
        return viewport.get_camera_3d()
    return null


func _instantiate_model() -> void:
    if not (model or model_filename):
        _clear_instance()
        return

    if model:
        _model = model
    else:
        _model = E3DModelManager.load_model(data_path, model_filename)

    if not _model:
        _clear_instance()
        return

    if _e3d_loaded:
        _clear_instance()

    _e3d_loaded = false
    e3d_loading.emit()
    submodels_aabb = E3DModelTool.get_aabb(_model)
    _current_editable = editable_in_editor or instancer == Instancer.EDITABLE_NODES
    _current_instancer = _resolve_instancer()

    if not _current_instancer:
        push_error("Selected instancer is not supported!")
        return

    _current_instancer.instantiate(self, _model, _current_editable)
    _e3d_loaded = true
    e3d_loaded.emit()


func _clear_instance() -> void:
    if _current_instancer:
        _current_instancer.clear(self)
    _e3d_loaded = false
