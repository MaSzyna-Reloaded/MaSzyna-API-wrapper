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
var _needs_reload:bool = false
var _t:float = 0.0

func _get_aabb() -> AABB:
    return submodels_aabb

func _update_head_display():
    if is_inside_tree() and head_display_node_path:
        var node:MeshInstance3D = get_node(head_display_node_path)
        if node:
            node.material_override = head_display_material

func _reload():
    if is_inside_tree() and model_filename and data_path:
        if data_path and model_filename:
            var _do_load = func():
                var model = E3DModelManager.load_model(data_path, model_filename)
                if model:
                    match instancer:
                        Instancer.NODES:
                            var _do_instantiate = func():
                                E3DNodesInstancer.instantiate(model, self, editable_in_editor)
                                _update_head_display()
                                e3d_loaded.emit()
                            SceneryResourceLoader.schedule(
                                "Creating model %s" % self.name, _do_instantiate
                            )
                        _:
                            push_error("Selected instancer is not supported!")
            SceneryResourceLoader.schedule("Loading %s" % self.name, _do_load)


func _on_user_setting_changed(section, key):
    if section == "e3d" and key == "use_alpha_transparency":
        _schedule_reload()

func _schedule_reload():
    _needs_reload = true

func _process(delta):
    _t += delta
    if _t > 0.1:
        _t = 0.0
        if _needs_reload:
            _reload()
            _needs_reload = false

func _ready():
    _reload()
    UserSettings.game_dir_changed.connect(_schedule_reload)
    UserSettings.cache_cleared.connect(_schedule_reload)
    UserSettings.models_reload_requested.connect(_schedule_reload)
    UserSettings.setting_changed.connect(_on_user_setting_changed)
