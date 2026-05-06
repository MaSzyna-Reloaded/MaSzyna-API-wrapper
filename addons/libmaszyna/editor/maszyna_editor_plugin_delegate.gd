extends Object
class_name MaszynaEditorPluginDelegate

func _editor_plugin_enter_tree(editor_plugin: EditorPlugin) -> void:
    pass

func _editor_plugin_exit_tree(editor_plugin: EditorPlugin) -> void:
    pass

func _editor_plugin_process(editor_plugin: EditorPlugin, _delta: float) -> void:
    pass
    
func _editor_plugin_forward_3d_gui_input(editor_plugin: EditorPlugin, viewport_camera: Camera3D, event: InputEvent) -> int:
    return editor_plugin.AFTER_GUI_INPUT_PASS
