@tool
extends VisualInstance3D
class_name E3DModelInstance

## Displays a MaSzyna E3D model in 3D space.
##
## [E3DModelInstance] is a node client of [E3DRenderingServer]: it loads an [E3DModel] and keeps
## one server instance built with the selected instancer. [code]NODES[/code] and
## [code]EDITABLE_NODES[/code] build a regular node hierarchy under this node.
## [code]OPTIMIZED[/code] renders through [code]RenderingServer[/code] and does not create child nodes.
## If [member model] is set, it will be used to instnatiate. Otherwise the
## [member data_path] and [member model_filename] will be used to load with [E3DModelManager].


## Emitted after the current model instance has been created.
signal e3d_loading
signal e3d_loaded

## Selected instancing backend (same values as [enum E3DRenderingServer.Instancer])
enum Instancer {
     OPTIMIZED,  ## Renders using [code]RenderingServer[/code] without creating child mesh nodes.
     NODES,  ## Creates a regular hierarchy of generated 3D nodes.
     EDITABLE_NODES,  ## Creates generated 3D nodes intended to stay editable in the editor.
}

var _model: E3DModel
var _dirty: bool = false
var _e3d_loaded: bool = false
var _rid: RID = RID()

@export var lights_state: Dictionary[String, bool] = {}:
    set(x):
        lights_state = _merge_lights_state(x)
        if _rid.is_valid():
            E3DRenderingServer.instance_set_lights_state(_rid, lights_state)


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

## Submodel tree paths (relative to the model root, e.g. [code]"banan/podswietlenie_on"[/code])
## for which real alpha blending is forced, instead of the default alpha-scissor cutout.
## Applies to the resolved submodel and all of its descendants (e.g. cabin instrument
## backlight groups). Submodel names alone are not unique across the tree, so paths
## are resolved via [method E3DModel.get_node_or_null].
@export var force_alpha_submodel_paths:Array[NodePath] = []:
    set(x):
        if not x == force_alpha_submodel_paths:
            force_alpha_submodel_paths = x
            _dirty = true

## When true, every submodel already flagged [code]material_transparent[/code] gets real alpha
## blending instead of the default alpha-scissor cutout - unlike [member force_alpha_submodel_paths],
## this is not limited to specific named submodels. Opaque submodels are unaffected. Intended for
## self-contained models (e.g. a cabin interior) where alpha-scissor's crisp cutout looks wrong
## across the board (glass, instrument backlight glow, ...), unlike mixed-purpose exterior content.
@export var force_alpha:bool = false:
    set(x):
        if not x == force_alpha:
            force_alpha = x
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
            _dirty = false
            _process_dirty(_delta)


func _process_dirty(_delta: float) -> void:
    reload()


## Reloads the configured E3D model and recreates the server instance using the selected instancer.
func reload() -> void:
    if is_inside_tree() and (model or model_filename):
        _dirty = false
        _e3d_loaded = false
        e3d_loading.emit()
        _free_instance()

        if model:
            _model = model
        else:
            _model = E3DModelManager.load_model(data_path, model_filename)
        if _model:
            lights_state = _merge_lights_state(lights_state)
            submodels_aabb = E3DModelTool.get_aabb(_model)
            _create_instance()
            _e3d_loaded = true
            e3d_loaded.emit()


func _ready() -> void:
    reload()


func _enter_tree() -> void:
    if _model:
        _create_instance()


func _exit_tree() -> void:
    _free_instance()


func _notification(what: int) -> void:
    match what:
        NOTIFICATION_TRANSFORM_CHANGED:
            if _rid.is_valid():
                E3DRenderingServer.instance_set_transform(_rid, global_transform)
        NOTIFICATION_VISIBILITY_CHANGED:
            if _rid.is_valid():
                E3DRenderingServer.instance_set_visible(_rid, is_visible_in_tree())


func is_e3d_loaded() -> bool:
    return _e3d_loaded


func _create_instance() -> void:
    var server_instancer: int = (
        Instancer.EDITABLE_NODES if editable_in_editor and instancer == Instancer.NODES else instancer
    )
    _rid = E3DRenderingServer.instance_create(_model, server_instancer)
    E3DRenderingServer.instance_set_options(
        _rid, data_path, PackedStringArray(skins), exclude_node_names, force_alpha, force_alpha_submodel_paths
    )
    E3DRenderingServer.instance_attach_node(_rid, self)
    E3DRenderingServer.instance_set_scenario(_rid, get_world_3d().scenario)
    E3DRenderingServer.instance_set_transform(_rid, global_transform)
    E3DRenderingServer.instance_set_visible(_rid, is_visible_in_tree())
    E3DRenderingServer.instance_set_layer_mask(_rid, layers)
    E3DRenderingServer.instance_set_lights_state(_rid, lights_state)
    E3DRenderingServer.instance_build(_rid)
    set_notify_transform(instancer == Instancer.OPTIMIZED)


func _free_instance() -> void:
    if _rid.is_valid():
        E3DRenderingServer.instance_free(_rid)
        _rid = RID()


func _merge_lights_state(new_state: Dictionary[String, bool]) -> Dictionary[String, bool]:
    var state: Dictionary[String, bool] = {}
    if _model:
        for light_name in _model.lights.keys():
            state[light_name] = false
    state.merge(new_state, true)
    if _model:
        for light_name: String in new_state.keys():
            if not _model.lights.has(light_name):
                state.erase(light_name)
    state.sort()
    return state
