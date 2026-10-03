@tool
extends EditorPlugin

## Scenery content is registered with SceneryStreamingServer, not built up front, so without a
## streaming camera an edited scenery would show nothing. In the editor the camera of the 3D view
## drives the streaming, the way the player's camera does in game (player.gd). What the streaming
## does and what the editor uses is shown in a bottom panel tab, next to the Nodebank, and a
## scenery's load floating over the 3D view.

const STREAMING_DOCK:PackedScene = preload("./scenery_streaming_dock.tscn")
const LOAD_INDICATOR:PackedScene = preload("./scenery_load_indicator.tscn")
const SceneryLoadIndicator = preload("./scenery_load_indicator.gd")

var _streaming_dock:Control = null
## Floating over the 3D view while a scenery of the edited scene loads
var _load_indicator:SceneryLoadIndicator = null


func _ready() -> void:
    SceneryStreamingServer.streaming_set_camera(EditorInterface.get_editor_viewport_3d(0).get_camera_3d())


func _enter_tree() -> void:
    _streaming_dock = STREAMING_DOCK.instantiate()
    add_control_to_bottom_panel(_streaming_dock, "Scenery Streaming")
    _load_indicator = LOAD_INDICATOR.instantiate()
    # over the 3D view itself, where it moves nothing of the editor's layout. The editor has no
    # API for the control around the view: it is the Node3DEditorViewport holding the view's
    # SubViewportContainer.
    EditorInterface.get_editor_viewport_3d(0).get_parent().get_parent().add_child(_load_indicator)
    scene_changed.connect(_load_indicator.set_scene_root)
    _load_indicator.set_scene_root(EditorInterface.get_edited_scene_root())


func _exit_tree() -> void:
    SceneryStreamingServer.streaming_set_camera(null)
    remove_control_from_bottom_panel(_streaming_dock)
    _streaming_dock.free()
    _streaming_dock = null
    scene_changed.disconnect(_load_indicator.set_scene_root)
    _load_indicator.free()
    _load_indicator = null
