@tool
extends EditorPlugin

## Scenery content is registered with SceneryStreamingServer, not built up front, so without a
## streaming camera an edited scenery would show nothing. In the editor the camera of the 3D view
## drives the streaming, the way the player's camera does in game (player.gd). What the streaming
## does and what the editor uses is shown in a bottom panel tab, next to the Nodebank.

const STREAMING_DOCK:PackedScene = preload("./scenery_streaming_dock.tscn")

var _streaming_dock:Control = null


func _ready() -> void:
    SceneryStreamingServer.streaming_set_camera(EditorInterface.get_editor_viewport_3d(0).get_camera_3d())


func _enter_tree() -> void:
    _streaming_dock = STREAMING_DOCK.instantiate()
    add_control_to_bottom_panel(_streaming_dock, "Scenery Streaming")


func _exit_tree() -> void:
    SceneryStreamingServer.streaming_set_camera(null)
    remove_control_from_bottom_panel(_streaming_dock)
    _streaming_dock.free()
    _streaming_dock = null
