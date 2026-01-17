@tool
extends Node

signal instances_reloaded

var _t: float = 0.0
var _needs_reload: bool = false
var _instances = []

func _on_user_setting_changed(section, key):
    if section == "e3d" and key == "use_alpha_transparency":
        reload_all()

func _ready():
    UserSettings.game_dir_changed.connect(reload_all)
    UserSettings.cache_cleared.connect(reload_all)
    UserSettings.models_reload_requested.connect(reload_all)
    UserSettings.setting_changed.connect(_on_user_setting_changed)

func register_instance(instance: E3DModelInstance):
    _instances.append(instance)

func unregister_instance(instance: E3DModelInstance):
    var idx = _instances.find(instance)
    if idx > -1:
        _instances.pop_at(idx)

func reload_all():
    for instance in _instances:
        reload_instance(instance)
    instances_reloaded.emit()

func reload_instance(instance: E3DModelInstance):
    if instance.is_inside_tree() and instance.model_filename and instance.data_path:
        if instance.data_path and instance.model_filename:
            var _do_load = func():
                var model = E3DModelManager.load_model(instance.data_path, instance.model_filename)
                if model:
                    match instance.instancer:
                        E3DModelInstance.Instancer.NODES:
                            var _do_instantiate = func():
                                E3DNodesInstancer.instantiate(model, instance, instance.editable_in_editor)
                                #_update_head_display()
                                instance.e3d_loaded.emit()
                            SceneryResourceLoader.schedule(
                                "Creating model %s" % instance.name, _do_instantiate
                            )
                        _:
                            push_error("Selected instancer is not supported!")
            SceneryResourceLoader.schedule("Loading %s" % instance.name, _do_load)
