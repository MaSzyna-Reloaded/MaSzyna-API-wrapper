extends Node3D


# Called when the node enters the scene tree for the first time.
func _enter_tree() -> void:
    SceneryStreamingServer.streaming_set_camera($FreeCamera3D)

func _exit_tree() -> void:
    SceneryStreamingServer.streaming_set_camera(null)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
