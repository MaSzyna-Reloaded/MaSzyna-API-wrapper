extends UIChip

## A spinner on a HUD tile, looking like the others (DrivingAid.apply_style()), while
## SceneryStreamingServer builds the pieces around the camera: it fades in when the builds start
## and out when they finish. The spinning is the shader's (streaming_spinner.gdshader), so no
## script runs per frame.

## Seconds of the fade in and of the fade out
const FADE_SEC:float = 0.35

var _fade:Tween = null


func _ready() -> void:
    super()
    DrivingAid.apply_style(self)


func _enter_tree() -> void:
    SceneryStreamingServer.streaming_builds_started.connect(_on_streaming_builds_started)
    SceneryStreamingServer.streaming_builds_finished.connect(_on_streaming_builds_finished)
    if SceneryStreamingServer.streaming_is_building():
        _on_streaming_builds_started()


func _exit_tree() -> void:
    SceneryStreamingServer.streaming_builds_started.disconnect(_on_streaming_builds_started)
    SceneryStreamingServer.streaming_builds_finished.disconnect(_on_streaming_builds_finished)


func _on_streaming_builds_started() -> void:
    if _fade:
        _fade.kill()
    show()
    _fade = create_tween()
    _fade.tween_property(self, "modulate:a", 1.0, FADE_SEC).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_OUT)


func _on_streaming_builds_finished() -> void:
    if _fade:
        _fade.kill()
    _fade = create_tween()
    _fade.tween_property(self, "modulate:a", 0.0, FADE_SEC).set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_IN)
    # hidden, the shader is not drawn at all
    _fade.tween_callback(hide)
