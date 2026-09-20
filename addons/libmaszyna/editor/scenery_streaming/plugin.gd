@tool
extends EditorPlugin

## Scenery content is registered with SceneryStreamingServer, not built up front, so without a
## streaming camera an edited scenery would show nothing. In the editor the camera of the 3D view
## drives the streaming, the way the player's camera does in game (player.gd).
func _ready() -> void:
    SceneryStreamingServer.set_camera(EditorInterface.get_editor_viewport_3d(0).get_camera_3d())


func _exit_tree() -> void:
    SceneryStreamingServer.set_camera(null)
