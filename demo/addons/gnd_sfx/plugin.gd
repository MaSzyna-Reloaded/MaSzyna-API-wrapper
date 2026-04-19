@tool
extends EditorPlugin

var _automation_inspector_plugin: EditorInspectorPlugin


func _enter_tree() -> void:
    _automation_inspector_plugin = SfxAutomationInspectorPlugin.new()
    add_inspector_plugin(_automation_inspector_plugin)


func _exit_tree() -> void:
    if _automation_inspector_plugin != null:
        remove_inspector_plugin(_automation_inspector_plugin)
        _automation_inspector_plugin = null
