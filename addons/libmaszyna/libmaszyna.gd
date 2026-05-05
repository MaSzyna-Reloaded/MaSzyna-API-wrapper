@tool
extends EditorPlugin

# Custom nodes

var maszyna_environment_node_script = preload("res://addons/libmaszyna/environment/maszyna_environment_node.gd")
var maszyna_environment_node_icon = preload("res://addons/libmaszyna/environment/maszyna_environment_node_icon.png")
var maszyna_track_3d_script = preload("res://addons/libmaszyna/maszyna_track_3d.gd")
var maszyna_switch_3d_script = preload("res://addons/libmaszyna/maszyna_switch_3d.gd")

# Editor plugins

var maszyna_editor_plugins:Array[MaszynaEditorPluginDelegate] = []

var e3d_submodel_toolbar = preload("res://addons/libmaszyna/editor/toolbar_e3d_instance.tscn")
var e3d_submodel_toolbar_instance
var nodebank_panel_scene = preload("res://addons/libmaszyna/editor/nodebank/nodebank_panel.tscn")
var nodebank_panel_instance
var nodebank_bottom_button
var nodebank_editor_plugin: NodebankEditorPluginDelegate

var user_settings_dock
var user_settings_dock_scene = preload("res://addons/libmaszyna/editor/user_settings_dock.tscn")

func _enter_tree():
    set_input_event_forwarding_always_enabled()
    set_process(false)

    e3d_submodel_toolbar_instance = e3d_submodel_toolbar.instantiate()
    add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

    nodebank_panel_instance = nodebank_panel_scene.instantiate()
    nodebank_editor_plugin = NodebankEditorPluginDelegate.new()
    nodebank_panel_instance.set_plugin_runtime()
    nodebank_panel_instance.item_drag_started.connect(nodebank_editor_plugin.drag_start)
    nodebank_bottom_button = add_control_to_bottom_panel(nodebank_panel_instance, "Nodebank")

    register_maszyna_editor_plugin(nodebank_editor_plugin)
    
    add_custom_project_setting("maszyna/import_model_scale_factor", 1.0, TYPE_FLOAT)

    add_autoload_singleton("MaszynaEnvironment", "res://addons/libmaszyna/environment/maszyna_environment.gd")
    add_autoload_singleton("Console", "res://addons/libmaszyna/console/console.gd")
    add_autoload_singleton("AudioStreamManager", "res://addons/libmaszyna/sound/audio_stream_manager.gd")

    add_custom_type(
        "MaszynaEnvironmentNode",
        "Node",
        maszyna_environment_node_script,
        maszyna_environment_node_icon,
    )

    add_custom_type(
        "MaszynaTrack3D",
        "Path3D",
        maszyna_track_3d_script,
        null
    )

    add_custom_type(
        "MaszynaSwitch3D",
        "Node3D",
        maszyna_switch_3d_script,
        null
    )

    user_settings_dock = user_settings_dock_scene.instantiate()
    add_control_to_dock(DOCK_SLOT_RIGHT_UL, user_settings_dock)
    
    for plugin in maszyna_editor_plugins:
        plugin._editor_plugin_enter_tree(self)
        
    if maszyna_editor_plugins.size() > 0:
        set_process(true)
        
        
func _exit_tree():
    for plugin in maszyna_editor_plugins:
        plugin._editor_plugin_exit_tree(self)
        
    remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, e3d_submodel_toolbar_instance)

    if nodebank_panel_instance:
        remove_control_from_bottom_panel(nodebank_panel_instance)
        nodebank_panel_instance.free()
        nodebank_panel_instance = null

    if user_settings_dock:
        remove_control_from_docks(user_settings_dock)

    if nodebank_editor_plugin:
        unregister_maszyna_editor_plugin(nodebank_editor_plugin)

    remove_custom_type("MaszynaEnvironmentNode")
    remove_custom_type("MaszynaTrack3D")
    remove_custom_type("MaszynaSwitch3D")

    remove_autoload_singleton("AudioStreamManager")
    remove_autoload_singleton("Console")
    remove_autoload_singleton("MaszynaEnvironment")


func register_maszyna_editor_plugin(editor_plugin:MaszynaEditorPluginDelegate):
    maszyna_editor_plugins.append(editor_plugin)
    
func unregister_maszyna_editor_plugin(editor_plugin:MaszynaEditorPluginDelegate):
    var idx = maszyna_editor_plugins.find(editor_plugin)
    if idx > -1:
        maszyna_editor_plugins.remove_at(idx)
    
func add_custom_project_setting(name: String, default_value, type: int, hint: int = PROPERTY_HINT_NONE, hint_string: String = "") -> void:
    if ProjectSettings.has_setting(name):
        return

    var setting_info: Dictionary = {
        "name": name,
        "type": type,
        "hint": hint,
        "hint_string": hint_string
    }

    ProjectSettings.set_setting(name, default_value)
    ProjectSettings.add_property_info(setting_info)
    ProjectSettings.set_initial_value(name, default_value)


func _forward_3d_gui_input(viewport_camera: Camera3D, event: InputEvent) -> int:
    for plugin in maszyna_editor_plugins:
        var result = plugin._editor_plugin_forward_3d_gui_input(self, viewport_camera, event)
        if result == AFTER_GUI_INPUT_PASS:
            continue
        return result
    return AFTER_GUI_INPUT_PASS


func _process(_delta: float) -> void:
    for plugin in maszyna_editor_plugins:
        plugin._editor_plugin_process(self, _delta)
