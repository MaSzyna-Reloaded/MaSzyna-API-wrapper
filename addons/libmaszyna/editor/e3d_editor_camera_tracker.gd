extends MaszynaEditorPluginDelegate
class_name E3DEditorCameraTracker

static var _camera: Camera3D


static func get_camera() -> Camera3D:
    if is_instance_valid(_camera):
        return _camera
    return null


func _editor_plugin_exit_tree(_editor_plugin: EditorPlugin) -> void:
    _camera = null


func _editor_plugin_process(_editor_plugin: EditorPlugin, _delta: float) -> void:
    var editor_viewport: SubViewport = EditorInterface.get_editor_viewport_3d(0)
    if editor_viewport:
        _camera = editor_viewport.get_camera_3d()


func _editor_plugin_forward_3d_gui_input(editor_plugin: EditorPlugin, viewport_camera: Camera3D, _event: InputEvent) -> int:
    _camera = viewport_camera
    return editor_plugin.AFTER_GUI_INPUT_PASS
